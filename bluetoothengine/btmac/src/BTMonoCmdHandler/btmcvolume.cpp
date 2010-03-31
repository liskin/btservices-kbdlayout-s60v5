/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  call status handling. 
*
*/


// INCLUDE FILES
#include "btmcphonestatus.h"
#include "btmcvolume.h"
#include <mmkeybearerscancodes.h> // for volume pubsub
#include <mediakeysinternalpskeys.h>
#include <MediaStreamPropertiesPSKeys.h> // for current volume pubsub
#include <mmf/server/sounddevice.h>
#include <e32math.h>

#include "debug.h"


const TInt KNoAudioStreaming = -1;
const TInt KMaxHfVol = 15;
const TInt KSpeakerVolumeSubscribeService = 30;
const TInt KDefaultStep = 1000;

// -----------------------------------------------------------------------------
// CBtmcVolume::NewL
// -----------------------------------------------------------------------------
CBtmcVolume* CBtmcVolume::NewL(CBtmcPhoneStatus& aParent)
    {
    CBtmcVolume* self = new(ELeave) CBtmcVolume(aParent);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcVolume::~CBtmcVolume
// -----------------------------------------------------------------------------
CBtmcVolume::~CBtmcVolume()
    {
    TRACE_FUNC_ENTRY
    delete iActive;
    iVolLevelProperty.Close();
    iVolKeyEventProperty.Close();
    TRACE_FUNC_EXIT
    }

void CBtmcVolume::SetSpeakerVolumeL(TInt aHfVol)
    {
    TRACE_FUNC
    TRACE_INFO((_L("current vols phone %d, acc %d"), iPhnSpkrVol, iAccSpkrVol))
    // Store the new volume setting of headset regardless of 
    // whether volume control is active or not currently
    iAccSpkrVol = HfToPhoneVolScale(aHfVol);
    TRACE_INFO((_L("new acc vol %d"), iAccSpkrVol))
    if ( IsActiveRemoteVolumeControl() )
        {
        TInt vol = GetNewPhoneVol();
        if (vol != KNoAudioStreaming)
            {
            TInt prevPhVol = iPhnSpkrVol;
            iPhnSpkrVol = vol;
            DoSetSpeakerVolL( prevPhVol );
            }
        }
    }

void CBtmcVolume::SetMicrophoneVolumeL(TInt /*aHfVol*/)
    {
    return;
    }

void CBtmcVolume::ActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    if (!IsActiveRemoteVolumeControl() )
        {
		iVolLevelProperty.Subscribe(iActive->iStatus);     
        iActive->GoActive();
        TInt vol = GetNewPhoneVol();
        TRACE_INFO((_L("current phone vol %d, acc vol %d"), vol, iAccSpkrVol))
        if (vol != KNoAudioStreaming)
            {
            iPhnSpkrVol = vol;
            if (iAccSpkrVol != vol)
                {
        		TRAP_IGNORE(iParent.HandleSpeakerVolumeEventL(PhoneToHfVolScale(iPhnSpkrVol)));
        		iAccSpkrVol = iPhnSpkrVol;
                }
            }
        }
    }

void CBtmcVolume::DeActivateRemoteVolumeControl()
    {
    iActive->Cancel();
    TRACE_FUNC
    }

void CBtmcVolume::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("aErr %d, INTERNAL ACTION STATE %d"), aErr, iAction))
    switch (aActive.ServiceId())
        {
        case KSpeakerVolumeSubscribeService:
            {
            if(!aErr)
                {
				iVolLevelProperty.Subscribe(aActive.iStatus);
				aActive.GoActive();

                TInt vol = GetNewPhoneVol();
                TRACE_INFO((_L("new phone vol %d, acc vol %d"), vol, iAccSpkrVol))
                if (vol != KNoAudioStreaming)
                    {
                    TInt prevPhVol = iPhnSpkrVol;
                    iPhnSpkrVol = vol;
                    if (iAction == ESpeakerVolSubscribe && iPhnSpkrVol != iAccSpkrVol)
                        {
                        iParent.HandleSpeakerVolumeEventL(PhoneToHfVolScale(iPhnSpkrVol));
                        iAccSpkrVol = iPhnSpkrVol;
                        }
                    else if (iAction == ESpeakerVolSet)
                        {
                        DoSetSpeakerVolL( prevPhVol );
                        }
                    }
                }
            break;
            }
        default:
            break;
        }
    TRACE_FUNC_EXIT
    }

void CBtmcVolume::CancelRequest(TInt aServiceId)
    {
    switch (aServiceId)
        {
        case KSpeakerVolumeSubscribeService:
            {
            iVolLevelProperty.Cancel();
            break;
            }
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBtmcVolume::CBtmcVolume
// -----------------------------------------------------------------------------
CBtmcVolume::CBtmcVolume(CBtmcPhoneStatus& aParent)
    : iParent(aParent), iAccSpkrVol(KNoAudioStreaming), iStep(KDefaultStep)
    {
    }

// -----------------------------------------------------------------------------
// CBtmcVolume::ConstructL
// -----------------------------------------------------------------------------
void CBtmcVolume::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iVolLevelProperty.Attach(KPSUidMediaStreamProperties, KMediaStreamCurrentVolume))
    LEAVE_IF_ERROR(iVolKeyEventProperty.Attach(KPSUidMediaKeysEventNotifier, KMediaKeysAccessoryVolumeEvent))
    CMMFDevSound* devSound  = CMMFDevSound::NewL();
    iMaxSpkrVol = devSound->MaxVolume();
    TRACE_INFO((_L("Maximum volume is %d"), iMaxSpkrVol))
    delete devSound;
    iPhnSpkrVol = GetNewPhoneVol();
    iActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, KSpeakerVolumeSubscribeService);
    TRACE_FUNC_EXIT
    }

// -----------------------------------------------------------------------------
// CBtmcVolume::DoSetSpeakerVolL
// -----------------------------------------------------------------------------
//
void CBtmcVolume::DoSetSpeakerVolL( TInt aPrevPhVol )
    {
    TRACE_FUNC_ENTRY     
    TInt volClick( 0 );
    TInt diff1;
    TInt diff2;
    
    if( ( aPrevPhVol >= iAccSpkrVol && iPhnSpkrVol <= iAccSpkrVol ) ||
        ( aPrevPhVol <= iAccSpkrVol && iPhnSpkrVol >= iAccSpkrVol ) )
        {
        diff1 = Abs( aPrevPhVol - iAccSpkrVol );
        diff2 = Abs( iPhnSpkrVol - iAccSpkrVol );
        
        if (diff2 > diff1)
            {
            // the previous phone volume is closer to current headset volume setting
            volClick = ( aPrevPhVol < iPhnSpkrVol ) ? KPSVolumeDownClicked : KPSVolumeUpClicked;
            }
        else if ( (diff2 == diff1) && (diff1 !=0) )
            {
            volClick = ( aPrevPhVol < iPhnSpkrVol ) ? KPSVolumeDownClicked : KPSVolumeUpClicked;
            }
        // otherwise we have roughly synchronized vol in both ends.
        }
    else if( iAccSpkrVol != iPhnSpkrVol )
        {
        TInt step = Abs( iPhnSpkrVol - aPrevPhVol );
        if( step )
            {
            // Only update step value if it is not equal to zero
            iStep = step;
            }
        diff1 = Abs( iPhnSpkrVol - iAccSpkrVol );
        if( iAccSpkrVol > iPhnSpkrVol )
            {
            // We are going upward
            diff2 = Abs( (iPhnSpkrVol + iStep) - iAccSpkrVol );
            }
        else
            {
            // We are going downward
            diff2 = Abs( (iPhnSpkrVol - iStep) - iAccSpkrVol );
            }
        if( diff2 <= diff1 )
            {
            volClick = (iAccSpkrVol > iPhnSpkrVol) ? KPSVolumeUpClicked : KPSVolumeDownClicked;
            }        
        }
    TInt err( KErrNotFound );
    if( volClick )
        {
        err = iVolKeyEventProperty.Set( volClick );
        TRACE_INFO((_L("Set KMediaKeysVolumeKeyEvent click %d err %d"), volClick, err));
        }
    
    iAction = err ? ESpeakerVolSubscribe : ESpeakerVolSet;

    TRACE_FUNC_EXIT 
    }

// -------------------------------------------------------------------------------
// CBtmcVolume::HfToPhoneVolScale
// Converts volume level scale used in AT command VGS (0-15) into one used in
// DevSound Audio (0-MaxVol).
// -------------------------------------------------------------------------------
TInt CBtmcVolume::HfToPhoneVolScale(TInt aHfVol)
    {
 	TReal tgt;
 	TReal src(aHfVol);
 	TReal maxAG(iMaxSpkrVol);
 	TReal maxHF(KMaxHfVol);
 	TReal scale = maxAG/maxHF;
	Math::Round( tgt, src*scale, 0);
	TRACE_INFO((_L("HfToPhoneVolScale: HF %d is AG %d"), aHfVol, TInt(tgt)))
	return TInt(tgt);

    }

// -------------------------------------------------------------------------------
// CBtmcVolume::PhoneToHfVolScale
//
// Converts volume level scale used in DevSound Audio (0-10) into one used in AT command
// VGS (0-15).
// -------------------------------------------------------------------------------
TInt CBtmcVolume::PhoneToHfVolScale(TInt aPhoneVol)
    {
 	TReal tgt;
 	TReal src(aPhoneVol);
 	TReal maxAG(iMaxSpkrVol);
 	TReal maxHF(KMaxHfVol);
 	TReal scale = maxHF/maxAG;
	Math::Round( tgt, src*scale, 0);
	TRACE_INFO((_L("PhoneToHfVolScale: HF %d is AG %d"), TInt(tgt), aPhoneVol))
	return TInt(tgt);

    }

TInt CBtmcVolume::GetNewPhoneVol()
    {
    TInt vol = KNoAudioStreaming;
    iVolLevelProperty.Get(vol);
    TRACE_INFO((_L("phone vol P&S %d"), vol))
    return vol;
    }

TInt CBtmcVolume::GetVolume()
	{
    iPhnSpkrVol = GetNewPhoneVol();
	if (iPhnSpkrVol != KNoAudioStreaming)
	    return PhoneToHfVolScale(iPhnSpkrVol);
	return iPhnSpkrVol;
	}

TBool CBtmcVolume::IsActiveRemoteVolumeControl()
    {
    return iActive->IsActive();
    }

// End of file
