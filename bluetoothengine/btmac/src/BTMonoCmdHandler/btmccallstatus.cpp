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
* Description:  
*
*/

#include "atcodec.h"
#include "btmcprotocol.h"
#include "btmcprotocolstatus.h"
#include "btmccallstatus.h"
#include "btmcmobileline.h"
#include "btmcvoipline.h"
#include "debug.h"

const TInt KRingTimerService = 10;

CBtmcCallStatus* CBtmcCallStatus::NewL(
    CBtmcProtocol& aProtocol, 
    RMobilePhone& aPhone,
    TBtmcProfileId aProfile)
    {
    CBtmcCallStatus* self=new (ELeave) CBtmcCallStatus(aProtocol);
    CleanupStack::PushL(self);
    self->ConstructL(aPhone, aProfile);
    CleanupStack::Pop(self);
    return self;
    }

// ==========================================================
// CBtmcCallStatus::~CBtmcCallStatus
// 
// ==========================================================
//
CBtmcCallStatus::~CBtmcCallStatus()
    {
    TRACE_FUNC_ENTRY
    delete iTimerActive;
    iTimer.Close();
    
    iLines.ResetAndDestroy();
    iLines.Close();
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::CallStatus
// 
// ==========================================================
//
TInt CBtmcCallStatus::CallStatusL() const
    {
    TRACE_FUNC
    TInt mask = 0;
    TInt count = iLines.Count();
    for (TInt i = 0; i < count; i++)
        {
        mask |= iLines[i]->CallStatusL();
        }
    TRACE_INFO((_L(" overall CALL STATUS 0x%08x"), mask))    
    return mask;    
    }

// ==========================================================
// CBtmcCallStatus::ReportCallStatusL
// 
// ==========================================================
//
void CBtmcCallStatus::ReportCallStatusL()
    {
    TInt callBits = CallStatusL();
    ReportCallEventL(iProtocol.ProtocolStatus().iCallBits, callBits);
    iProtocol.ProtocolStatus().iCallBits = callBits;    
    }

// ==========================================================
// CBtmcCallStatus::RequestCompleted
// 
// ==========================================================
//
void CBtmcCallStatus::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    switch (aActive.ServiceId())
        {
        case KRingTimerService:
            {
            if (aErr == KErrNone)
                {
                ReportRingAndClipL();
                iTimer.After(aActive.iStatus, KRingInterval);
                aActive.GoActive();
                }
            break;
            }
        default:
            break;
        }
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::CancelRequest
// Constructor
// ==========================================================
//
void CBtmcCallStatus::CancelRequest(TInt aServiceId)
    {
    TRACE_FUNC_ENTRY
    if (aServiceId == KRingTimerService)
        {
        iTimer.Cancel();
        }
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::CBtmcCallStatus
// Constructor
// ==========================================================
//
CBtmcCallStatus::CBtmcCallStatus(CBtmcProtocol& aProtocol) : iProtocol(aProtocol)
    {
    }

// ==========================================================
// CBtmcCallStatus::ConstructL
// 
// ==========================================================
//
void CBtmcCallStatus::ConstructL(
    RMobilePhone& aPhone,
    TBtmcProfileId aProfile)
    {
    TRACE_FUNC_ENTRY
    CBtmcMobileLine* voiceline = CBtmcMobileLine::NewLC(*this, aPhone, KMmTsyVoice1LineName);
    iLines.AppendL(voiceline);
    CleanupStack::Pop(voiceline);
    CBtmcVoIPLine *voipline = NULL;
    TRAPD(err, voipline = CBtmcVoIPLine::NewL(*this, aPhone));
    if(!err)
        {
        CleanupStack::PushL(voipline);
        iLines.AppendL(voipline);
        CleanupStack::Pop(voipline);        
        }
    
    TRAP(err, voiceline = CBtmcMobileLine::NewL(*this, aPhone, KMmTsyVoice2LineName));
    if (!err)
        {
        CleanupStack::PushL(voiceline);
        iLines.AppendL(voiceline);
        CleanupStack::Pop(voiceline);
        }
    
    if (aProfile == EBtmcHFP0105 )
        {
        CBtmcMobileLine* dataline = CBtmcMobileLine::NewLC(*this, aPhone, KMmTsyDataLineName);
        iLines.AppendL(dataline);
        CleanupStack::Pop(dataline);
        }
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::HandleClccL
// 
// ==========================================================
//
void CBtmcCallStatus::HandleClccL()
    {
    TRACE_FUNC_ENTRY
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);
        
    if(CallStatusL() & KCallAllStatusMask)
        {
        TInt count = iLines.Count();        
        for(TInt i=0; i<count; i++)
            {            
            if( !iLines[i]->IsVoip() )
                {                
                const RPointerArray<CBtmcCallActive>& calls = iLines[i]->ActiveCalls();
                TInt count2 = calls.Count();            
                RMobileConferenceCall myConf;
                User::LeaveIfError(myConf.Open(iLines[i]->Phone()));
                CleanupClosePushL(myConf);
                TInt confcount;
                User::LeaveIfError(myConf.EnumerateCalls(confcount));                
                RMobileCall::TMobileCallInfoV1 info;
                RMobileCall::TMobileCallRemotePartyInfoV1 remote;                
                for(TInt j=0; j<count2; j++)
                    {                                        
                    calls[j]->GetCallInfo(info);                                        
                    if (info.iStatus != RMobileCall::EStatusIdle)
                        {
                        remote = info.iRemoteParty;
                        CATResult* cmd = MakeClccL(info, remote, confcount);
                        resarr.AppendL(cmd);
                        }
                    }
                CleanupStack::PopAndDestroy(&myConf);
                }
            else // VoIP case
                {                       
				const MCall& callInfo = iLines[i]->CallInformationL();
				if(&callInfo != NULL)
				    {				    
				    CATResult* cmd = MakeVoIPClccL(callInfo);
				    resarr.AppendL(cmd);
				    }				                    				
                }
            }
        }
    CATResult* ok = CATResult::NewL(EATOK);
    CleanupStack::PushL(ok);
    resarr.AppendL(ok);
    CleanupStack::Pop(ok);
    iProtocol.SendResponseL(resarr);
    CleanupStack::PopAndDestroy(&resarr);
    iProtocol.CmdHandlingCompletedL();
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::GetRingingNumL
// 
// ==========================================================
//
void CBtmcCallStatus::GetRingingNumL(TDes8& aNum)
	{
    TRACE_FUNC_ENTRY
	if(CallStatusL() & KCallRingingBit)
		{
		TInt count = iLines.Count();
		for(TInt i=0; i<count; i++)
			{
			if(!iLines[i]->IsVoip())
			    {
                const RPointerArray<CBtmcCallActive>& calls = iLines[i]->ActiveCalls();
                TInt count2 = calls.Count();
                RMobileConferenceCall myConf;
                User::LeaveIfError(myConf.Open(iLines[i]->Phone()));
                CleanupClosePushL(myConf);
                RMobileCall::TMobileCallInfoV1 info;
                RMobileCall::TMobileCallRemotePartyInfoV1 remote;
			
                for(TInt j=0; j<count2; j++)
                    {
                    calls[j]->GetCallInfo(info);
                    if (info.iStatus == RMobileCall::EStatusRinging
                       && (info.iValid & RMobileCall::KCallRemoteParty)
                       && info.iRemoteParty.iRemoteIdStatus == RMobileCall::ERemoteIdentityAvailable)
                        {
                        aNum.Copy(info.iRemoteParty.iRemoteNumber.iTelNumber);
                        break;
                        }
                    }
                CleanupStack::PopAndDestroy(&myConf);
			    }
	        }
		}
	TRACE_FUNC_EXIT
	}

// ==========================================================
// CBtmcCallStatus::GetOutgoingNumL
// 
// ==========================================================
//
void CBtmcCallStatus::GetOutgoingNumL(TDes8& aNum)
	{
    TRACE_FUNC_ENTRY
	TInt count = iLines.Count();
	for(TInt i=0; i<count; i++)
		{
		const RPointerArray<CBtmcCallActive>& calls = iLines[i]->ActiveCalls();
		TInt count2 = calls.Count();
		RMobileCall::TMobileCallInfoV1 info;
		RMobileCall::TMobileCallRemotePartyInfoV1 remote;
		
		for(TInt j=0; j<count2; j++)
			{
			calls[j]->GetCallInfo(info);
			if (info.iStatus == RMobileCall::EStatusDialling
				&& (info.iValid & RMobileCall::KCallDialledParty))
			    {
				TPtrC tempPtrC(KNullDesC);
				tempPtrC.Set(info.iDialledParty.iTelNumber);
				aNum.Copy(tempPtrC);
				break;
			    }
			}
		}
	TRACE_FUNC_EXIT
	}

TBTMonoATPhoneNumberType CBtmcCallStatus::NumberType( const TDesC8& aNum )
    {
    if (aNum.Length() == 0)
        {
        return EBTMonoATPhoneNumberUnavailable;
        }
    if(aNum.Locate('+') == 0)
        {
        return EBTMonoATPhoneNumberInternational;
        }
    return EBTMonoATPhoneNumberNational;
    }

// ==========================================================
// CBtmcCallStatus::MakeClccL
// 
// ==========================================================
//
CATResult* CBtmcCallStatus::MakeClccL(RMobileCall::TMobileCallInfoV1& info, 
    RMobileCall::TMobileCallRemotePartyInfoV1& remote, TInt aConferenceCallCount)
    {
    TRACE_FUNC_ENTRY
    RArray<TATParam> params;
    CleanupClosePushL(params);
    if(info.iValid & RMobileCall::KCallId)
        {
        LEAVE_IF_ERROR(params.Append(TATParam(info.iCallId))) // Index number
        }
    else
        {
        LEAVE_IF_ERROR(params.Append(TATParam()))
        }
        
    if(remote.iDirection == RMobileCall::EMobileOriginated) // outgoing
        {
        LEAVE_IF_ERROR(params.Append(TATParam(0)))
        }
    else // unknown or incoming
        {
        LEAVE_IF_ERROR(params.Append(TATParam(1)))            
        }
        
    switch(info.iStatus) // status
        {
        case RMobileCall::EStatusDialling:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(2)))
            break;
            }
        case RMobileCall::EStatusRinging:
            {
            TInt status = CallStatusL();
            if(status & KActiveCallMask)
            	{
            	LEAVE_IF_ERROR(params.Append(TATParam(5)))
            	}
            else
	            {
            	LEAVE_IF_ERROR(params.Append(TATParam(4)))
	            }
            break;
            }
        case RMobileCall::EStatusAnswering:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(5)))
            break;
            }
        case RMobileCall::EStatusConnecting:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(3)))
            break;
            }
        case RMobileCall::EStatusConnected:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(0)))
            break;
            }
        case RMobileCall::EStatusHold:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(1)))
            break;
            }
        default:
            LEAVE_IF_ERROR(params.Append(TATParam(0)))
        }
        
    switch(info.iService) // mode
        {
        case RMobilePhone::EVoiceService:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(0)))
            break;
            }
        case RMobilePhone::ECircuitDataService:
        case RMobilePhone::EPacketDataService:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(1)))
            break;
            }
        case RMobilePhone::EFaxService:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(2)))
            break;
            }
        default:
            LEAVE_IF_ERROR(params.Append(TATParam(0)))    
        }
        
    // multiparty
    TATParam param;
    if(aConferenceCallCount)
        LEAVE_IF_ERROR(params.Append(TATParam(1)))
    else
        LEAVE_IF_ERROR(params.Append(TATParam(0)))

    // number (optional)
	TPtrC tempPtrC(KNullDesC);
    if ((info.iValid & RMobileCall::KCallRemoteParty) && 
         info.iRemoteParty.iRemoteIdStatus == RMobileCall::ERemoteIdentityAvailable)
        {
        tempPtrC.Set(info.iRemoteParty.iRemoteNumber.iTelNumber);
        }
	else if (info.iValid & RMobileCall::KCallDialledParty)
		{
		//append number for outgoing call
		tempPtrC.Set(info.iDialledParty.iTelNumber);
		}
	
	if(tempPtrC != KNullDesC)
		{
		TBuf8<100> tempBuf8;
		tempBuf8.Copy(tempPtrC); // conversion to 8 bit
    	LEAVE_IF_ERROR(params.Append(TATParam(tempBuf8, EATDQStringParam)))		

	    // type (optional)
	    TBTMonoATPhoneNumberType numType = NumberType( tempBuf8 );
	    params.AppendL(TATParam(numType));
		}

    CATResult* clcc = CATResult::NewL(EATCLCC, EATActionResult, &params);
    CleanupStack::PopAndDestroy(&params);
    TRACE_FUNC_EXIT
    return clcc;
    }

// ==========================================================
// CBtmcCallStatus::MakeVoIPClccL
// 
// ==========================================================
//
CATResult* CBtmcCallStatus::MakeVoIPClccL(const MCall &callinfo)
    {
    TRACE_FUNC_ENTRY
    RArray<TATParam> params;
    CleanupClosePushL(params);
    LEAVE_IF_ERROR(params.Append(TATParam(1))) // Index number
    
    if(callinfo.CallDirection() == CCPCall::EMobileOriginated) // outgoing
        {        
        LEAVE_IF_ERROR(params.Append(TATParam(0)))
        }
    else // unknown or incoming
        {        
        LEAVE_IF_ERROR(params.Append(TATParam(1)))            
        }       
    
    switch(callinfo.CallState()) // status
        {
        case CCPCall::EStateDialling:
            {            
            LEAVE_IF_ERROR(params.Append(TATParam(2)))
            break;
            }
        case CCPCall::EStateRinging:
            {            
            TInt status = CallStatusL();
            if(status & KActiveCallMask)
                {
                LEAVE_IF_ERROR(params.Append(TATParam(5)))
                }
            else
                {
                LEAVE_IF_ERROR(params.Append(TATParam(4)))
                }
            break;
            }
        case CCPCall::EStateAnswering:
            {            
            LEAVE_IF_ERROR(params.Append(TATParam(5)))
            break;
            }
        case CCPCall::EStateConnecting:
            {            
            LEAVE_IF_ERROR(params.Append(TATParam(3)))
            break;
            }
        case CCPCall::EStateConnected:
            {            
            LEAVE_IF_ERROR(params.Append(TATParam(0)))
            break;
            }
        case CCPCall::EStateHold:
            {         
            LEAVE_IF_ERROR(params.Append(TATParam(1)))
            break;
            }
        default:            
            LEAVE_IF_ERROR(params.Append(TATParam(0)))
        }
        
    LEAVE_IF_ERROR(params.Append(TATParam(1)))
        
    // multiparty
    LEAVE_IF_ERROR(params.Append(TATParam(0)))

    CATResult* clcc = CATResult::NewL(EATCLCC, EATActionResult, &params);
    CleanupStack::PopAndDestroy(&params);
    TRACE_FUNC_EXIT
    return clcc;
    }

// Sending Call Waiting notification
// if incoming call starts ringing and there is active call already. 
// <+CCWA: "nnn", 129, 1> when calling number is national
// <+CCWA: "nnn", 145, 1> when calling number is international (first char is '+')
// <+CCWA: "", 128, 1>    when calling number is not available

// RING is started in the following cases:
//
// 1. call is ringing (RMobileCall::EStatusRinging) and no active calls exist.
// 2. call is terminating (RMobileCall::EStatusIdle) and ringing call exists.


// Call status notification 
// +CIEV: 2,0 is generated if last active(Connected or Hold)call disconnects
// +CIEV: 2,1 is generated if first call becomes active
//

// Call setup status notification 
// +CIEV: 3,0 if call set up state ends
// +CIEV: 3,1 if an incoming call set up is ongoing
// +CIEV: 3,2 if an outgoing call set up is ongoing
// +CIEV: 3,3 if remote party being alerted in an outgoing call

void CBtmcCallStatus::HandleMobileCallEventL(
    const TName& aName,
    const TDesC& aRemotePartyName,
    RMobileCall::TMobileCallStatus aStatus, TBool aOutgoingCall)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("INDICATOR STATUS: CIEV 0x%08x, CLI %d, CCWA %d, previous CALL BITS 0x%08x, OutgoingCall %d"), 
        iProtocol.ProtocolStatus().iIndicatorNotif, iProtocol.ProtocolStatus().iCallerIdNotif, iProtocol.ProtocolStatus().iCallWaitingNotif,
        iProtocol.ProtocolStatus().iCallBits, iProtocol.ProtocolStatus().iOutgoingCallNotif))
        
    TRACE_INFO((_L("NAME '%S', REMOTE PARTY '%S', STATUS %d"), 
        &aName, &aRemotePartyName, aStatus)); (void) aName; (void) aStatus;
        
    (void) aRemotePartyName;

    TInt callBits = CallStatusL();
    //Before SLC, no event will be reported
    if (iProtocol.ProtocolStatus().iSlc)
        {
        ReportCallEventL(iProtocol.ProtocolStatus().iCallBits, callBits, aOutgoingCall);
        }
    iProtocol.ProtocolStatus().iCallBits = callBits;
    TRACE_FUNC_EXIT
    }

void CBtmcCallStatus::ReportRingAndClipL(TBool aColp)
    {
    TRACE_FUNC_ENTRY
   	CATResult* event = CATResult::NewLC(EATRING, EATUnsolicitedResult);
    RArray<TATParam> params;
    CleanupClosePushL(params);   	

    if(!aColp)
    	{
    	iProtocol.SendUnsoltResultL(*event);
    	}
 
    if (iProtocol.ProtocolStatus().iCallerIdNotif == EBTMonoATCallerIdNotifEnabled && !aColp)
        {
        TRACE_INFO((_L("CLIP")))
        GetRingingNumL( iRemoteTelNum );
        TBTMonoATPhoneNumberType numType = NumberType( iRemoteTelNum );
        params.AppendL(TATParam(iRemoteTelNum, EATDQStringParam));
        params.AppendL(TATParam(numType));
       	LEAVE_IF_ERROR(event->Set(EATCLIP, EATUnsolicitedResult, &params))
		iProtocol.SendUnsoltResultL(*event);
		}
	else if( iProtocol.ProtocolStatus().iOutgoingCallNotif && aColp ) // COLP
		{
        TRACE_INFO((_L("COLP")))
		GetOutgoingNumL(iRemoteTelNum);
        TBTMonoATPhoneNumberType numType = NumberType( iRemoteTelNum );
        params.AppendL(TATParam(iRemoteTelNum, EATDQStringParam));
        params.AppendL(TATParam(numType));
       	LEAVE_IF_ERROR(event->Set(EATCOLP, EATUnsolicitedResult, &params))
		iProtocol.SendUnsoltResultL(*event);
		}

    CleanupStack::PopAndDestroy(&params);
    CleanupStack::PopAndDestroy(event);    
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::ReportCallEventL
// 
// ==========================================================
//
void CBtmcCallStatus::ReportCallEventL(TInt aPrevStatus, TInt aNewStatus, TBool /*aOutgoing*/)
    {
    TRACE_FUNC_ENTRY
    if ( iProtocol.ProtocolStatus().iProfile == EBtmcHSP)
        {
        // HSP 1.2 requires either RING indicator or in-band ringing
        // tone, but not both.
        TRACE_INFO((_L(" HSP connection, no call indicator")))  
        return;
        }
    
    // CCWA
    if (iProtocol.ProtocolStatus().iCallWaitingNotif == EBTMonoATCallWaitingNotifEnabled && 
        (!(aPrevStatus & KCallRingingBit) && (aNewStatus & KCallRingingBit)) &&
        ((aNewStatus & KCallConnectedBit) || 
         (aNewStatus & KCallHoldBit)))
        {
        StopTimer(KRingTimerService);
        GetRingingNumL( iRemoteTelNum );
        TBTMonoATPhoneNumberType numType = NumberType( iRemoteTelNum );
        RArray<TATParam> params;
        CleanupClosePushL(params);
        params.AppendL(TATParam(iRemoteTelNum, EATDQStringParam));
        params.AppendL(TATParam(numType));
        params.AppendL(TATParam(1));
        CATResult* ccwa = CATResult::NewLC(EATCCWA, EATUnsolicitedResult, &params);
        iProtocol.SendUnsoltResultL(*ccwa);
        CleanupStack::PopAndDestroy(ccwa);
        CleanupStack::PopAndDestroy(&params);
        }
        
    // Call and setup status
    if (iProtocol.ProtocolStatus().iIndicatorNotif)
        {    
        TInt presta = 0;
        presta |= (aPrevStatus & KCallConnectedBit) | (aPrevStatus & KCallHoldBit);
        
        TInt newsta = 0;
        newsta |= (aNewStatus & KCallConnectedBit) | (aNewStatus & KCallHoldBit);
        
        TRACE_INFO((_L("call (Active and Held): presta 0x%08x, newsta 0x%08x"), presta, newsta))
        
        // Call status
        // If an active call is terminated by AT+CHLD=1, we should not send NoCall indication.
        if (presta && !newsta && 
                !(iProtocol.ActiveChldHandling() && ( aNewStatus & KCallRingingBit ) ) )
            {
            ReportCallIndicatorL(EBTMonoATCallIndicator, EBTMonoATNoCall); 
            }
        else if (!presta && newsta)
            {
	        ReportCallIndicatorL(EBTMonoATCallIndicator, EBTMonoATCallActive);
            }

        // Call setup status    
        presta = 0;
        presta |= (aPrevStatus & KCallDiallingBit) | 
                  (aPrevStatus & KCallRingingBit) | 
                  (aPrevStatus & KCallAnsweringBit) | 
                  (aPrevStatus & KCallConnectingBit);
                 
        newsta = 0;
        newsta |= (aNewStatus & KCallDiallingBit) | 
                  (aNewStatus & KCallRingingBit) | 
                  (aNewStatus & KCallAnsweringBit) | 
                  (aNewStatus & KCallConnectingBit);
        TRACE_INFO((_L("call setup: presta 0x%08x, newsta 0x%08x"), presta, newsta))

        if (presta != newsta && !(aNewStatus & KCallAnsweringBit))
            {
            TInt callSetupInd = EBTMonoATNoCallSetup;

            if (newsta & KCallRingingBit)
                {
                callSetupInd = EBTMonoATCallRinging;
                }
            else if (newsta & KCallDiallingBit)
                {
                callSetupInd = EBTMonoATCallDialling;
                }
            else if (newsta & KCallConnectingBit)
                {
                callSetupInd = EBTMonoATCallConnecting;
                }
            ReportCallIndicatorL(EBTMonoATCallSetupIndicator, callSetupInd);
            ReportCallIndicatorL(EBTMonoATCall_SetupIndicator, callSetupInd);
		    // COLP
            if( (newsta & KCallDiallingBit) &&  iProtocol.ProtocolStatus().iOutgoingCallNotif)
            	ReportRingAndClipL(ETrue);
            }
        
        // Call Held status
        if ((aPrevStatus & KCallHoldBit) && !(aNewStatus & KCallHoldBit))
            {
            ReportCallIndicatorL(EBTMonoATCallHeldIndicator, EBTMonoATNoCallHeld);
            }
        else if (!(aPrevStatus & KCallHoldBit) && (aNewStatus & KCallHoldBit))
            {
            if ((aNewStatus & KCallConnectedBit))
                {
                ReportCallIndicatorL(EBTMonoATCallHeldIndicator, EBTMonoATCallHeldAndActive);
                }
            else if (!(aNewStatus & KCallConnectedBit) && !(aNewStatus & KCallRingingBit))
                {
                ReportCallIndicatorL(EBTMonoATCallHeldIndicator, EBTMonoATCallHeldOnly);
                }
            }
        else if ((aPrevStatus & KCallHoldBit) && (aNewStatus & KCallHoldBit))
            {
            if ((aPrevStatus & KCallConnectedBit) && !(aNewStatus & KCallConnectedBit))
                {
                ReportCallIndicatorL(EBTMonoATCallHeldIndicator, EBTMonoATCallHeldOnly);
                }
            else if (!(aPrevStatus & KCallConnectedBit) && (aNewStatus & KCallConnectedBit))
                {
                ReportCallIndicatorL(EBTMonoATCallHeldIndicator, EBTMonoATCallHeldAndActive);
                }
            }
        }

    // Ring 
    // We can not send RING when the active call is ended and 
    // waiting call answered by AT+CHLD=1.
    if (!(aNewStatus & KCallConnectedBit) && 
        !(aNewStatus & KCallHoldBit) && 
        (aNewStatus & KCallRingingBit) &&
        !iProtocol.ActiveChldHandling() )
        {
        ReportRingAndClipL();
        StartTimerL(KRingTimerService, KRingInterval);
        }
    else
        {
        StopTimer(KRingTimerService);
        }
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcCallStatus::StartTimerL
// 
// ==========================================================
//
void CBtmcCallStatus::StartTimerL(TInt aService, TInt aTimeout)
    {
    if (!iTimerActive)
        {
        TRACE_FUNC_ENTRY
        iTimerActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, aService);
        iTimer.CreateLocal();
        iTimer.After(iTimerActive->iStatus, aTimeout);
        iTimerActive->GoActive();
        TRACE_FUNC_EXIT
        }
    else
        {
        TRACE_WARNING(_L("WARNING, timer is active already"))
        }
    }
// ==========================================================
// CBtmcCallStatus::StopTimer
// 
// ==========================================================
//
void CBtmcCallStatus::StopTimer(TInt aService)
    {
    if (iTimerActive && iTimerActive->ServiceId() == aService)
        {
        TRACE_FUNC_ENTRY
        delete iTimerActive;
        iTimerActive = NULL;
        iTimer.Close();
        TRACE_FUNC_EXIT
        }
    }
// ==========================================================
// CBtmcCallStatus::ReportCallIndicatorL
// 
// ==========================================================
//
void CBtmcCallStatus::ReportCallIndicatorL(TBTMonoATPhoneIndicatorId aIndicator, TInt aValue)
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
    
// End of File
