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
* Description:   Base class implements volume control of AVRCP Controller.	
*
*
*/

// INCLUDE FILES
#include <remconbeareravrcp.h> //   KRemConBearerAvrcpImplementationUid = 0x1020685f
#include <MediaStreamPropertiesPSKeys.h>
#include <mmkeybearerscancodes.h> // for volume pubsub
#include <mediakeysinternalpskeys.h>
#include <remconinterfaceselector.h> 
#include <remconcoreapicontroller.h> 
#include <remconaddress.h> 
#include <sounddevice.h>

#include "btrccVolumeLevelControllerBase.h"
#include "debug.h"


// MODULE DATA STRUCTURES
const TInt KVolumeChangeListenerServiceId = 0x01; 

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::CBTRCCVolumeLevelController
// C++ constructor.
// -----------------------------------------------------------------------------
//
CBTRCCVolumeLevelControllerBase::CBTRCCVolumeLevelControllerBase(MBTRCCVolumeControllerObserver &aObserver): 
    iObserver(aObserver), iLocalMaxVolume(0), iState(ESubscribePhoneVolume) 
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::ConstructL()
    {
    TRACE_FUNC
    LEAVE_IF_ERROR(iVolLevelProperty.Attach(KPSUidMediaStreamProperties, KMediaStreamCurrentVolume))
    LEAVE_IF_ERROR(iVolKeyEventProperty.Attach(KPSUidMediaKeysEventNotifier, KMediaKeysAccessoryVolumeEvent))
    iBtrccActive = CBTRCCActive::NewL(*this, KVolumeChangeListenerServiceId); 
    }

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTRCCVolumeLevelControllerBase::~CBTRCCVolumeLevelControllerBase()
    {
    TRACE_FUNC
    delete iBtrccActive; 
    iVolLevelProperty.Close();
    iVolKeyEventProperty.Close();
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::Start
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::Start() 
    {
    TRACE_FUNC

	iVolLevelProperty.Subscribe(iBtrccActive->iStatus);
    iBtrccActive->GoActive(); 

    (void) GetPhoneVolume(iPhoneVolume);
    ScalePhoneVolume(iPhoneVolume);
    if( iPhoneVolume > -1)
        {
        DoStart( iPhoneVolume );
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::Stop
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::Stop() 
    {
    TRACE_FUNC
    iBtrccActive->Cancel();
    DoStop(); 
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::Reset
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::Reset()
    {
    TRACE_FUNC 
    iBtrccActive->Cancel();
    DoReset(); 
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::RegisterVolumeChangeNotification
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::RegisterVolumeChangeNotification() 
    {
    TRACE_FUNC
    return;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::GetLocalMaxVolume
// -----------------------------------------------------------------------------
//
TInt CBTRCCVolumeLevelControllerBase::GetLocalMaxVolume() 
    {
    TRACE_FUNC
    if(iLocalMaxVolume == 0)
        {
        // fetch max volume from adaptation and cache it.
        CMMFDevSound* devSound = NULL;
        TRAPD( err, devSound  = CMMFDevSound::NewL() );
        if(!err)
            {
            iLocalMaxVolume = devSound->MaxVolume();
            TRACE_INFO((_L("Maximum volume is %d"), iLocalMaxVolume))
            delete devSound; // This is not needed for anything else.
            }
        }
    return iLocalMaxVolume; 
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::AccessoryChangedVolume
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::AccessoryChangedVolume(TInt aVolumeInPhoneScale) 
    {
    TRACE_FUNC
    iRemoteVolume = aVolumeInPhoneScale;
    TInt vol;
    TInt err = GetPhoneVolume(vol);
    ScalePhoneVolume(iPhoneVolume);
    if(!err)
        {
        TInt prevPhVol = iPhoneVolume;
        iPhoneVolume = vol;
        TInt remoteVol = RoundRemoteVolume(prevPhVol);
        SetPhoneVolume(remoteVol);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::SetPhoneVolume
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::SetPhoneVolume(TInt aRemoteVol)
    {
    TRACE_FUNC
    TRACE_INFO((_L("Remote Volume = %d, iPhoneVolume = %d"), aRemoteVol, iPhoneVolume))
    TInt err( KErrNotFound );
    if (aRemoteVol != iPhoneVolume)
        {
        TInt event = (aRemoteVol > iPhoneVolume) ? KPSVolumeUpClicked : KPSVolumeDownClicked;
        err = iVolKeyEventProperty.Set(event);
        TRACE_INFO((_L("Set KMediaKeysVolumeKeyEvent click %d err %d"), event, err));
        }    
    iState = err ? ESubscribePhoneVolume : ESetPhoneVolume;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::GetPhoneVolume
// -----------------------------------------------------------------------------
//
TInt CBTRCCVolumeLevelControllerBase::GetPhoneVolume(TInt &aVol)
    {
    aVol = -1;
    TInt err = iVolLevelProperty.Get(aVol);
    TRACE_INFO((_L("Get volume P&S ret %d vol %d"), err, aVol))
    return err;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::GetCurrentLocalVolume
// -----------------------------------------------------------------------------
//
TInt CBTRCCVolumeLevelControllerBase::GetCurrentLocalVolume()
    {
    return iPhoneVolume;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::GetCurrentRemoteVolume
// -----------------------------------------------------------------------------
//
TInt CBTRCCVolumeLevelControllerBase::GetCurrentRemoteVolume()
    {
    return iRemoteVolume;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::RoundRemoteVolume
// -----------------------------------------------------------------------------
//
TInt CBTRCCVolumeLevelControllerBase::RoundRemoteVolume(TInt /*aPrevPhVol*/)
    {
    TRACE_FUNC
    // default implementation
    return iRemoteVolume;
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::ScalePhoneVolume
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::ScalePhoneVolume(TInt& /*aVolume*/)
    {
    TRACE_FUNC
    // default implementation
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::RequestCompletedL(CBTRCCActive& aActive, TInt aErr)
    {
    TRACE_FUNC
    if(aActive.ServiceId() == KVolumeChangeListenerServiceId)
        {
        TInt prevPhVol = iPhoneVolume;
        TInt err = GetPhoneVolume(iPhoneVolume);
        ScalePhoneVolume(iPhoneVolume);
                
        if(!err && !aErr && iPhoneVolume > -1)
            {
            switch(iState)
                {
                case ESubscribePhoneVolume:
                    // Tell the specialization of the new volume level. 
                    AdjustRemoteVolume(iPhoneVolume); 
                    break;
                case ESetPhoneVolume:
                    TInt remoteVol = RoundRemoteVolume(prevPhVol);
                    SetPhoneVolume(remoteVol);
                    break;
                }
            }
        
        iVolLevelProperty.Subscribe(iBtrccActive->iStatus);
        iBtrccActive->GoActive();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::CancelRequest
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::CancelRequest(TInt aServiceId) 
    {
    TRACE_FUNC
    switch (aServiceId)
        {
        case KVolumeChangeListenerServiceId:
            {
            iVolLevelProperty.Cancel();
            break;
            }
        default:
            break;
        } 
    }

void CBTRCCVolumeLevelControllerBase::VolumeControlError(TInt aError) 
    {
    iObserver.VolumeControlError(aError); 
    }


//  End of File  
