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
    if(!err)
        {
        iPhoneVolume = vol;
        SetPhoneVolume();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCVolumeLevelControllerBase::SetPhoneVolume
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::SetPhoneVolume()
    {
    TRACE_FUNC
    TRACE_INFO((_L("iRemoteVolume = %d, iPhoneVolume = %d"), iRemoteVolume, iPhoneVolume))
    if (iRemoteVolume != iPhoneVolume)
        {
        TInt event = (iRemoteVolume > iPhoneVolume) ? KPSVolumeUpClicked : KPSVolumeDownClicked;
        TInt err = iVolKeyEventProperty.Set(event);
        if (err)
            {
            TRACE_ERROR((_L("Set KMediaKeysVolumeKeyEvent err %d"), err));
            }
        iState = ESetPhoneVolume;
        }    
    else
        {
        iState = ESubscribePhoneVolume;
        } 
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
// CBTRCCVolumeLevelControllerBase::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBTRCCVolumeLevelControllerBase::RequestCompletedL(CBTRCCActive& aActive, TInt aErr)
    {
    TRACE_FUNC
    if(aActive.ServiceId() == KVolumeChangeListenerServiceId)
        {
        TInt err = GetPhoneVolume(iPhoneVolume);
                
        if(!err && !aErr && iPhoneVolume > -1)
            {
            switch(iState)
                {
                case ESubscribePhoneVolume:
                    // Tell the specialization of the new volume level. 
                    AdjustRemoteVolume(iPhoneVolume); 
                    break;
                case ESetPhoneVolume:
                    SetPhoneVolume();
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
