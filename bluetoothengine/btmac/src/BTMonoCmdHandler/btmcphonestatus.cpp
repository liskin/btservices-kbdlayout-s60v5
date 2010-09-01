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
#include <nssvascoreconstant.h>
#include <etelmm.h>
#include "atcodec.h"
#include "btmcprotocol.h"
#include "btmcprotocolstatus.h"
#include "btmc_defs.h"
#include "btmcvolume.h"
#include "btmcsignal.h"
#include "btmcbattery.h"

#include "debug.h"

const TInt KNetworkRegistrationSubscribe = 20;
const TInt KVoiceDialSubscribe = 21;


// -----------------------------------------------------------------------------
// CBtmcPhoneStatus::NewL
// -----------------------------------------------------------------------------
CBtmcPhoneStatus* CBtmcPhoneStatus::NewL(
    CBtmcProtocol& aProtocol, RMobilePhone& aPhone, TBtmcProfileId aProfile) 
    {
    CBtmcPhoneStatus* self = CBtmcPhoneStatus::NewLC(aProtocol, aPhone, aProfile);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcPhoneStatus::NewLC
// -----------------------------------------------------------------------------
CBtmcPhoneStatus* CBtmcPhoneStatus::NewLC(
    CBtmcProtocol& aProtocol, RMobilePhone& aPhone, TBtmcProfileId aProfile) 
    {
    CBtmcPhoneStatus* self = new(ELeave) CBtmcPhoneStatus(aProtocol, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL(aProfile);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcPhoneStatus::~CBtmcPhoneStatus
// -----------------------------------------------------------------------------
CBtmcPhoneStatus::~CBtmcPhoneStatus()
    {
    TRACE_FUNC_ENTRY
    iActives.ResetAndDestroy();
    iActives.Close();
    iVoiceDialProperty.Close();
    delete iVol;
    delete iSignal;
    delete iBattery;
    
    TRACE_FUNC_EXIT
    }

RMobilePhone::TMobilePhoneRegistrationStatus CBtmcPhoneStatus::NetworkStatus() const
    {
    return iNetworkStatus;
    }

void CBtmcPhoneStatus::SetVolumeControlFeatureL(TBool aSupported)
    {
    TRACE_FUNC_ENTRY
    if (aSupported && !iVol)
        {
        iVol = CBtmcVolume::NewL(*this);
        }
    else if (!aSupported)
        {
        delete iVol;
        iVol = NULL;
        }
    TRACE_FUNC_EXIT
    }

void CBtmcPhoneStatus::SetVoiceRecognitionControlL(TBool aEnabled)
    {
    TRACE_FUNC_ENTRY
    if (aEnabled && !iVoiceRecognitionEnabled)
        {
        // AO for Voice Dial
        LEAVE_IF_ERROR(iVoiceDialProperty.Attach(KSINDUID, ERecognitionState))
        CBtmcActive* ao = CBtmcActive::NewLC(*this, CActive::EPriorityStandard, KVoiceDialSubscribe);
        iActives.AppendL(ao);
        iVoiceDialProperty.Subscribe(ao->iStatus);
        ao->GoActive();
        CleanupStack::Pop(ao);
        iVoiceRecognitionEnabled = ETrue;      
        }

    TRACE_FUNC_EXIT
    }

void CBtmcPhoneStatus::SetSpeakerVolumeL(TInt aHfVol)
    {
    if (this && iVol)
        {
        TRACE_FUNC_ENTRY
        iVol->SetSpeakerVolumeL(aHfVol);
        TRACE_FUNC_EXIT
        }
    }

void CBtmcPhoneStatus::ActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    if (iVol)
        iVol->ActivateRemoteVolumeControl();
    }

void CBtmcPhoneStatus::DeActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    if (iVol)
        iVol->DeActivateRemoteVolumeControl();
    }

TInt CBtmcPhoneStatus::GetSignalStrength()
    {
    return iSignal->GetSignalStrength();
    }

TInt CBtmcPhoneStatus::GetBatteryCharge()
    {
    return iBattery->GetBatteryCharge();
    }

void CBtmcPhoneStatus::SetRecognitionInitiator(TBTMonoVoiceRecognitionInitiator aInitiator)
    {
    iVoiceRecogInitiator = aInitiator;
    }

// Send unsolicited network status responses:
// +CIEV: 1,0 is generated when Home/Roam network becomes unavailable
// +CIEV: 1,1 generated when Home/Roam network becomes available 
// For HFP1.5 +CIEV: 6, 1 when roaming is active, +CIEV: 6,0 when roaming is not active

void CBtmcPhoneStatus::HandleNetworkRegistrationEventL(
    RMobilePhone::TMobilePhoneRegistrationStatus aOldStatus, 
    RMobilePhone::TMobilePhoneRegistrationStatus aNewStatus)
    {
    if ( iProtocol.ProtocolStatus().iIndicatorNotif & KIndServiceBit )
        {
        if ((aOldStatus == RMobilePhone::ERegisteredOnHomeNetwork || 
             aOldStatus == RMobilePhone::ERegisteredRoaming) &&
            (aNewStatus != RMobilePhone::ERegisteredOnHomeNetwork && 
             aNewStatus != RMobilePhone::ERegisteredRoaming))
            {
            ReportIndicatorL(EBTMonoATNetworkIndicator, EBTMonoATNetworkUnavailable);
            }
        else if ((aOldStatus != RMobilePhone::ERegisteredOnHomeNetwork && 
                  aOldStatus != RMobilePhone::ERegisteredRoaming) &&
                 (aNewStatus == RMobilePhone::ERegisteredOnHomeNetwork || 
                  aNewStatus == RMobilePhone::ERegisteredRoaming))
            {
            ReportIndicatorL(EBTMonoATNetworkIndicator, EBTMonoATNetworkAvailable);
            }
        }
        
    // Roaming status
    if( iProtocol.ProtocolStatus().iIndicatorNotif & KIndRoamBit )
        {
        if(aOldStatus != RMobilePhone::ERegisteredRoaming && aNewStatus == RMobilePhone::ERegisteredRoaming)
            {
            ReportIndicatorL(EBTMonoATRoamingIndicator, EBTMonoATRoamingActive);
            }
        else if(aOldStatus == RMobilePhone::ERegisteredRoaming && aNewStatus != RMobilePhone::ERegisteredRoaming)
            {
            ReportIndicatorL(EBTMonoATRoamingIndicator, EBTMonoATRoamingInactive);
            }
        }
    }
    
void CBtmcPhoneStatus::HandleSpeakerVolumeEventL(TInt aVol)
    {
    TRACE_FUNC_ENTRY
    CATResult* event = CATResult::NewLC(EATVGS, EATUnsolicitedResult, TATParam(aVol));
    iProtocol.SendUnsoltResultL(*event);
    CleanupStack::PopAndDestroy(event);
    TRACE_FUNC_EXIT
    }

void CBtmcPhoneStatus::HandleVoiceDialEventL(TInt aEnabled)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Voice dial Initiator %d, Voice dial event %d"), iVoiceRecogInitiator, aEnabled))
    TBTMonoATVoiceRecognition value = EBTMonoATVoiceRecognitionOff;    
    
    if (iVoiceRecogInitiator != EBTMonoVoiceRecognitionActivatedByRemote && aEnabled == ERecognitionStarted)
        {// phone activated or headset deactivated voice recognition, informs headset status change
        value = EBTMonoATVoiceRecognitionOn;
        }
    else if(aEnabled == KErrNotFound)
    	{// headset or phone ui deactivated voice recognition, inform headset status change
    	value = EBTMonoATVoiceRecognitionOff;
    	}
    else
	    {
    	return;
    	}
    iVoiceRecogInitiator = EBTMonoVoiceRecognitionDefaultInitiator;
    CATResult* event = CATResult::NewLC(EATBVRA, EATUnsolicitedResult, TATParam(value));
    iProtocol.SendUnsoltResultL(*event);
    CleanupStack::PopAndDestroy(event);
    TRACE_FUNC_EXIT
    }

void CBtmcPhoneStatus::HandleNetworkStrengthChangeL(TInt8 aStrength)
    {
    if(iProtocol.ProtocolStatus().iIndicatorNotif && 
    	(iProtocol.ProtocolStatus().iIndicatorNotif & KIndSignalBit))
        ReportIndicatorL(EBTMonoATSignalStrengthIndicator, TInt(aStrength));
    }

void CBtmcPhoneStatus::HandleBatteryChangeL(TInt aLevel)
    {
    TRACE_FUNC_ENTRY
    if(iProtocol.ProtocolStatus().iIndicatorNotif && 
    	(iProtocol.ProtocolStatus().iIndicatorNotif & KIndChargeBit))
        ReportIndicatorL(EBTMonoATBatteryChargeIndicator, aLevel);
    TRACE_FUNC_EXIT
    }

    
void CBtmcPhoneStatus::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    switch (aActive.ServiceId())
        {
        case KNetworkRegistrationSubscribe:
            {
            if (!aErr)
                {
                HandleNetworkRegistrationEventL(iNetworkStatus, iNewNetworkStatus);
                iNetworkStatus = iNewNetworkStatus;
                iPhone.NotifyNetworkRegistrationStatusChange(aActive.iStatus, iNewNetworkStatus);
                aActive.GoActive();
                }
            break;
            }
        case KVoiceDialSubscribe:
            {
            if (!aErr || aErr == KErrNotFound)
                {
				iVoiceDialProperty.Subscribe(aActive.iStatus);
				aActive.GoActive();

                TInt value = aErr;
				if (!aErr)
					{
					TInt err = iVoiceDialProperty.Get(value);
					if (err != KErrNone)
					   {
					   value = err;
					   }
					}
				HandleVoiceDialEventL(value);    
                }
            break;
            }
        default:
            break;
        }
    }

void CBtmcPhoneStatus::CancelRequest(TInt aServiceId)
    {
    switch (aServiceId)
        {
        case KNetworkRegistrationSubscribe:
            {
            iPhone.CancelAsyncRequest(EMobilePhoneNotifyNetworkRegistrationStatusChange);
            break;
            }
        case KVoiceDialSubscribe:
            {
            iVoiceDialProperty.Cancel();
            break;    
            }
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBtmcPhoneStatus::CBtmcPhoneStatus
// -----------------------------------------------------------------------------
CBtmcPhoneStatus::CBtmcPhoneStatus(
    CBtmcProtocol& aProtocol, RMobilePhone& aPhone)
    : iProtocol(aProtocol), 
      iPhone(aPhone),
      iNetworkStatus(RMobilePhone::ERegistrationUnknown)
    {
    }

// -----------------------------------------------------------------------------
// CBtmcPhoneStatus::ConstructL
// -----------------------------------------------------------------------------
void CBtmcPhoneStatus::ConstructL(TBtmcProfileId aProfile)
    {
    TRACE_FUNC
    // HSP profile doesn't need these functions:
    if ( aProfile == EBtmcHFP0105 )
        {
        iBattery = CBtmcBattery::NewL(*this);
        iBattery->GoActive();
        iSignal = CBtmcSignal::NewL(*this);
        iSignal->GoActive();
        TRequestStatus sta = KRequestPending;
        iPhone.GetNetworkRegistrationStatus(sta, iNetworkStatus);
        User::WaitForRequest(sta);
        TRACE_INFO((_L(" Network status %d"), iNetworkStatus))
        // AO for Network status
        CBtmcActive* ao = 
            CBtmcActive::NewLC(*this, CActive::EPriorityStandard, KNetworkRegistrationSubscribe);
        iActives.AppendL(ao);
        iPhone.NotifyNetworkRegistrationStatusChange(ao->iStatus, iNewNetworkStatus);
        ao->GoActive();
        CleanupStack::Pop(ao);
        }
    TRACE_FUNC_EXIT  
    }

void CBtmcPhoneStatus::ReportIndicatorL(TBTMonoATPhoneIndicatorId aIndicator, TInt aValue)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("indicator %d, value %d"), aIndicator, aValue))
    RArray<TATParam> params;
    CleanupClosePushL(params);
    params.AppendL(TATParam(aIndicator));
    params.AppendL(TATParam(aValue));
    CATResult* ciev = CATResult::NewLC(EATCIEV, EATUnsolicitedResult, &params);
    iProtocol.SendUnsoltResultL(*ciev);
    CleanupStack::PopAndDestroy(ciev);
    CleanupStack::PopAndDestroy(&params);
    TRACE_FUNC_EXIT
    }

TInt CBtmcPhoneStatus::GetRssiStrength()
	{
	return iSignal->GetRssiStrength();
	}
    
// End of file
