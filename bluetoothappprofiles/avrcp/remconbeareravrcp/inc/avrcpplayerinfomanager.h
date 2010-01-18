// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef AVRCPPLAYERINFOMANAGER_H
#define AVRCPPLAYERINFOMANAGER_H

#include <btsdp.h>
#include <e32base.h>
#include <playerinformationtargetobserver.h>
#include <remconcoreapi.h>
#include <remcon/clientid.h>
#include <remcon/playertype.h>
#include <remcon/remconbearerobserver.h>
#include "avrcpinternalinterface.h"
#include "avrcputils.h"
#include "playerbitmasks.h"
#include "playerstatewatcher.h"

const TInt KAvrcpMediaPlayerItemBaseSize = 28;

NONSHARABLE_CLASS(MPlayerChangeObserver)
	{
public:
	virtual void MpcoAvailablePlayersChanged() = 0;
	virtual void MpcoAddressedPlayerChangedLocally(TRemConClientId aClientId) = 0;
	virtual void MpcoUidCounterChanged(TRemConClientId aClientId) = 0;
	};

NONSHARABLE_STRUCT(TAvrcpMediaPlayerItem)
	{
public:
	TRemConClientId iId;
	TUint8 iPlayerType;
	TUint32 iPlayerSubType;
	TPlayerFeatureBitmask iFeatureBitmask;
	TPtrC8 iName;
	// iUidCounter is set from the bulk thread and read from the control thread
	TUint16 iUidCounter;
	// iLastUpdatedUidCounter is only accessed from the control thread to see if an update
	// is needed
	TUint16 iLastUpdatedUidCounter; 				
	MPlayerEventsObserver::TPlaybackStatus iPlaybackStatus;
	TUint8 iSdpFeatures;
	TBool iBulkClientAvailable;
	// We store absolute volume support because it's returned as part of 
	// a GetCapabilities(EventIds) response.  Some implementations poll
	// for changes to this value so it's probably worthwhile avoiding
	// having to ask RemCon for it repeatedly.
	TBool iAbsoluteVolumeSupport;
	};

class MIncomingCommandHandler;
class MRemConCommandInterface;
class CUidWatcher;
NONSHARABLE_CLASS(CAvrcpPlayerInfoManager) 
	: public CBase, public MUidObserver, public MPlayStatusObserver
	{
//---------------------------------------------------------
// Control thread only functions
//---------------------------------------------------------
public:
	static CAvrcpPlayerInfoManager* NewL(MRemConBearerObserver& aObserver, MRemConCommandInterface& aCommandInterface);
	~CAvrcpPlayerInfoManager();
	
	// Interface to bearer/remcon
	void ClientAvailable(const TRemConClientId& aId, TPlayerType aClientType, TPlayerSubType aClientSubType, const TDesC8& aName);
	void ClientNotAvailable(const TRemConClientId& aId);
	void ClientStatus(TBool aControllerPresent, TBool aTargetPresent);	
	TInt SetLocalAddressedClient(const TRemConClientId& aId);
	void ControllerFeaturesUpdatedL(RArray<TUid>& aSupportedInterfaces);
	MIncomingCommandHandler& InternalCommandHandler();

	// Interface to command handler
	void AddObserverL(MPlayerChangeObserver& aObserver);
	void RemoveObserver(MPlayerChangeObserver& aObserver);
	TUint16 UidCounterL(const TRemConClientId& aClientId) const;
	TBool AbsoluteVolumeSupportedL(const TRemConClientId& aClientId) const;
	TBool BrowsingSupportedL(const TRemConClientId& aClientId) const;

private:
	// Interface to uid counter updates
	static TInt PlayerUpdateCallBack(TAny* aPlayerInfoMgr);
	void UidCounterUpdate();
	
	// from MPlayStatusObserver
	void MpsoPlayStatusChanged(TRemConClientId aId, MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus);
	void MpsoError(TRemConClientId aId);


//---------------------------------------------------------
// Bulk thread only functions
//---------------------------------------------------------
public:	
	// Interface to bearer/remcon
	void BulkClientAvailable(const TRemConClientId& aId);
	void BulkClientNotAvailable(const TRemConClientId& aId);
	MIncomingCommandHandler* BulkStartedL(MRemConCommandInterface& aCommandInterface);
	void BulkStopped();
	
	// Interface to command handler
	void MediaPlayerItemL(const TUint16& aAvrcpPlayerId, RMediaPlayerItem& aItem);


private: // from MUidObserver
	void MuoUidChanged(TRemConClientId aId, TUint16 aUidCounter);
	void MuoError(TRemConClientId aId);



//---------------------------------------------------------
// Common functions
//---------------------------------------------------------
public:
	// Used to map between AVRCP player ids and RemCon client ids
	TRemConClientId ClientL(TUint16 aAvrcpPlayerId) const;
	TRemConClientId Client(TUint16 aAvrcpPlayerId) const;
	TUint16 PlayerL(const TRemConClientId& aClientId) const;
	
	// Used to retrieve list of current players
	TInt PlayerListing(TUint aStartItem, TUint aEndItem, RArray<TUint>& aPlayers);
	TInt PlayerListing(RArray<TUint>& aPlayers);

private:
	CAvrcpPlayerInfoManager(MRemConBearerObserver& aObserver);
	void ConstructL(MRemConCommandInterface& aCommandInterface);
	
	TBool ValidPlayer(const TUint16& aAvrcpPlayerId) const;
	TInt NextPlayerIndex();
	TInt SetPlayerFeatures(const TRemConClientId& aId, TPlayerFeatureBitmask& aBitmask, TUint8& aSdpFeatures, TBool& aAbsoluteVolumeSupported);
	void UpdateTgServiceRecordL();
	
private:
	RPointerArray<MPlayerChangeObserver> iObservers;
	RArray<TAvrcpMediaPlayerItem> iPlayers;
	CUidWatcher* iUidWatcher;
	CPlayStatusWatcher* iPlayStatusWatcher;
	MRemConBearerObserver& iControlBearerObserver; // unowned - used to determine features of players
	RSpecificThreadCallBack iControlThreadCallBack;
	
	RSdp						iSdp;
	RSdpDatabase				iSdpDatabase;
	TSdpServRecordHandle		iControllerRecord;
	TSdpServRecordHandle		iTargetRecord;

	mutable RFastLock iLock; // for use in const functions
	};

// The lock is only used for the following members....
//

#endif //AVRCPPLAYERINFOMANAGER_H
