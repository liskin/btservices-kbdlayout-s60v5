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

#include <absolutevolumeapitarget.h>
#include <absolutevolumeapitargetobserver.h>
#include <remconinterfaceselector.h>
#include <absolutevolumeapi.h>
#include <absolutevolumeutils.h>
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_REMCONABSOLUTEVOLUME);
_LIT8(KLogFormat, "Operation Id = 0x%x, Data Lengh = %d");
_LIT8(KLogNewL, "CRemConAbsoluteVolumeTarget::NewL");
#endif

/**
Allocates and constructs a new CRemConAbsoluteVolumeTarget object

@param aInterfaceSelector The interface selector. The client must have 
       created one of these first.
@param aObserver The observer through which the client will receive absolute
       volume commands from M class MRemConAbsoluteVolumeTargetObserver.
@param aVolume The initial relative volume on the client.
@param aMaxVolume The client maximum volume against which aVolume is relative.
@return A new CRemConAbsoluteVolumeTarget, owned by the interface selector.
@panic AbsoluteVolumeTarget 0 if aMaxVolume is zero
       AbsoluteVolumeTarget 1 if aVolume greater than aMaxVolume
       
*/
EXPORT_C CRemConAbsoluteVolumeTarget* CRemConAbsoluteVolumeTarget::NewL(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConAbsoluteVolumeTargetObserver& aObserver,
		TUint32 aVolume, 
		TUint32 aMaxVolume)
	{
	LOG(KLogNewL);
	
	__ASSERT_ALWAYS(aMaxVolume > 0, 
			User::Panic(KAbsoluteVolumeTargetPanicName, 
					ETargetInvalidMaxVolume)
			);
	__ASSERT_ALWAYS(aVolume <= aMaxVolume, 
			User::Panic(KAbsoluteVolumeTargetPanicName, 
					ETargetVolumeBeyondMaxVolume)
			);
	
	CRemConAbsoluteVolumeTarget* self = 
	new(ELeave) CRemConAbsoluteVolumeTarget(aInterfaceSelector, 
			aObserver, aVolume, aMaxVolume);
	
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/** 
Constructor.
@param aInterfaceSelector The interface selector.
@param aObserver The observer of this interface.
@param aVolume The initial relative volume on the client
@param aMaxVolume The maximum volume on the client against which 
       aVolume is relative.
*/
CRemConAbsoluteVolumeTarget::CRemConAbsoluteVolumeTarget(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConAbsoluteVolumeTargetObserver& aObserver,
		TUint32 aVolume, 
		TUint32 aMaxVolume)
:	CRemConInterfaceBase(TUid::Uid(KRemConAbsoluteVolumeTargetApiUid),
		KAbsoluteVolumeRequestDataSize, 
		aInterfaceSelector,
		ERemConClientTypeTarget), 
	iObserver(aObserver),
	iAbsoluteVolumeNotificationRequest(EFalse),
	iClientVolume(aVolume),
	iClientMaxVolume(aMaxVolume)
	{
	}
	
void CRemConAbsoluteVolumeTarget::ConstructL()
	{
	iOutBuf.CreateL(KAbsoluteVolumeResponseDataSize);

	//Mandate the following features supported.
	RRemConInterfaceFeatures features;
	User::LeaveIfError(features.Open());
	CleanupClosePushL(features);
	features.AddOperationL(KRemConSetAbsoluteVolume);
	
	BaseConstructL(features);
	CleanupStack::PopAndDestroy(&features);
	}

EXPORT_C CRemConAbsoluteVolumeTarget::~CRemConAbsoluteVolumeTarget()
	{
	iOutBuf.Close();
	}

TAny* CRemConAbsoluteVolumeTarget::GetInterfaceIf(TUid aUid)
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
Called by the client in response to a MrcavtoSetAbsoluteVolume() call.

@param aVolume The relative volume against the client maximum volume.  
@param aErr The error code.
     - KErrNone if the client has changed its absolute volume.
     - System wide error code otherwise.
@panic AbsoluteVolumeTarget 1, if volume is greater than max volume.
*/
EXPORT_C void CRemConAbsoluteVolumeTarget::SetAbsoluteVolumeResponse(
		TUint32 aVolume, 
		TInt aErr)
	{
	__ASSERT_ALWAYS(aVolume <= iClientMaxVolume, 
			User::Panic(KAbsoluteVolumeTargetPanicName, 
					ETargetVolumeBeyondMaxVolume)
			);
	
	RRemConAbsoluteVolumeResponse response;
	response.iError = KErrNone;
	response.iVolume = aVolume;
	response.iMaxVolume = iClientMaxVolume;
	TRAPD(error, response.WriteL(iOutBuf));
	if (error != KErrNone)
		{
		SendError(KErrAbsoluteVolumeInternalError, KRemConSetAbsoluteVolume);
		return;
		}
	
	if (aErr == KErrNone)
		{
		// send the response back to the CT
		error = InterfaceSelector().SendUnreliable(
				TUid::Uid(KRemConAbsoluteVolumeTargetApiUid),
				KRemConSetAbsoluteVolume, ERemConResponse, iOutBuf );
		}
	else
		{
		SendError(KErrAbsoluteVolumeInternalError, KRemConSetAbsoluteVolume);
		}
	}

/** 
Must be called each time the volume changes on the client.

It is used to inform the controller if it has requested updates on the client
volume change.

@param aVolume The relative volume against the client maximum volume. 
@panic AbsoluteVolume 1, if volume greater than the client max volume.
*/
EXPORT_C void CRemConAbsoluteVolumeTarget::AbsoluteVolumeChanged(
		TUint32 aVolume)
	{
	__ASSERT_ALWAYS(aVolume <= iClientMaxVolume, 
			User::Panic(KAbsoluteVolumeTargetPanicName, 
					ETargetVolumeBeyondMaxVolume)
			);
	
	if (aVolume != iClientVolume)
		{
		// Records the current volume each time
		// when the client absolute volume is changed
		iClientVolume = aVolume;
		
		if (iAbsoluteVolumeNotificationRequest)
			{
			iAbsoluteVolumeNotificationRequest = EFalse;
			SendNotificationResponse(ERemConNotifyResponseChanged);
			}
		}
	}

// From MRemConInterfaceIf
void CRemConAbsoluteVolumeTarget::SendError(TInt aError, TUint aOperationId)
	{
	TInt error = KErrNone;
	RRemConAbsoluteVolumeResponse errRsp;
	errRsp.iError = aError;
	TRAP(error, errRsp.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		InterfaceSelector().SendUnreliable(
				TUid::Uid(KRemConAbsoluteVolumeTargetApiUid),
				aOperationId, ERemConResponse, iOutBuf);
		}
	}

void CRemConAbsoluteVolumeTarget::MrcibNewMessage(TUint aOperationId, 
		const TDesC8& aData, 
		TRemConMessageSubType aMsgSubType)
	{
	LOG_FUNC
	LOG2(KLogFormat, aOperationId, aData.Length());

	switch(aOperationId)
		{
	case KRemConSetAbsoluteVolume:
		{
		ProcessSetAbsoluteVolume(aData);
		break;
		}
	case KRemConAbsoluteVolumeNotification:
		{
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

/**
Processes the request for setting absolute volume.

@param aData The absolute volume data to be setted.
*/
void CRemConAbsoluteVolumeTarget::ProcessSetAbsoluteVolume(
		const TDesC8& aData)
	{	
	TInt error;
	RRemConAbsoluteVolumeRequest request;
	TRAP(error, request.ReadL(aData));
	if ( error == KErrNone)
		{
		iObserver.MrcavtoSetAbsoluteVolumeRequest(request.iVolume, 
				request.iMaxVolume);
		}
	else
		{
		SendError(KErrAbsoluteVolumeInternalError, KRemConSetAbsoluteVolume);
		}
	}

/**
Processes the request for notify command waiting interim.
*/
void CRemConAbsoluteVolumeTarget::ProcessGetStatusAndBeginObserving()
	{
	//Flag is ETure to indicate the request for absolute volume change 
	//notification has been received.
	iAbsoluteVolumeNotificationRequest = ETrue;
		
	//send the interim response with the current absolute volume.
	SendNotificationResponse(ERemConNotifyResponseInterim);
	}

/**
Processes the request for notify command waiting changed.
*/
void CRemConAbsoluteVolumeTarget::ProcessGetStatus()
	{
	// send the current value
	SendNotificationResponse(ERemConNotifyResponseChanged);
	}

/**
Sends absolute volume interim or change response according to the message 
type aMsgSubType

@param aMsgSubType The remcon submessage type.
*/
void CRemConAbsoluteVolumeTarget::SendNotificationResponse(
		TRemConMessageSubType aMsgSubType)
	{
	LOG_FUNC

	TInt error = 0;
	RRemConAbsoluteVolumeResponse response;
	response.iError = KErrNone;
	response.iVolume = iClientVolume;
	response.iMaxVolume = iClientMaxVolume;
	TRAP(error, response.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		error = InterfaceSelector().SendUnreliable(
				TUid::Uid(KRemConAbsoluteVolumeTargetApiUid),
				KRemConAbsoluteVolumeNotification, 
				ERemConResponse, 
				aMsgSubType,
				iOutBuf);
		}
	}
