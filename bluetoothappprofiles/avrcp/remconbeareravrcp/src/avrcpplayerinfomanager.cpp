// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file
 @internalComponent
 @released
*/

#include <avctpservices.h> // for AVCTP protocol version
#include <bt_sock.h> // for PSM values
#include <remcon/clientid.h>
#include <absolutevolumeapi.h>
#include <remcon/remconbearerbulkobserver.h>

#include "avrcp.h"
#include "avrcpplayerinfomanager.h"
#include "avrcpsdputils.h"
#include "playerbitmasks.h"
#include "avrcputils.h"
#include "avrcplog.h"
#include "mediabrowse.h"
#include "nowplaying.h"
#include "playerinformation.h"
#include "remcongroupnavigation.h"

#ifdef _DEBUG
PANICCATEGORY("playerinfo");
#endif // _DEBUG

static TBool PlayerCompare(const TRemConClientId* aId, const TAvrcpMediaPlayerItem& aItem)
	{
	LOG_STATIC_FUNC
	return (aId && *aId == aItem.iId);
	}

enum TFirstAbsVolSupport { EFirstAbsVolSupport };
static TBool FirstAbsVolSupport(const TFirstAbsVolSupport*, const TAvrcpMediaPlayerItem& aItem)
	{
	LOG_STATIC_FUNC
	return aItem.iId != KNullClientId && aItem.iAbsoluteVolumeSupport;
	}

enum TFirstBrowsingSupport { EFirstBrowsingSupport };
static TBool FirstBrowsingSupport(const TFirstBrowsingSupport*, const TAvrcpMediaPlayerItem& aItem)
	{
	LOG_STATIC_FUNC
	return aItem.iId != KNullClientId && (aItem.iSdpFeatures & AvrcpSdp::EBrowsing);
	}

CAvrcpPlayerInfoManager* CAvrcpPlayerInfoManager::NewL(MRemConBearerObserver& aObserver, MRemConCommandInterface& aCommandInterface)
	{
	LOG_STATIC_FUNC
	CAvrcpPlayerInfoManager* info = new(ELeave) CAvrcpPlayerInfoManager(aObserver);
	CleanupStack::PushL(info);
	info->ConstructL(aCommandInterface);
	CleanupStack::Pop(info);
	return info;
	}

void CAvrcpPlayerInfoManager::ConstructL(MRemConCommandInterface& aCommandInterface)
	{
	LOG_FUNC
	LEAVEIFERRORL(iLock.CreateLocal());
	LEAVEIFERRORL(iSdp.Connect());
	LEAVEIFERRORL(iSdpDatabase.Open(iSdp));
	
	iPlayStatusWatcher = CPlayStatusWatcher::NewL(*this, aCommandInterface);
	
	TCallBack cb(PlayerUpdateCallBack, this);
	iControlThreadCallBack.Create(cb, CActive::EPriorityStandard);
	iControlThreadCallBack.Start();
	}

CAvrcpPlayerInfoManager::~CAvrcpPlayerInfoManager()
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iLock.Wait();// serialise access...
	iLock.Close();
	delete iPlayStatusWatcher;
	ASSERT_DEBUG(!iUidWatcher); // created in bulk thread context...
	iControlThreadCallBack.Close();
	iObservers.Close();
	iPlayers.Close();
	iSdpDatabase.Close();
	iSdp.Close();
	}

CAvrcpPlayerInfoManager::CAvrcpPlayerInfoManager(MRemConBearerObserver& aObserver)
	: iControlBearerObserver(aObserver)
	{
	LOG_FUNC
	}

/**
This function sets up the handling in the bulk thread.  This involves creating the 
player state watcher, which maintains an up to date view of the UID counter values
for each player with a bulk interface.  It does this by acting as an internal
command handler.  The bulk bearer expects the player info manager to provide it
with a command handling interface for commands on internal interfaces, so we provide
it with the interface for the state watcher, which is where all our internal commands
come from.
*/
MIncomingCommandHandler* CAvrcpPlayerInfoManager::BulkStartedL(MRemConCommandInterface& aCommandInterface)
	{
	LOG_FUNC;
	ASSERT_BULK_THREAD;

	iUidWatcher = CUidWatcher::NewL(*this, aCommandInterface);
	return iUidWatcher;
	}

void CAvrcpPlayerInfoManager::BulkStopped()
	{
	LOG_FUNC;
	ASSERT_BULK_THREAD;
	delete iUidWatcher;
	iUidWatcher = NULL;
	}

// Helper function for ClientAvailable and TargetFeaturesUpdated
TInt CAvrcpPlayerInfoManager::SetItemDetails(TAvrcpMediaPlayerItem& aItem, TPlayerType aPlayerType, TPlayerSubType aPlayerSubType, const TDesC8& aName)
	{
	aItem.iPlayerType = aPlayerType;
	aItem.iPlayerSubType = aPlayerSubType;
	aItem.iName.Set(aName);
	aItem.iFeatureBitmask = TPlayerFeatureBitmask();
	return SetPlayerFeatures(aItem.iId, aItem.iFeatureBitmask, aItem.iSdpFeatures, aItem.iAbsoluteVolumeSupport);
	}

// Helper function for ClientAvailable and TargetFeaturesUpdated
void CAvrcpPlayerInfoManager::UpdateSdpRecord(const TAvrcpMediaPlayerItem& aItem)
	{
	// Update SDP record, if this fails we carry on, it's non-fatal
	TInt sdpErr = KErrNone;
	if(!iTargetRecord)
		{
		TRAP(sdpErr, AvrcpSdpUtils::CreateServiceRecordL(iSdpDatabase, iTargetRecord, ETrue, 
					(aItem.iSdpFeatures & AvrcpSdp::EBrowsing) ? AvrcpSdp::KAvrcpProfileVersion14 : AvrcpSdp::KAvrcpProfileVersion13));
		}
	if(sdpErr == KErrNone)
		{
		TRAP_IGNORE(UpdateTgServiceRecordL());
		}
	}

void CAvrcpPlayerInfoManager::ClientAvailable(const TRemConClientId& aId, TPlayerType aPlayerType, TPlayerSubType aPlayerSubType, const TDesC8& aName)
	{
	LOG_FUNC;
	ASSERT_CONTROL_THREAD;
	iLock.Wait();
	// Add this to our client list, using any holes in the client id array
	TInt index = NextPlayerIndex();
	if(index < 0)
		{
		// Couldn't allocate memory to store this player, remote will just
		// have to make do with the current set
		iLock.Signal();
		return;
		}
	
	TAvrcpMediaPlayerItem& item = iPlayers[index];
	item.iId = aId;
	item.iBulkClientAvailable = EFalse;
	item.iUidCounter = 0;
	item.iLastUpdatedUidCounter = 0;
	item.iPlaybackStatus = MPlayerEventsObserver::EStopped;
	TInt err = SetItemDetails(item, aPlayerType, aPlayerSubType, aName);

	// Release lock before calling out of player info manager in case
	// anyone needs to call back in - we're finished updating the
	// info now.
	iLock.Signal();

	if(!err)
		{
		TRAP(err, iPlayStatusWatcher->StartWatchingPlayerL(aId));
		if(!err)
			{
			UpdateSdpRecord(item);
		     for(TInt i = 0; i<iObservers.Count(); i++)
				{
				iObservers[i]->MpcoAvailablePlayersChanged();
				}
			}
		  else    
			{
			iLock.Wait();
			iPlayers[index].iId = KNullClientId;
			iLock.Signal();
			}
		}
	}

void CAvrcpPlayerInfoManager::ClientNotAvailable(const TRemConClientId& aId)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iLock.Wait();
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index < 0)
		{
		// May not be here if we couldn't allocate memory to store this
		// when we were first told about it
		iLock.Signal();
		return;
		}
	
	iPlayStatusWatcher->StopWatchingPlayer(aId);
	iPlayers[index].iId = KNullClientId;
	
	//Release lock as soon as we've stopped fiddling
	iLock.Signal();
	
	for(TInt i = 0; i<iObservers.Count(); i++)
		{
		iObservers[i]->MpcoAvailablePlayersChanged();
		}
	
	// Don't put requirement on ordering of ClientNotAvailable/ClientStatus calls
	if(iTargetRecord)
		{
		TRAP_IGNORE(UpdateTgServiceRecordL());
		}
	}

void CAvrcpPlayerInfoManager::ClientStatus(TBool aControllerPresent, TBool aTargetPresent)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	// SDP only used in the control thread
	
	//If we have gone from 1->0 on either of these we can remove the record now,
	// otherwise wait for more detail on ClientAvailable or ControllerFeaturesUpdatedL
	if(!aControllerPresent && iControllerRecord)
		{
		iSdpDatabase.DeleteRecord(iControllerRecord);
		iControllerRecord = 0;
		}
			
	if(!aTargetPresent && iTargetRecord)
		{
		iSdpDatabase.DeleteRecord(iTargetRecord);
		iTargetRecord = 0;
		}
	}

TInt CAvrcpPlayerInfoManager::SetLocalAddressedClient(const TRemConClientId& aId)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD

	TUint16 playerId = 0;
	TRAPD(err, playerId = PlayerL(aId));
	static_cast<void>(playerId==playerId); // We only want to check if the bearer knows about the client.
	if(err == KErrNone)
		{
		for(TInt i = 0; i<iObservers.Count(); i++)
			{
			iObservers[i]->MpcoAddressedPlayerChangedLocally(aId);
			}
		}
	
	return err;
	}

void CAvrcpPlayerInfoManager::ControllerFeaturesUpdatedL(RArray<TUid>& aSupportedInterfaces)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	// SDP only used in the control thread

	TUint16 avrcpVersion = AvrcpSdp::KAvrcpProfileVersion13; 
	TUint16 avctpVersion = AvrcpSdp::KAvctpProtocolVersion12; 

	if(aSupportedInterfaces.Find(TUid::Uid(KRemConAbsoluteVolumeControllerApiUid)) >= 0)
		{
		avrcpVersion = AvrcpSdp::KAvrcpProfileVersion14;
		avctpVersion = AvrcpSdp::KAvctpProtocolVersion13;
		}

	if(!iControllerRecord)
		{
		AvrcpSdpUtils::CreateServiceRecordL(iSdpDatabase, iControllerRecord, EFalse, avrcpVersion);		
		AvrcpSdpUtils::UpdateBrowseListL(iSdpDatabase, iControllerRecord);
		}

	AvrcpSdpUtils::UpdateProtocolDescriptorListL(iSdpDatabase, iControllerRecord, avctpVersion);
	AvrcpSdpUtils::UpdateProfileDescriptorListL(iSdpDatabase, iControllerRecord, avrcpVersion);
	AvrcpSdpUtils::UpdateSupportedFeaturesL(iSdpDatabase, iControllerRecord, AvrcpSdp::ERemoteControl, AvrcpSdp::KAvrcpBaseCtFeatures);
	}

void CAvrcpPlayerInfoManager::TargetFeaturesUpdated(const TRemConClientId& aId, TPlayerType aPlayerType, TPlayerSubType aPlayerSubType, const TDesC8& aName)
	{
	LOG_FUNC;
	ASSERT_CONTROL_THREAD;
	iLock.Wait();
	// Find this client in our client list
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index < 0)
		{
		// Couldn't find client in client list, maybe we removed it after an earlier failure
		iLock.Signal();
		return;
		}

	TAvrcpMediaPlayerItem& item = iPlayers[index];
	TInt err = SetItemDetails(item, aPlayerType, aPlayerSubType, aName);

	// Release lock before calling out of player info manager in case
	// anyone needs to call back in - we're finished updating the
	// info now.
	iLock.Signal();

	if(!err)
		{
		UpdateSdpRecord(item);
		}
	else    
		{
		// There was an error updating the features so remove this client from the client list
		iLock.Wait();
		iPlayers[index].iId = KNullClientId;
		iLock.Signal();
		}

	for(TInt i = 0; i<iObservers.Count(); i++)
		{
		iObservers[i]->MpcoAvailablePlayersChanged();
		}
	}

MIncomingCommandHandler& CAvrcpPlayerInfoManager::InternalCommandHandler()
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD

	return *iPlayStatusWatcher;
	}

void CAvrcpPlayerInfoManager::BulkClientAvailable(const TRemConClientId& aId)
	{
	LOG_FUNC
	ASSERT_BULK_THREAD
	iLock.Wait();
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index < 0)
		{
		iLock.Signal();
		return;
		}
	ASSERT(!iPlayers[index].iBulkClientAvailable);
	
	TRAPD(err, iUidWatcher->StartWatchingPlayerL(aId));
	
	if(!err)
		{
		iPlayers[index].iBulkClientAvailable = ETrue;
		}
	
	iLock.Signal();
	}

void CAvrcpPlayerInfoManager::BulkClientNotAvailable(const TRemConClientId& aId)
	{
	LOG_FUNC
	ASSERT_BULK_THREAD
	iLock.Wait();
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index >= 0)
		{
		ASSERT(iPlayers[index].iBulkClientAvailable);
		iPlayers[index].iBulkClientAvailable = EFalse;
		}
	iLock.Signal();
	iUidWatcher->StopWatchingPlayer(aId);
	}

void CAvrcpPlayerInfoManager::AddObserverL(MPlayerChangeObserver& aObserver)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iObservers.AppendL(&aObserver);
	}

void CAvrcpPlayerInfoManager::RemoveObserver(MPlayerChangeObserver& aObserver)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	TInt index = iObservers.Find(&aObserver);
	if(index >= 0)
		{
		iObservers.Remove(index);
		}
	}

TRemConClientId CAvrcpPlayerInfoManager::ClientL(TUint16 aAvrcpPlayerId) const
	{
	LOG_FUNC
	// THREAD - Bulk and Control
	
	iLock.Wait();
	CleanupSignalPushL(iLock);
	
	TRemConClientId clientId = KNullClientId;
	if(ValidPlayer(aAvrcpPlayerId))
		{
		clientId = iPlayers[aAvrcpPlayerId].iId;
		}
	else
		{
		LEAVEL(KErrNotFound);
		}
	
	CleanupStack::PopAndDestroy(&iLock);
	
	return clientId;
	}

TRemConClientId CAvrcpPlayerInfoManager::Client(TUint16 aAvrcpPlayerId) const
	{
	LOG_FUNC
	// THREAD - Bulk and Control
	
	iLock.Wait();
	
	TRemConClientId clientId = KNullClientId;
	__ASSERT_DEBUG(ValidPlayer(aAvrcpPlayerId), AvrcpUtils::Panic(EInvalidPlayerId));
	
	clientId = iPlayers[aAvrcpPlayerId].iId;
	
	iLock.Signal();
	return clientId;
	}

TUint16 CAvrcpPlayerInfoManager::PlayerL(const TRemConClientId& aClientId) const
	{
	LOG_FUNC
	// THREAD - Bulk and Control
	
	iLock.Wait();
	
	TUint16 player = 0;
	TBool found = EFalse;
	for(TInt i = 0; i < iPlayers.Count(); i++)
		{
		if(PlayerCompare(&aClientId, iPlayers[i]))
			{
			player = i;
			found = ETrue;
			}
		}

	iLock.Signal();
	
	if(!found)
		{
		LEAVEL(KErrNotFound);
		}
	
	return player;
	}

TInt CAvrcpPlayerInfoManager::PlayerListing(TUint aStartItem, TUint aEndItem, RArray<TUint>& aPlayers)
	{
	LOG_FUNC
	ASSERT_BULK_THREAD

	if(aStartItem > aEndItem)
		{
		return KErrArgument;
		}
	
	iLock.Wait();
	if(aStartItem >= iPlayers.Count())
		{
		iLock.Signal();
		return KErrArgument;
		}
	
	TInt err = KErrNone;
	for(TUint i = aStartItem; i <= aEndItem && i < iPlayers.Count() && err == KErrNone; i++)
		{
		if(iPlayers[i].iId != KNullClientId)
			{
			err = aPlayers.Append(i);
			}
		}
	
	iLock.Signal();
	return err;
	}

TInt CAvrcpPlayerInfoManager::PlayerListing(RArray<TUint>& aPlayers)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iLock.Wait();
	TInt err = KErrNone;
	for(TInt i = 0; i < iPlayers.Count() && err == KErrNone; i++)
		{
		if(iPlayers[i].iId != KNullClientId)
			{
			err = aPlayers.Append(i);
			}
		}
	
	iLock.Signal();
	return err;
	}

void CAvrcpPlayerInfoManager::MediaPlayerItemL(const TUint16& aAvrcpPlayerId, RMediaPlayerItem& aItem)
	{
	LOG_FUNC
	ASSERT_BULK_THREAD
	iLock.Wait();
	CleanupSignalPushL(iLock);
	
	if(ValidPlayer(aAvrcpPlayerId))
		{
		aItem.iType = AvrcpBrowsing::EMediaPlayer;
		aItem.iPlayerId = aAvrcpPlayerId;
		aItem.iPlayerType = iPlayers[aAvrcpPlayerId].iPlayerType;
		aItem.iPlayerSubType = iPlayers[aAvrcpPlayerId].iPlayerSubType;
		aItem.iPlayStatus = iPlayers[aAvrcpPlayerId].iPlaybackStatus;
		aItem.iFeatureBitmask = iPlayers[aAvrcpPlayerId].iFeatureBitmask.FeatureBitmask();
		aItem.iCharset = KUtf8MibEnum;
		aItem.iNameLength = iPlayers[aAvrcpPlayerId].iName.Length();
		// Takes a copy of media player name
		aItem.iName.CreateL(iPlayers[aAvrcpPlayerId].iName);
		
		aItem.iLength = 28 + aItem.iNameLength;
		}
	else
		{
		LEAVEL(KErrNotFound);
		}
	
	CleanupStack::PopAndDestroy(&iLock);
	}

TBool CAvrcpPlayerInfoManager::ValidPlayer(const TUint16& aAvrcpPlayerId) const
	{
	LOG_FUNC

	if(aAvrcpPlayerId >= iPlayers.Count() || iPlayers[aAvrcpPlayerId].iId == KNullClientId)
		{
		return EFalse;
		}
	else
		{
		return ETrue;
		}
	}

TInt CAvrcpPlayerInfoManager::NextPlayerIndex()
	{
	LOG_FUNC

	TRemConClientId nullId = KNullClientId;
	TInt index = iPlayers.Find(nullId, PlayerCompare);

	if(index < 0)
		{
		// No gaps to fill, add another item to the player list
		TAvrcpMediaPlayerItem item;
		TInt err = iPlayers.Append(item);
		if(!err)
			{
			index = iPlayers.Count() - 1;
			}
		}
	
	return index;
	}

void CAvrcpPlayerInfoManager::MuoUidChanged(TRemConClientId aId, TUint16 aUidCounter)
	{
	LOG_FUNC;
	ASSERT_BULK_THREAD
	iLock.Wait();
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index >= 0)
		{
		iPlayers[index].iUidCounter = aUidCounter;
		}
	iLock.Signal();
	
	iControlThreadCallBack.CallBack();
	}

void CAvrcpPlayerInfoManager::MuoError(TRemConClientId aId)
	{
	// Narg.  Tell everyone we can't use this player anymore
	BulkClientNotAvailable(aId);
	}

void CAvrcpPlayerInfoManager::MpsoPlayStatusChanged(TRemConClientId aId, MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iLock.Wait();
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index >= 0)
		{
		iPlayers[index].iPlaybackStatus = static_cast<MPlayerEventsObserver::TPlaybackStatus>(aPlaybackStatus);
		}
	iLock.Signal();
	}

void CAvrcpPlayerInfoManager::MpsoError(TRemConClientId aId)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iLock.Wait();
	TInt index = iPlayers.Find(aId, PlayerCompare);
	if(index >= 0)
		{
		iPlayers[index].iPlaybackStatus = MPlayerEventsObserver::EError;
		}
	iLock.Signal();
	}

TUint16 CAvrcpPlayerInfoManager::UidCounterL(const TRemConClientId& aClientId) const
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	iLock.Wait();
	CleanupSignalPushL(iLock);
	TUint16 uidCounter = KErrNone;
	TInt index = iPlayers.Find(aClientId, PlayerCompare);
	if(index < 0)
		{
		LEAVEL(KErrNotFound);
		}
	uidCounter = iPlayers[index].iUidCounter;
	CleanupStack::PopAndDestroy(&iLock);
	return uidCounter;
	}

TInt CAvrcpPlayerInfoManager::SetPlayerFeatures(const TRemConClientId& aId, TPlayerFeatureBitmask& aBitmask, TUint8& aSdpFeatures, TBool& aAbsoluteVolumeSupported)
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD

	TInt err;
	RArray<TUint> operations;
	
	aAbsoluteVolumeSupported = EFalse;
	aSdpFeatures = 0;
	err = iControlBearerObserver.SupportedOperations(aId, TUid::Uid(KRemConCoreApiUid), operations);
	if(!err)
		{
		aBitmask.SetCoreApiFeatures(operations);
		}
	else if(err != KErrNotSupported)
		{
		operations.Close();
		return err;
		}
	
	err = iControlBearerObserver.SupportedOperations(aId, TUid::Uid(KRemConPlayerInformationUid), operations);
	if(!err)
		{
		aSdpFeatures |= AvrcpSdp::EPlayer;
		}
	else if(err != KErrNotSupported)
		{
		operations.Close();
		return err;
		}
	
	err = iControlBearerObserver.SupportedOperations(aId, TUid::Uid(KRemConGroupNavigationApiUid), operations);
	if(!err)
		{
		aSdpFeatures |= AvrcpSdp::EGroupNavigation;
		aBitmask.SetGroupNavigationApiFeatures(operations);
		}
	else if(err != KErrNotSupported)
		{
		return err;
		}
	
	err = iControlBearerObserver.SupportedOperations(aId, TUid::Uid(KRemConAbsoluteVolumeTargetApiUid), operations);
	if(!err)
		{
		aBitmask.SetAbsoluteVolumeApiFeatures(operations);
		aAbsoluteVolumeSupported = ETrue;
		}
	else if(err != KErrNotSupported)
		{
		operations.Close();
		return err;
		}
	
	err = iControlBearerObserver.SupportedOperations(aId, TUid::Uid(KRemConNowPlayingApiUid), operations);
	if(!err)
		{
		aBitmask.SetNowPlayingApiFeatures(operations);
		}
	else if(err != KErrNotSupported)
		{
		operations.Close();
		return err;
		}
	
	err = iControlBearerObserver.SupportedOperations(aId, TUid::Uid(KRemConMediaBrowseApiUid), operations);
	if(!err)
		{
		aSdpFeatures |= AvrcpSdp::EBrowsing;
		aBitmask.SetMediaBrowseApiFeatures(operations);
		}
	
	operations.Close();
	return err == KErrNotSupported ? KErrNone : err;
	}

void CAvrcpPlayerInfoManager::UpdateTgServiceRecordL()
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD

	TUint16 features = AvrcpSdp::KAvrcpBaseTgFeatures;
	for(TInt i=0; i<iPlayers.Count(); i++)
		{
		if(ValidPlayer(i))
			{
			features |= iPlayers[i].iSdpFeatures;
			}
		}
	
	TUint16 avrcpVersion = (features & AvrcpSdp::EBrowsing) ? AvrcpSdp::KAvrcpProfileVersion14 : AvrcpSdp::KAvrcpProfileVersion13;
	TUint16 avctpVersion = (avrcpVersion == AvrcpSdp::KAvrcpProfileVersion14) ? AvrcpSdp::KAvctpProtocolVersion13 : AvrcpSdp::KAvctpProtocolVersion12;
	
	AvrcpSdpUtils::UpdateProtocolDescriptorListL(iSdpDatabase, iTargetRecord, avctpVersion);
	if(avrcpVersion == AvrcpSdp::KAvrcpProfileVersion14)
		{
		AvrcpSdpUtils::UpdateAdditionalProtocolDescriptorListL(iSdpDatabase, iTargetRecord);
		}
	
	AvrcpSdpUtils::UpdateProfileDescriptorListL(iSdpDatabase, iTargetRecord, avrcpVersion);
	AvrcpSdpUtils::UpdateBrowseListL(iSdpDatabase, iTargetRecord);
	AvrcpSdpUtils::UpdateSupportedFeaturesL(iSdpDatabase, iTargetRecord, AvrcpSdp::ERemoteControlTarget, features);
	}

void CAvrcpPlayerInfoManager::UidCounterUpdate()
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD
	
	for(TInt i=0; i<iPlayers.Count(); i++)
		{
		if(ValidPlayer(i))
			{
			iLock.Wait();
			TUint16 currentUidCounter = iPlayers[i].iUidCounter;
			TUint16 lastUpdate = iPlayers[i].iLastUpdatedUidCounter;
			iPlayers[i].iLastUpdatedUidCounter = currentUidCounter;
			TRemConClientId clientId = iPlayers[i].iId;
			iLock.Signal();
			
			if(currentUidCounter != lastUpdate)
				{
				for(TInt j=0; j<iObservers.Count(); j++)
					{
					// Observer can request most up to date value if it wants it
					iObservers[j]->MpcoUidCounterChanged(clientId);
					}
				}
			}
		}
	}

/**
If the client ID is set to a valid ID then we shall return the support
status for the specific player referenced by the ID.
Otherwise we shall return generic support which will indicate support across
the device.
@return whether absolute volume control is supported either by the specific player
associated with a client ID, or generally by the device if an invalid client ID is
provided.
 */
TBool CAvrcpPlayerInfoManager::AbsoluteVolumeSupportedL(const TRemConClientId& aClientId) const
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD

	iLock.Wait();
	CleanupSignalPushL(iLock);
	
	TBool supported = EFalse;
	// If we receive a "NULL" client ID then it means that we should 
	// return whether abs vol is generically supported by the device.
	if(aClientId == KNullClientId)
		{
		// Try to find the first player supporting abs vol, if there is one then it is supported 
		TInt index = iPlayers.Find(EFirstAbsVolSupport, FirstAbsVolSupport);
		supported = (index >= 0);
		}
	else
		{
		// The abs vol support for a specific player is required, so return that.
		TInt index = iPlayers.Find(aClientId, PlayerCompare);
		if(index < 0)
			{
			LEAVEL(KErrNotFound);
			}
		supported = iPlayers[index].iAbsoluteVolumeSupport;
		}
	
	CleanupStack::PopAndDestroy(&iLock);
	
	return supported;
	}

TBool CAvrcpPlayerInfoManager::BrowsingSupportedL(const TRemConClientId& aClientId) const
	{
	LOG_FUNC
	ASSERT_CONTROL_THREAD

	iLock.Wait();
	CleanupSignalPushL(iLock);
	
	TBool supported = EFalse;
	// If we receive a "NULL" client ID then it means that we should 
	// return whether browsing is generically supported by the device.
	if(aClientId == KNullClientId)
		{
		// Try to find the first player supporting browsing, if there is one then it is supported 
		TInt index = iPlayers.Find(EFirstBrowsingSupport, FirstBrowsingSupport);
		supported = (index >= 0);
		}
	else
		{
		// The browsing support for a specific player is required, so return that.
		TInt index = iPlayers.Find(aClientId, PlayerCompare);
		if(index < 0)
			{
			LEAVEL(KErrNotFound);
			}
		supported = iPlayers[index].iSdpFeatures & AvrcpSdp::EBrowsing;
		}
	
	CleanupStack::PopAndDestroy(&iLock);
	
	return supported;
	}

TInt CAvrcpPlayerInfoManager::PlayerUpdateCallBack(TAny* aPlayerInfoMgr)
	{
	static_cast<CAvrcpPlayerInfoManager*>(aPlayerInfoMgr)->UidCounterUpdate();
	return KErrNone;
	}

