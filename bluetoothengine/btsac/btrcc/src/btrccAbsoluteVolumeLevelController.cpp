/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  This class listens to notification coming from Remote Control framework.
*				 BTRCC is interested in only those of initiated by BT Remote Control Plugin.	
*
*/


// INCLUDE FILES
#include <remconbeareravrcp.h> //	KRemConBearerAvrcpImplementationUid = 0x1020685f
#include <MediaStreamPropertiesPSKeys.h>
#include <remconinterfaceselector.h> 
#include <remconaddress.h>

#include "btrccAbsoluteVolumeLevelController.h"
#include "debug.h"

const TInt KAbsoluteVolumeLevelSetServiceId = 0x02; 
const TInt KMaxRetries = 3;

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTRCCAbsoluteVolumeLevelController* CBTRCCAbsoluteVolumeLevelController::NewL(CRemConInterfaceSelector& aInterfaceSelector, MBTRCCVolumeControllerObserver& aObserver)
	{
	CBTRCCAbsoluteVolumeLevelController* self = new (ELeave) CBTRCCAbsoluteVolumeLevelController(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL(aInterfaceSelector);
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::CBTRCCAbsoluteVolumeLevelController
// C++ constructor.
// -----------------------------------------------------------------------------
//
CBTRCCAbsoluteVolumeLevelController::CBTRCCAbsoluteVolumeLevelController(MBTRCCVolumeControllerObserver& aObserver)
: CBTRCCVolumeLevelControllerBase(aObserver)
	{
	}

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::ConstructL(CRemConInterfaceSelector& aInterfaceSelector)
	{
   	TRACE_FUNC
   	CBTRCCVolumeLevelControllerBase::ConstructL();
   	iLocalMaxVolume = GetLocalMaxVolume();  
    iInterfaceSelector = &aInterfaceSelector;
   	
    iRemConAbsoluteVolumeController = CRemConAbsoluteVolumeController::NewL(aInterfaceSelector, 
                                                                     *this, 
                                                                     iLocalMaxVolume);    
    TRACE_INFO((_L("CRemConAbsoluteVolumeController created")))
    iBtrccAbsoluteVolumeActive = CBTRCCActive::NewL(*this, KAbsoluteVolumeLevelSetServiceId);
    TRACE_INFO((_L("CBTRCCActive created")))
	}

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTRCCAbsoluteVolumeLevelController::~CBTRCCAbsoluteVolumeLevelController()
	{
	TRACE_FUNC
	delete iBtrccAbsoluteVolumeActive; 
	}

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::DoStart
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::DoStart(TInt aInitialVolume) 
    {
    TRACE_FUNC
    AdjustRemoteVolume(aInitialVolume); 
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::DoStop
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::DoStop() 
    {
    TRACE_FUNC
    iRemConAbsoluteVolumeController->CancelAbsoluteVolumeNotification();
    iRemConAbsoluteVolumeController->CancelSetAbsoluteVolume(); 
    iBtrccAbsoluteVolumeActive->Cancel();
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::DoReset
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::DoReset()
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::AdjustRemoteVolume
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::AdjustRemoteVolume(TInt aVolume)
    {
	TRACE_INFO((_L("Setting volume to %d"), aVolume))
	
	if(iBtrccAbsoluteVolumeActive->IsActive()) 
        {
        iRemConAbsoluteVolumeController->CancelSetAbsoluteVolume(); 
        iBtrccAbsoluteVolumeActive->Cancel();
        }
	
    //Try to register volume change notification
	RegisterVolumeChangeNotification();
    
	// iNumRemotes is not needed by us, but the method requires it. 
    iRemConAbsoluteVolumeController->SetAbsoluteVolume(iBtrccAbsoluteVolumeActive->iStatus, 
            aVolume, iNumRemotes); 
    iBtrccAbsoluteVolumeActive->GoActive();
    }


// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::RegisterVolumeChangeNotification
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::RegisterVolumeChangeNotification()
    {

    iRemConAbsoluteVolumeController->CancelAbsoluteVolumeNotification(); 
    iRemConAbsoluteVolumeController->RegisterAbsoluteVolumeNotification();
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::GetPhoneVolume
// -----------------------------------------------------------------------------
//
TInt CBTRCCAbsoluteVolumeLevelController::GetPhoneVolume(TInt &aVol)
    {
    TRACE_FUNC
    return CBTRCCVolumeLevelControllerBase::GetPhoneVolume(aVol);
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::RequestCompletedL(CBTRCCActive& aActive, TInt aErr)
    {
    TRACE_FUNC
    if( aActive.ServiceId() != KAbsoluteVolumeLevelSetServiceId )
        {
        // This is something the base class can handle. 
        CBTRCCVolumeLevelControllerBase::RequestCompletedL(aActive, aErr);
        }
    else if(aErr && iRetryCounter < KMaxRetries) // else it is KAbsoluteVolumeLevelSetServiceId
        {
        TRACE_INFO((_L("Sending abs vol cmd failed with error %d"), aErr))
        iRetryCounter++;
        AdjustRemoteVolume(GetCurrentLocalVolume());
        }
    else
        {
        if(iRetryCounter >= KMaxRetries)
            {
            VolumeControlError(EVolumeAdjustmentFailed); 
            }
        iRetryCounter = 0;
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::CancelRequest
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::CancelRequest(TInt aServiceId) 
    {
    TRACE_FUNC
    if( aServiceId != KAbsoluteVolumeLevelSetServiceId )
        {
        // This is something the base class can handle. 
        CBTRCCVolumeLevelControllerBase::CancelRequest(aServiceId);
        } 
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::MrcavcoCurrentVolume
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::MrcavcoCurrentVolume(TUint32 aVolume, 
        TUint32 aMaxVolume, 
        TInt aError)
    {
    TRACE_FUNC
    AccessoryChangedVolume(aVolume, aMaxVolume, aError);
    /*if(aError != KErrNone) 
        {
        // Register again to the notifications. 
        iRemConAbsoluteVolumeController->RegisterAbsoluteVolumeNotification();     
        }*/
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::MrcavcoSetAbsoluteVolumeResponse
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::MrcavcoSetAbsoluteVolumeResponse(TUint32 aVolume, 
        TUint32 aMaxVolume, 
        TInt aError)
    {
    TRACE_FUNC
    TRACE_INFO((_L("Accessory volume is %d / %d"), aVolume, aMaxVolume))
    AccessoryChangedVolume(aVolume, aMaxVolume, aError);
    }

// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::MrcavcoAbsoluteVolumeNotificationError
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::MrcavcoAbsoluteVolumeNotificationError()
    {
    TRACE_FUNC
    // This is called when there's some error with writing the commands. 
    // It means that we can't send notifications or responses to those, so 
    // it's better to stop volume controlling and reset the link.  
    VolumeControlError(ERegisterNotificationsFailed); 
    }
// -----------------------------------------------------------------------------
// CBTRCCAbsoluteVolumeLevelController::AccessoryChangedVolume
// -----------------------------------------------------------------------------
//
void CBTRCCAbsoluteVolumeLevelController::AccessoryChangedVolume(TUint32 aVolume, TUint32 aMaxVolume, TInt aError)
    {
    TRACE_FUNC
    TRACE_INFO((_L("Accessory volume is %d / %d"), aVolume, aMaxVolume))
    if( ValidVolumeParams(aVolume, aMaxVolume) && (aError == KErrNone) )
        {
        // Convert volume scale to phone's volume scale. (User reals and round in the end.)  
        TInt volumeInPhoneScale = (TReal)aVolume * ((TReal)iLocalMaxVolume / (TReal)aMaxVolume) + 0.5;
        ASSERT(volumeInPhoneScale <= iLocalMaxVolume);
        TRACE_INFO((_L("Volume in phone scale is %d"), volumeInPhoneScale))
        CBTRCCVolumeLevelControllerBase::AccessoryChangedVolume(volumeInPhoneScale); 
        }
    }

TBool CBTRCCAbsoluteVolumeLevelController::ValidVolumeParams(TUint32 aVolume, TUint32 aMaxVolume)
    {
    return (( aMaxVolume > 0 ) && (aVolume <= aMaxVolume));
    }

//  End of File  
