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
 @publishedAll
 @released
*/

#include <e32def.h>
#include <absolutevolumeapicontroller.h>
#include <absolutevolumeapicontrollerobserver.h>
#include <remconinterfaceselector.h>
#include "absolutevolumesender.h"
#include <absolutevolumeutils.h>
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_REMCONABSOLUTEVOLUME);
#endif

#ifdef _DEBUG
_LIT(KAbsoluteVolumeControllerFaultName, "AbsVolFault");
// The panic codes associated with category KAbsoluteVolumeControllerFaultName are line numbers in this file.
#endif

/**
Allocates and constructs a new CRemConAbsoluteVolumeController object

@param aInterfaceSelector The interface selector. The client must have 
       created one of these first.
@param aObserver The observer. The client must have implemented the observer,
       owned by the client.
@param aMaxVolume The client maximum volume.
@return A new CRemConAbsoluteVolumeController, owned by the interface selector
@panic AbsoluteVolumeController 0 if aMaxVolume is zero.
*/
EXPORT_C 
CRemConAbsoluteVolumeController* CRemConAbsoluteVolumeController::NewL(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConAbsoluteVolumeControllerObserver& aObserver,
		TUint32 aMaxVolume)
	{
	LOG_STATIC_FUNC
	
	__ASSERT_ALWAYS(aMaxVolume > 0, 
				User::Panic(KAbsoluteVolumeControllerPanicName, 
						EControllerInvalidMaxVolume)
				);
				
	CRemConAbsoluteVolumeController* self = 
	new(ELeave) CRemConAbsoluteVolumeController(aInterfaceSelector, 
			aObserver, aMaxVolume);
	
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/** 
Constructor.

@param aInterfaceSelector The interface selector.
@param aObserver The observer.
*/
CRemConAbsoluteVolumeController::CRemConAbsoluteVolumeController(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConAbsoluteVolumeControllerObserver& aObserver,
		TUint32 aMaxVolume)
:	CRemConInterfaceBase(TUid::Uid(KRemConAbsoluteVolumeControllerApiUid), 
		KAbsoluteVolumeResponseDataSize,
		aInterfaceSelector,
		ERemConClientTypeController),
    iObserver(aObserver),
    iClientMaxVolume(aMaxVolume),
	iNotificationRequested(EFalse),
	iAbsVolRequest(ENotInUse),
	iNotifyRequest(ENotInUse)
	{
	LOG_FUNC
	}

EXPORT_C CRemConAbsoluteVolumeController::~CRemConAbsoluteVolumeController()
    {
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	delete iAbsVolSender;
    }

void CRemConAbsoluteVolumeController::ConstructL()
	{
	LOG_FUNC
	
	iAbsVolSender = CAbsVolSender::NewL(InterfaceSelector(), *this);
	BaseConstructL();
	}

/**
Sets an absolute volume on the target device,
Any responses will be returned via the observer interface.

@pre The send of any previous SetAbsoluteVolume command has completed.
@param aStatus Indicates the completion of the send request.  The client must
			   not block execution by using User::WaitForRequest to await 
			   completion of this send.
@param aVolume The relative volume against the client max volume.
@param aNumRemotes The number of remotes to which the command was sent.
@panic AbsoluteVolumeController 1, if aVolume greater than the client 
       max volume.
*/
EXPORT_C void CRemConAbsoluteVolumeController::SetAbsoluteVolume(
		TRequestStatus& aStatus, 
		TUint32 aVolume, 
		TUint& aNumRemotes)
	{
	LOG_FUNC
	LOG1(_L8("\taVolume = %d"), aVolume);
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	__ASSERT_ALWAYS(aVolume <= iClientMaxVolume, 
			User::Panic(KAbsoluteVolumeControllerPanicName, 
					EControllerVolumeBeyondMaxVolume)
			);
	__ASSERT_DEBUG(iAbsVolRequest == ENotInUse, User::Panic(KAbsoluteVolumeControllerPanicName, 
				EMultipleSetAbsoluteVolumes));
	
	RRemConAbsoluteVolumeRequest setAbsVol;
	setAbsVol.iVolume = aVolume;
	setAbsVol.iMaxVolume = iClientMaxVolume;
	TRAPD(err, setAbsVol.WriteL(iSetData));
	if (err == KErrNone)
		{
		// Store the client's info so we can complete their request later
		aStatus = KRequestPending;
		iAbsVolRequest = EPending;
		__ASSERT_DEBUG(iClientStatus == NULL, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
		iClientStatus = &aStatus;
		__ASSERT_DEBUG(iClientNumRemotes == NULL, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
		iClientNumRemotes = &aNumRemotes;
		KickOffSendIfNeeded();
		}
	else
		{
		iObserver.MrcavcoSetAbsoluteVolumeResponse(0, 0, err);
		}
	}

EXPORT_C void CRemConAbsoluteVolumeController::CancelSetAbsoluteVolume()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	switch ( iAbsVolRequest )
		{
	case ENotInUse:
		// Nothing to do.
		break;
	case ESending:
		iAbsVolSender->Cancel();
		SetAbsoluteVolumeSendComplete(KErrCancel);
		break;
	case EPending:
		SetAbsoluteVolumeSendComplete(KErrCancel);
		break;
	default:
		__ASSERT_DEBUG(EFalse, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
		break;
		}
	}

/**
Requests notification when the volume on the target device changes,
Any responses will be returned via the observer interface.

Volume changes will continue to be provided until either the 
CancelAbsoluteVolumeNotification function is called, or MrcavcoCurrentVolume
is called on the client with an error.

@see MRemConAbsoluteVolumeControllerObserver::MrcavcoCurrentVolume
@see CRemConAbsoluteVolumeController::CancelAbsoluteVolumeNotification
@pre The client is not currently registered to receive absolute volume notifications.
*/
EXPORT_C 
void CRemConAbsoluteVolumeController::RegisterAbsoluteVolumeNotification()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	// The request is outstanding, so not allowed to register again.
	__ASSERT_DEBUG(!iNotificationRequested, User::Panic(KAbsoluteVolumeControllerPanicName, EAbsoluteVolumeNotificationAlreadyRegistered));
	__ASSERT_DEBUG(iNotifyRequest == ENotInUse, User::Panic(KAbsoluteVolumeControllerPanicName, EMultipleNotifies));
	
	RRemConAbsoluteVolumeRequest absVol;
	TRAPD(err, absVol.WriteL(iNotifyData));
	if (err == KErrNone)
		{
		iNotifyRequest = EPending;
		iNotificationRequested = ETrue;
		KickOffSendIfNeeded();
		}
	else
		{
		iObserver.MrcavcoAbsoluteVolumeNotificationError();
		}
	}

/**
Called by the client to tell the controller that the client doesn't wish to 
receicve the volume change notification
until the client re-register again.
*/
EXPORT_C 
void CRemConAbsoluteVolumeController::CancelAbsoluteVolumeNotification()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	if ( iNotifyRequest == ESending )
		{
		iAbsVolSender->Cancel();
		}
	
	iNotificationRequested = EFalse;
	RegisterNotifySendComplete();
	}

TAny* CRemConAbsoluteVolumeController::GetInterfaceIf(TUid aUid)
    {
	LOG_FUNC
	
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConInterfaceIf2) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConInterfaceIf2*>(this)
			);
		}
	
	return ret;
    }

void CRemConAbsoluteVolumeController::MrcibNewMessage(TUint aOperationId, 
		const TDesC8& aData, 
		TRemConMessageSubType aMessageSubType)
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	switch (aOperationId)
	    {
	case KRemConAbsoluteVolumeNotification:
		HandleNotify(aData, aMessageSubType);
	    break;
	case KRemConSetAbsoluteVolume:
    	HandleSetAbsoluteVolumeResponse(aData);
	    break;
	default:	   
	    break;
	    }
	}

/** 
Process the 'volume changed notification response'

@param aData The response data.
@param aMessageSubType The RemCon submessage type.
*/
void CRemConAbsoluteVolumeController::HandleNotify(const TDesC8& aData,
		TRemConMessageSubType aMessageSubType)
    {
	LOG_FUNC
	
    TInt err = KErrNone;
    
    if(iNotificationRequested)
    	{
	    RRemConAbsoluteVolumeResponse absVol;
	    TRAP(err, absVol.ReadL(aData))
	    if (err == KErrNone)
	    	{
	    	if (absVol.iError == KErrNone)
	    		{
	    		switch ( aMessageSubType )
	    	        {
	    	    case ERemConNotifyResponseInterim:
	    	    	{
	    	    	VolumeUpdate(absVol.iVolume, absVol.iMaxVolume);
		    	    break;
	    	    	}	    	    
	    	    case ERemConNotifyResponseChanged:
		    		{
	    	    	VolumeUpdate(absVol.iVolume, absVol.iMaxVolume);

		    		// Register notification again.
		    		iNotificationRequested = EFalse;
		    		RegisterAbsoluteVolumeNotification();
		    	    break;
		    		}	    			
	    	    default:    	    
	    	        break;
	    	        }//switch
	    		}
	    	else //Error response
	    		{
	    		iNotificationRequested = EFalse;
	    		iObserver.MrcavcoAbsoluteVolumeNotificationError();
	    		}
	    	}
	    else
	    	{
    		iNotificationRequested = EFalse;
    		iObserver.MrcavcoAbsoluteVolumeNotificationError();
	    	}
    	}
    }

/** 
Process the 'set absolute volume response'
@param aData The response data.
*/
void CRemConAbsoluteVolumeController::HandleSetAbsoluteVolumeResponse( 
		const TDesC8& aData )
    {
	LOG_FUNC
	
    TInt err = KErrNone;
    RRemConAbsoluteVolumeResponse absVol;
    TRAP(err, absVol.ReadL(aData))
    if (err == KErrNone)
    	{
    	iCurrentVolume = absVol.iVolume;
    	iCurrentMaxVolume = absVol.iMaxVolume;
    	iObserver.MrcavcoSetAbsoluteVolumeResponse(absVol.iVolume, 
    			absVol.iMaxVolume,
    			absVol.iError);
    	}
    else
    	{
    	iObserver.MrcavcoSetAbsoluteVolumeResponse(0, 
    			0,
    			err);
    	}
    }

void CRemConAbsoluteVolumeController::MavsoSendComplete(TInt aResult)
	{
	LOG_FUNC
	LOG1(_L8("\taResult = %d"), aResult);
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	if ( iAbsVolRequest == ESending )
		{
		__ASSERT_DEBUG(iNotifyRequest != ESending, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
		SetAbsoluteVolumeSendComplete(aResult);
		}
	else if ( iNotifyRequest == ESending )
		{
		// This updates our own state and kicks off any pending send. The 
		// later client upcall (if there was an error) gives them a chance 
		// to make further calls on us.
		RegisterNotifySendComplete();

		if(aResult != KErrNone)
			{
			iNotificationRequested = EFalse;
			iObserver.MrcavcoAbsoluteVolumeNotificationError();
			}
		}
	else
		{
		// Send complete with no send outstanding.
		__ASSERT_DEBUG(EFalse, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
		}
	}

void CRemConAbsoluteVolumeController::KickOffSendIfNeeded()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	if ( iAbsVolRequest == ESending || iNotifyRequest == ESending )
		{
		// Any pending send will be kicked off when current send completes and this function is called again.
		return;
		}
		
	if ( iAbsVolRequest == EPending )
		{
		SendSetAbsoluteVolume();
		}
	else if ( iNotifyRequest == EPending )
		{
		SendNotify();
		}
	}

void CRemConAbsoluteVolumeController::SetAbsoluteVolumeSendComplete(TInt aResult)
	{
	LOG_FUNC
	LOG1(_L8("\taResult = %d"), aResult);
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	__ASSERT_DEBUG(iClientStatus, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
	User::RequestComplete(iClientStatus, aResult);
	iClientStatus = NULL;
	iClientNumRemotes = NULL;
	iSetData.SetLength(0);
	__ASSERT_DEBUG(iAbsVolRequest != ENotInUse, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
	iAbsVolRequest = ENotInUse;
	
	KickOffSendIfNeeded();
	}

void CRemConAbsoluteVolumeController::RegisterNotifySendComplete()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);

	iNotifyData.SetLength(0);
	// This method doesn't actually complete a client request so we don't bother asserting state.
	iNotifyRequest = ENotInUse;
	
	KickOffSendIfNeeded();
	}

void CRemConAbsoluteVolumeController::VolumeUpdate(TUint32 aVolume, TUint32 aMaxVolume)
	{
	LOG_FUNC
	LOG2(_L8("\taVolume = %d, aMaxVolume = %d"), aVolume, aMaxVolume);
	
	// Only update the client if the volume has changed
    if (aVolume != iCurrentVolume || aMaxVolume != iCurrentMaxVolume)
	    {
	    iCurrentVolume = aVolume; //store the new value
	    iCurrentMaxVolume = aMaxVolume;
	    
	    iObserver.MrcavcoCurrentVolume(aVolume,
	    		aMaxVolume,
	    		KErrNone);
	    }
	}

void CRemConAbsoluteVolumeController::SendSetAbsoluteVolume()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	__ASSERT_DEBUG(iClientNumRemotes, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
	iAbsVolSender->SendSetAbsoluteVolume(*iClientNumRemotes, iSetData);
	__ASSERT_DEBUG(iAbsVolRequest == EPending, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
	iAbsVolRequest = ESending;
	}

void CRemConAbsoluteVolumeController::SendNotify()
	{
	LOG_FUNC
	LOG2(_L8("\tiAbsVolRequest = %d, iNotifyRequest = %d"), iAbsVolRequest, iNotifyRequest);
	
	iAbsVolSender->SendNotify(iNotifyData);
	__ASSERT_DEBUG(iNotifyRequest == EPending, User::Panic(KAbsoluteVolumeControllerFaultName, __LINE__));
	iNotifyRequest = ESending;
	}
