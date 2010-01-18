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

#include <remconmediabrowsetypes.h>
#include <remcondatabaseawarenowplayingtargetobserver.h>
#include <remcondatabaseunawarenowplayingtargetobserver.h>
#include <remcon/remconnowplayingtargetbase.h>
#include <remcon/avrcpspec.h>
#include <remconinterfaceselector.h>
#include <bluetooth/logger.h>

#include "mediabrowse.h"
#include "nowplaying.h"
#include "remconnowplayingfault.h"
#include "remconqueuemessage.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_REMCONNOWPLAYINGAPI);
_LIT8(KLogFormat, "Operation Id = 0x%x, Data Lengh = %d");
#endif

CRemConNowPlayingTargetBase::CRemConNowPlayingTargetBase(
	CRemConInterfaceSelector& aInterfaceSelector,
	MRemConDatabaseAwareNowPlayingTargetObserver& aObserver)
	: CRemConInterfaceBase(
	TUid::Uid(KRemConNowPlayingApiUid),
	KMaxLengthNowPlayingMsg,
	aInterfaceSelector,
	ERemConClientTypeTarget), 
	iDaObserver(&aObserver),
	iInterfaceSelector(aInterfaceSelector),
	iInProgress(EFalse)
	{
	}
		
CRemConNowPlayingTargetBase::CRemConNowPlayingTargetBase(
	CRemConInterfaceSelector& aInterfaceSelector,
	MRemConDatabaseUnawareNowPlayingTargetObserver& aObserver)
	: CRemConInterfaceBase(
	TUid::Uid(KRemConNowPlayingApiUid),
	KMaxLengthNowPlayingMsg,
	aInterfaceSelector,
	ERemConClientTypeTarget), 
	iDuObserver(&aObserver),
	iInterfaceSelector(aInterfaceSelector),
	iInProgress(EFalse)
	{
	}
	
void CRemConNowPlayingTargetBase::BaseConstructL()
	{
	iMsgQueue = new(ELeave)TRemConMessageQueue;
	TCallBack cb(&NextMessageCb, this);
	iNextMessageCallBack = new (ELeave) CAsyncCallBack(cb,
			CActive::EPriorityStandard);
		
	iOutBuf.CreateL(KMaxLengthNowPlayingMsg);
	
    // Mandate the following features supported.
	RRemConInterfaceFeatures features;
	User::LeaveIfError(features.Open());
	CleanupClosePushL(features);

	features.AddOperationL(EPlayItem);
	features.AddOperationL(EAddToNowPlaying);
	features.AddOperationL(ERegisterNotification);
	
    CRemConInterfaceBase::BaseConstructL(features);
    CleanupStack::PopAndDestroy(&features);
	}
	

EXPORT_C CRemConNowPlayingTargetBase::~CRemConNowPlayingTargetBase()
	{
	iOutBuf.Close();

	iNextMessageCallBack->Cancel();
	delete iNextMessageCallBack;
	
	iMsgQueue->Reset();
	delete iMsgQueue;
	}

/** 
@internalComponent
@released

Gets a pointer to a specific interface version.

@return A pointer to the interface, NULL if not supported.
*/
TAny* CRemConNowPlayingTargetBase::GetInterfaceIf(TUid aUid)
	{
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConInterfaceIf2) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConInterfaceIf2*>(this)
			);
		}

	return ret;
	}

/**
Provide a response to the play item command. For database aware clients this 
would have been called via MrcdanptoPlayItem(). For database unaware client
this would have been called via MrcdunptoPlayItem()

The client must call NowPlayingContentChanged() if the content of the now 
playing list has been changed after the client successfully plays the item
and provides the response with KErrNone through this function.

@param aErr The result of play item operation.
     - KErrNone if the operation was successful.
     - KErrInvalidMediaLibraryStateCookie if the client is able to maintain 
       unique UIDs accross the entire virtual filesystem, but the media 
       library state cookie provided does not match the client's state cookie.
     - KErrInvalidUid if the uid of the item provided does not refer to any 
       valid item.
     - KErrNowPlayingUidIsADirectory if the uid of the item provided refers to
       a directory which can not be handled by this media player.
     - KErrNowPlayingMediaInUse if the item is in use and can not be played.
     - System wide error code otherwise.

@see NowPlayingContentChanged()
*/
EXPORT_C void CRemConNowPlayingTargetBase::PlayItemResponse(TInt aErr)
	{
	TUint8 status = RAvrcpIPC::SymbianErrToStatus(aErr);
	SendResponse(status, EPlayItem);
	
	iInProgress = EFalse;		
	if (!iMsgQueue->IsEmpty())
		{
		iNextMessageCallBack->CallBack();
		}
	}

/**
Sends the response for the request MrcnptoAddToNowPlaying()

The client must call NowPlayingContentChanged() if the content of the now 
playing list has been changed after the client successfully adds the item
to the now playing list and provides the response with KErrNone through 
this function.

param aErr The result of add to now playing operation.
    - KErrNone if the operation was successful.
    - KErrInvalidMediaLibraryStateCookie if the client is able to maintain 
      unique UIDs accross the entire virtual filesystem, but the media 
      library state cookie provided does not match the client's state cookie.
    - KErrInvalidUid if the uid of the item provided does not 
      refer to any valid item.
    - KErrNowPlayingUidIsADirectory if the uid of the item provided refers to
      a directory which can not be handled by this media player.
    - KErrNowPlayingMediaInUse if the item is in use and can not be added to 
      now playing list.
    - KErrNowPlayingListFull if the now playing list if full and no more items
      can be added.
    - System wide error code otherwise.

@see NowPlayingContentChanged()
*/
EXPORT_C void CRemConNowPlayingTargetBase::AddToNowPlayingResponse(TInt aErr)
	{
	TUint8 status = RAvrcpIPC::SymbianErrToStatus(aErr);
	SendResponse(status, EAddToNowPlaying);

	iInProgress = EFalse;		
	if (!iMsgQueue->IsEmpty())
		{
		iNextMessageCallBack->CallBack();
		}
	}

/**
Called by the client in the case that the now playing content changed.

@see PlayItemResponse()
@see AddToNowPlayingResponse()
*/
EXPORT_C void CRemConNowPlayingTargetBase::NowPlayingContentChanged()
	{
	if(iNotificationPending)
		{
		iNotificationPending = EFalse;
		SendNotificationResponse(ERemConNotifyResponseChanged);
		}
	}

void CRemConNowPlayingTargetBase::SendResponse(TUint8 aStatus, TUint aOperationId)
	{
	TInt error = KErrNone;

	// Formats the response in a RRemConNowPlayingResponse
	RRemConNowPlayingResponse response;
	response.iStatus = aStatus;
	
	TRAP(error, response.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		// Send the response back to the CT
		error = InterfaceSelector().SendUnreliable(
				TUid::Uid(KRemConNowPlayingApiUid),
				aOperationId, ERemConResponse, iOutBuf );
		}

	}

void CRemConNowPlayingTargetBase::SendError(TInt aError, TUint aOperationId)
	{
	TInt error = KErrNone;	
	RAvrcpIPCError response;
	response.iError = RAvrcpIPC::SymbianErrorCheck(aError);
	TRAP(error, response.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConNowPlayingApiUid),
				aOperationId, ERemConResponse, iOutBuf);
		}
	}

// From MRemConInterfaceIf2
void CRemConNowPlayingTargetBase::MrcibNewMessage(
		TUint aOperationId, 
		const TDesC8& aData, 
		TRemConMessageSubType aMsgSubType)
	{
	LOG_FUNC
	LOG2(KLogFormat, aOperationId, aData.Length());

	TMetadataTransferPDU currentOp =
	RAvrcpIPC::GetPDUIdFromIPCOperationId(aOperationId);
	switch(currentOp)
		{
	case EPlayItem:
		if ( !iInProgress && 
			 !iMsgQueue->Find(TUid::Uid(KRemConNowPlayingApiUid), EPlayItem))
			{
			ProcessPlayItem(aData);
			}
		else
			{
			AddToOperationQueue(EPlayItem, aData);
			}
		break;
	case EAddToNowPlaying:
		if ( !iInProgress && 
			 !iMsgQueue->Find(TUid::Uid(KRemConNowPlayingApiUid), EAddToNowPlaying))
			{
			ProcessAddToNowPlaying(aData);
			}
		else
			{
			AddToOperationQueue(EAddToNowPlaying, aData);
			}
		break;
	case ERegisterNotification:
		{
		// obtain eventId from aOperationId
		TRegisterNotificationEvent eventId = 
		RAvrcpIPC::GetEventIdFromIPCOperationId(aOperationId);
		
		// If asserted here, must something wrong occured in the 
		// remconbearer handling
		__ASSERT_DEBUG(eventId == ERegisterNotificationNowPlayingContentChanged
				, NowPlayingFault::Fault(EInvalidEventId));
        
		// register for Notifications
		if (aMsgSubType == ERemConNotifyCommandAwaitingInterim)
			{
			ProcessGetStatusAndBeginObserving();
			}
		else if (aMsgSubType == ERemConNotifyCommandAwaitingChanged)
			{
			ProcessGetStatus();
			}
	 	break;
		}
	default:
		break;
		};
	}
	
void CRemConNowPlayingTargetBase::ProcessPlayItem(const TDesC8& aData)
	{
	TUint64 item;
	TRemConFolderScope scope;
	TUint16 uidCounter;
	
	if (!ProcessNowPlaying(aData, EPlayItem, item, scope, uidCounter))
		{
		return;
		}
	
	iInProgress = ETrue;
	if (iDaObserver && (uidCounter > 0))	
		{
		iDaObserver->MrcdanptoPlayItem(item, scope, uidCounter);
		}
	else if (!iDaObserver && (0 == uidCounter))
		{
		iDuObserver->MrcdunptoPlayItem(item, scope);
		}
	else
		{
		PlayItemResponse(KErrAvrcpAirInvalidParameter);
		}
	}

void CRemConNowPlayingTargetBase::ProcessAddToNowPlaying(const TDesC8& aData)
	{
	TUint64 item;
	TRemConFolderScope scope;
	TUint16 uidCounter;
	
	if (!ProcessNowPlaying(aData, EAddToNowPlaying, item, scope, uidCounter))
		{
		return;
		}

	iInProgress = ETrue;
	if (iDaObserver && (uidCounter > 0))
		{
		iDaObserver->MrcdanptoAddToNowPlaying(item, scope, uidCounter);
		}
	else if (!iDaObserver && (0 == uidCounter))
		{
		iDuObserver->MrcdunptoAddToNowPlaying(item, scope);
		}
	else
		{
		AddToNowPlayingResponse(KErrAvrcpAirInvalidParameter);
		}
	}

TBool CRemConNowPlayingTargetBase::ProcessNowPlaying(
		const TDesC8& aData, 
		TUint aOperationId,
		TUint64& aItem, 
		TRemConFolderScope& aScope, 
		TUint16& aMediaLibraryStateCookie)
	{
	RRemConNowPlayingRequest request;
	TRAPD(error, request.ReadL(aData))
	if (error != KErrNone)
		{
		// Nothing in packet
		SendError(KErrAvrcpMetadataParameterNotFound, aOperationId);
		return EFalse;
		}
    
	aItem = request.iElement;
	aScope = request.iScope;
	aMediaLibraryStateCookie = request.iUidCounter;
	
	//Does not allow invalid scope
	if (   aScope != EBrowseFolder 
		&& aScope != ESearchResultFolder
		&& aScope != ENowPlayingFolder )
		{
		SendError(KErrAvrcpAirInvalidScope, aOperationId);
		return EFalse;
		}
	return ETrue;
	}

void CRemConNowPlayingTargetBase::ProcessGetStatusAndBeginObserving()
	{
	// and request another notification (if there is not one already pending)
	// on the next state change  
	iNotificationPending = ETrue;
		
	// send the current status
	SendNotificationResponse(ERemConNotifyResponseInterim);
	}

void CRemConNowPlayingTargetBase::ProcessGetStatus()
	{
	// send the current value
	SendNotificationResponse(ERemConNotifyResponseChanged);
	}

void CRemConNowPlayingTargetBase::SendNotificationResponse(
		TRemConMessageSubType aMsgSubType)
	{
	LOG_FUNC

	TInt error = KErrNone;

	// No extra data needed
	RAvrcpIPCError response;
	response.iError = KErrNone;
	TRAP(error, response.WriteL(iOutBuf));

	if (error == KErrNone)
		{
		// send the response back to the CT
		TUint operationId = 
		RAvrcpIPC::SetIPCOperationIdFromEventId(
				ERegisterNotificationNowPlayingContentChanged);
		
		error = InterfaceSelector().SendUnreliable(
				TUid::Uid(KRemConNowPlayingApiUid),
				operationId, ERemConResponse, aMsgSubType, iOutBuf);
		}
	}

TInt CRemConNowPlayingTargetBase::NextMessageCb(TAny* aThis)
	{
	static_cast<CRemConNowPlayingTargetBase*>(aThis)->DoNextMessage();
	return KErrNone;
	}

void CRemConNowPlayingTargetBase::DoNextMessage()
	{
	CRemConQueuedMessage* msg = iMsgQueue->First();
	iMsgQueue->Remove(*msg);
	
	switch (msg->iOperationId)
		{
		case EPlayItem:
			ProcessPlayItem(msg->Data());
			break;
		case EAddToNowPlaying:
			ProcessAddToNowPlaying(msg->Data());
			break;
		default:
			// Never come here.
			ASSERT(NULL);
			break;
		}
	delete msg;
	}

void CRemConNowPlayingTargetBase::AddToOperationQueue(TInt aOperationId, 
		const TDesC8& aData)
	{
	CRemConQueuedMessage* msg = NULL;
	TRAPD(err, msg = CRemConQueuedMessage::NewL(
			TUid::Uid(KRemConNowPlayingApiUid), aData, aOperationId));			
	if (err == KErrNone)
		{
		iMsgQueue->AddLast(*msg);
		}
	else
		{
		SendError(KErrAvrcpMetadataInternalError, aOperationId);
		}
	}

