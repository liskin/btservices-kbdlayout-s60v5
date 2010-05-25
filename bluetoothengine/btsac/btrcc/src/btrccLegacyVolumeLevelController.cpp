/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   This class implements legacy volume control of AVRCP Controller.	
*
*
*/

// INCLUDE FILES
#include <remconbeareravrcp.h> //	KRemConBearerAvrcpImplementationUid = 0x1020685f
#include <MediaStreamPropertiesPSKeys.h>
#include <remconinterfaceselector.h> 
#include <remconcoreapicontroller.h> 
#include <remconaddress.h>
#include <e32math.h>

#include "btrccLegacyVolumeLevelController.h"
#include "debug.h"

const TInt KVolumeNotSynchronized = -1;
const TInt KLegacyVolumeLevelSetServiceId = 0x03;

const TInt KVolumeSyncRecheckTimer = 500000; // 0.5 second
const TInt KVolumeCommandInterval = 50000; // 0.05 second

const TInt KVolumeScaleMax = 15;

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTRCCLegacyVolumeLevelController* CBTRCCLegacyVolumeLevelController::NewL(CRemConInterfaceSelector& aInterfaceSelector, MBTRCCVolumeControllerObserver& aObserver)
	{
	CBTRCCLegacyVolumeLevelController* self = new (ELeave) CBTRCCLegacyVolumeLevelController(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL(aInterfaceSelector);
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::CBTRCCVolumeLevelController
// C++ constructor.
// -----------------------------------------------------------------------------
//
CBTRCCLegacyVolumeLevelController::CBTRCCLegacyVolumeLevelController(MBTRCCVolumeControllerObserver& aObserver)
  : CBTRCCVolumeLevelControllerBase(aObserver), iState(EStateWaitingForVolumeLevelChanges), iPhoneVolume(KVolumeNotSynchronized),
    iCurrentPhoneVolume(KVolumeNotSynchronized), iCurrentRemoteVolume(KVolumeNotSynchronized)
	{
	}

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::ConstructL(CRemConInterfaceSelector& aInterfaceSelector)
	{
   	TRACE_FUNC
   	CBTRCCVolumeLevelControllerBase::ConstructL();
    iInterfaceSelector = &aInterfaceSelector;
   	iRemConVolController = CRemConCoreApiController::NewL(aInterfaceSelector, *this); 
    TRACE_INFO((_L("CRemConCoreApiController created")))
    iBtrccLegacyVolumeActive = CBTRCCActive::NewL(*this, KLegacyVolumeLevelSetServiceId);
    TRACE_INFO((_L("CBTRCCActive created")))
    iCtrlRTimer.CreateLocal();    
    TRACE_INFO((_L("RTimer created")))
    iLocalMaxVolume = GetLocalMaxVolume();
	}

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTRCCLegacyVolumeLevelController::~CBTRCCLegacyVolumeLevelController()
	{
	TRACE_FUNC
    delete iBtrccLegacyVolumeActive;
	iCtrlRTimer.Close();
	}

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::DoStart
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::DoStart(TInt aInitialVolume) 
    {
    TRACE_FUNC   	
    AdjustRemoteVolume(aInitialVolume);
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::DoStop
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::DoStop() 
    {
    TRACE_FUNC
    iBtrccLegacyVolumeActive->Cancel();
    iState = EStateWaitingForVolumeLevelChanges;
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::DoReset
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::DoReset()
    {
    TRACE_FUNC
    iBtrccLegacyVolumeActive->Cancel();
   	iCurrentRemoteVolume = KVolumeNotSynchronized;
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::AdjustRemoteVolume
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::AdjustRemoteVolume(TInt aVolume)
    {
    // A call to this is made when PS Key changes. 
    TRACE_INFO((_L("CBTRCCLegacyVolumeLevelController::AdjustRemoteVolume, volume %d, state %d"), aVolume, iState))    
    iPhoneVolume = aVolume;
    
    if(iCurrentRemoteVolume == KVolumeNotSynchronized)
        {
        // We set current phone volume level as the reference synchronization point
        // if we haven't done it yet. Otherwise we synchronize remote to the current level.
        // Just synchronize volumes and return, there is nothing else to do at this point.
        iCurrentRemoteVolume = iPhoneVolume;
        return;
        }
    
    switch(iState)
        {
        case EStateWaitingForVolumeLevelChanges:
            // Just break here.
            break;
        case EStateDelay:
            // Sharp volume jump handling is ongoing. In this case we want to update
            // new volume level immediately.
            if( iBtrccLegacyVolumeActive->IsActive() )
                {
                iBtrccLegacyVolumeActive->Cancel();
                }
            break;
        default:
            // We are middle of timing or volume up/down processing. New value
            // is stored, just return here. New value is updated after previous
            // job is done.
            return;
        }
    
    DoRemoteControl();
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::DoRemoteControl
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::DoRemoteControl()
    {
    TRACE_INFO((_L("target volume level %d"), iPhoneVolume))
    TRACE_INFO((_L("phone current volume level %d"), iCurrentPhoneVolume))
	TRACE_INFO((_L("remote current volume level %d"), iCurrentRemoteVolume))
	
	TState preState = iState;
	iState = EStateWaitingForVolumeLevelChanges;
	
	if ( iPhoneVolume != iCurrentRemoteVolume )
	    {
	    TInt prevPhoneVolume = iCurrentPhoneVolume;
	    iCurrentPhoneVolume = iPhoneVolume;

        if ( iCurrentPhoneVolume == 0 && prevPhoneVolume > 1 )
            {
            // A sharp volume jump, most probably caused by audio stream switching.
            // Delay the starting of audio adjustment for 0.5 seconds.
            iState = EStateDelay;
            }
	    else if (iCurrentPhoneVolume > iCurrentRemoteVolume)
	        {
	        iState = preState == EStateVolumeUpTiming ? EStateVolumeUp : EStateVolumeUpTiming;  
	        }
    	else if (iCurrentPhoneVolume < iCurrentRemoteVolume)
    	    {
	        iState = preState == EStateVolumeDownTiming ? EStateVolumeDown : EStateVolumeDownTiming; 
    	    }
	    }
	
    switch (iState)
        {
        case EStateVolumeUpTiming:
        	{
        	TRACE_INFO(_L("Volume up timing"))
        	StartTimer(KVolumeCommandInterval);
        	break;	
        	}        
        case EStateVolumeUp:
            {
			TRACE_INFO(_L("Volume up"))
			SendVolumeCommand(EStateVolumeUp); 
            break;
            }
        case EStateVolumeDownTiming:
        	{
        	TRACE_INFO(_L("Volume down timing"))
        	StartTimer(KVolumeCommandInterval);
        	break;	
        	}            
        case EStateVolumeDown:
            {
			TRACE_INFO(_L("Volume down"))
			SendVolumeCommand(EStateVolumeDown); 
            break;
            }
        case EStateDelay:
            {
            TRACE_INFO(_L("Delay before updating volume"))
            StartTimer( KVolumeSyncRecheckTimer );
            break;
            }
        case EStateWaitingForVolumeLevelChanges:
            {
			TRACE_INFO(_L("Waiting volume change"))
            break;
            }
        default:
            {
            // Not possible
            }
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::ScalePhoneVolume
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::ScalePhoneVolume(TInt &aVolume)
    {
    TRACE_FUNC        
    // Converts volume level scale used in DevSound Audio (0 - 10000) into 0 - KVolumeScaleMax.
    if( aVolume <= 0 )
        {
        // 0 level and -1 (no stream) don't need adjustment. 
        return;
        }
    
    TReal tgt;
    TReal src(aVolume);    
    TReal maxScale(KVolumeScaleMax);    
    TReal maxDevSound(iLocalMaxVolume);
    TReal scale = maxScale/maxDevSound;
    Math::Round( tgt, src*scale, 0);
    
    if( TInt(tgt) == 0 )
        {
        // Only aVol value 0 is real zero (muted). All other scaled zero
        // values have to be upgraded to level 1.
        tgt = 1;
        }
    TRACE_INFO((_L("Volume scaled: original %d, scaled %d"), aVolume, TInt(tgt)))
    aVolume = TInt(tgt);
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::RequestCompletedL(CBTRCCActive& aActive, TInt aErr)
    {
    TRACE_FUNC 
    if( aActive.ServiceId() == KLegacyVolumeLevelSetServiceId )
        {
        TRACE_INFO( (_L("CBTRCCLegacyVolumeLevelController iState: %d"), iState ) )
        if ( aErr != KErrNone )
            {
            // Something wrong happened, could be e.g. the connection has
            // been released while we are changing volume on remote device.
            iState = EStateWaitingForVolumeLevelChanges;
            return;
            }
        switch(iState)
            {
            case EStateVolumeUp:
                {
                ++iCurrentRemoteVolume;
                break;
                }
            case EStateVolumeDown:
                {
                --iCurrentRemoteVolume;
                break;
                }
            case EStateDelay:
            case EStateWaitingForVolumeLevelChanges:
            case EStateVolumeUpTiming:
            case EStateVolumeDownTiming:
                break;
            default:
                // Impossible to happen.
                break;
            }
        DoRemoteControl();
        }
    else 
        {
        // This is something the base class can handle. 
        CBTRCCVolumeLevelControllerBase::RequestCompletedL(aActive, aErr);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::CancelRequest
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::CancelRequest(TInt aServiceId) 
    {
    TRACE_FUNC 
    if( aServiceId == KLegacyVolumeLevelSetServiceId )
        {
        // Cancel possible active timers
        iCtrlRTimer.Cancel();

        if (iState == EStateVolumeUp)
            {
            ++iCurrentRemoteVolume;
            }
        else if (iState == EStateVolumeDown)
            {
            --iCurrentRemoteVolume;
            }
        }
    else
        {
        // This is something the base class can handle.
        CBTRCCVolumeLevelControllerBase::CancelRequest(aServiceId);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::StartTimer
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::StartTimer(TInt aInterval) 
    {
    TRACE_FUNC 
    if( !iBtrccLegacyVolumeActive->IsActive() )
        {
        iCtrlRTimer.After(iBtrccLegacyVolumeActive->iStatus, aInterval);
        iBtrccLegacyVolumeActive->GoActive();
        }
    else
        {
        TRACE_INFO(_L("CBTRCCLegacyVolumeLevelController::StartTimer() ERR, timer already active!"))
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::SendVolumeCommand
// -----------------------------------------------------------------------------
//
void CBTRCCLegacyVolumeLevelController::SendVolumeCommand(TInt aDirection) 
    {
    TRACE_FUNC 
    if( !iBtrccLegacyVolumeActive->IsActive() )
        {
        if( aDirection == EStateVolumeUp )
            {
            iRemConVolController->VolumeUp(iBtrccLegacyVolumeActive->iStatus, iNumRemotes, ERemConCoreApiButtonClick);
            }
        else
            {
            iRemConVolController->VolumeDown(iBtrccLegacyVolumeActive->iStatus, iNumRemotes, ERemConCoreApiButtonClick);
            }
        iBtrccLegacyVolumeActive->GoActive();
        }
    else
        {
        TRACE_INFO(_L("CBTRCCLegacyVolumeLevelController::DoRemConControl() ERR, previous action ongoing!"))
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLegacyVolumeLevelController::MrccacoResponse()
// -----------------------------------------------------------------------------
//
TInt CBTRCCLegacyVolumeLevelController::MrccacoResponse()
	{
	TRACE_FUNC
	return 0; 
    }

//  End of File  
