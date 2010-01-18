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
#include "btmcprotocol.h"
#include "btmcmobileline.h"
#include "btmc_defs.h"
#include "debug.h"
#include "btmccallstatus.h"
#include "btmcdummy.h"

const TInt KMobileLineActiveService = 30;
const TInt KMobileCallActiveService = 31;

// ==========================================================
// CBtmcMobileLine::NewL
// ==========================================================
CBtmcMobileLine* CBtmcMobileLine::NewL(
    CBtmcCallStatus& aParent, RMobilePhone& aPhone, const TDesC& aLineName) 
    {
    CBtmcMobileLine* self = CBtmcMobileLine::NewLC(aParent, aPhone, aLineName);
    CleanupStack::Pop(self);
    return self;
    }

// ==========================================================
// CBtmcMobileLine::NewLC
// ==========================================================
CBtmcMobileLine* CBtmcMobileLine::NewLC(
    CBtmcCallStatus& aParent, RMobilePhone& aPhone, const TDesC& aLineName) 
    {
    CBtmcMobileLine* self = new(ELeave) CBtmcMobileLine(aParent, aPhone, aLineName);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ==========================================================
// CBtmcMobileLine::~CBtmcMobileLine
// ==========================================================
CBtmcMobileLine::~CBtmcMobileLine()
    {
    TRACE_FUNC_ENTRY
    iCallActives.ResetAndDestroy();
    iCallActives.Close();
    delete iLineIncomingActive;
    delete iLineOutgoingActive;
    iLine.Close();
    }

// ==========================================================
// CBtmcMobileLine::CallInformationL
// ==========================================================
const MCall& CBtmcMobileLine::CallInformationL()
    {
    MBtmcDummy dummy = MBtmcDummy();    
    const MCall& fakeCall (dummy); 
    return fakeCall; 
    }

// ==========================================================
// CBtmcMobileLine::CallStatusL
// ==========================================================
TInt CBtmcMobileLine::CallStatusL() const
    {
    TRACE_FUNC_ENTRY
    TInt mask = 0;
    TInt count = iCallActives.Count();
    for (TInt i = 0; i < count; i++)
        {
        switch (iCallActives[i]->CallStatus()) 
            {
            case RMobileCall::EStatusDialling:
                {
                mask |= KCallDiallingBit;
                break;
                }
            case RMobileCall::EStatusRinging:
                {
                mask |= KCallRingingBit;
                break;
                }
            case RMobileCall::EStatusAnswering:
                {
                mask |= KCallAnsweringBit;
                break;
                }
            case RMobileCall::EStatusConnecting:
                {
                mask |= KCallConnectingBit;
                break;
                }
            case RMobileCall::EStatusConnected:
                {
                mask |= KCallConnectedBit;
                break;
                }
            case RMobileCall::EStatusHold:
                {
                mask |= KCallHoldBit;
                break;
                }
            default:
                break;
            }
        }
    TRACE_INFO((_L("Line '%S', CALL STATUS 0x%08x"), &iLineName, mask))
    TRACE_FUNC_EXIT
    return mask;
    }
// ==========================================================
// CBtmcMobileLine::RequestCompletedL
// ==========================================================
void CBtmcMobileLine::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("MOBILE LINE '%S'"), &iLineName))
    switch (aActive.ServiceId())
        {
        case KMobileLineActiveService:
            {
            if (aErr == KErrNone)
                {
                CBtmcCallActive* callActive = CBtmcCallActive::NewLC(
                    *this, CActive::EPriorityStandard, KMobileCallActiveService, 
                    iLine, iName);
                iCallActives.AppendL(callActive);
                iParent.HandleMobileCallEventL(iName, callActive->RemotePartyNumber(), callActive->CallStatus());
                callActive->GoActive();
                CleanupStack::Pop(callActive);
                }
            
            if(&aActive == iLineOutgoingActive)
            	iLine.NotifyCallAdded(aActive.iStatus, iName);
            else
            	iLine.NotifyIncomingCall(aActive.iStatus, iName);
            	
            aActive.GoActive();
            break;
            }
        case KMobileCallActiveService:
            {
            if (aErr == KErrNone)
                {
                CBtmcCallActive& calla = reinterpret_cast<CBtmcCallActive&>(aActive);
                RMobileCall::TMobileCallStatus status = calla.CallStatus();
                if(status != RMobileCall::EStatusDisconnecting)
                    iParent.HandleMobileCallEventL(calla.CallName(), calla.RemotePartyNumber(), calla.CallStatus());
                if (status == RMobileCall::EStatusIdle)
                    {
					if ( calla.IsEmergencyCall() )
						{
        				TRACE_INFO((_L("Emergency call object going to idle state, not deleting")))
        	        	aActive.GoActive();	
        	        	break;
						}
        	        
                    TInt idx = iCallActives.Find(&calla);
                    if (idx >= 0)
                        {
                        delete iCallActives[idx];
                        iCallActives.Remove(idx);
                        }
                    }
                else
                    {
                    aActive.GoActive();
                    }
                }
            break;
            }
        default:
            break;
        }
    TRACE_FUNC_EXIT
    }
// ==========================================================
// CBtmcMobileLine::CancelRequest
// ==========================================================
void CBtmcMobileLine::CancelRequest(TInt aServiceId)
    {
    TRACE_FUNC_ENTRY
    if (aServiceId == KMobileLineActiveService)
        {
        iLine.NotifyCallAddedCancel();
        iLine.NotifyIncomingCallCancel();
        }
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcMobileLine::CBtmcMobileLine
// ==========================================================
CBtmcMobileLine::CBtmcMobileLine(
    CBtmcCallStatus& aParent, RMobilePhone& aPhone, const TDesC& aLineName)
    : iParent(aParent), iPhone(aPhone), iLineName(aLineName)
    {
    }

// ==========================================================
// CBtmcMobileLine::ConstructL
// ==========================================================
void CBtmcMobileLine::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iLine.Open(iPhone, iLineName))
    
    TInt count = 0;
    LEAVE_IF_ERROR(iLine.EnumerateCall(count))

    for (TInt i = 0; i < count; i++)
        {
        RLine::TCallInfo info;
        LEAVE_IF_ERROR(iLine.GetCallInfo(i, info))

        CBtmcCallActive* callActive = CBtmcCallActive::NewLC(
            *this, CActive::EPriorityStandard, KMobileCallActiveService, 
            iLine, info.iCallName);
        iCallActives.AppendL(callActive);
        callActive->GoActive();
        CleanupStack::Pop(callActive);
        }

    iLineIncomingActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, KMobileLineActiveService);
    iLineOutgoingActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, KMobileLineActiveService);
    iLine.NotifyCallAdded(iLineOutgoingActive->iStatus, iName);
    iLine.NotifyIncomingCall(iLineIncomingActive->iStatus, iName);
    iLineOutgoingActive->GoActive();
    iLineIncomingActive->GoActive();
    
    TRACE_INFO((_L("MOBILE LINE '%S' constructed"), &iLineName))
    TRACE_FUNC_EXIT
    }

// ==========================================================
// CBtmcMobileLine::ActiveCalls()
// ==========================================================
const RPointerArray<CBtmcCallActive>& CBtmcMobileLine::ActiveCalls() const
    {
    return iCallActives;
    }

// ==========================================================
// CBtmcMobileLine::Phone()
// ==========================================================
RMobilePhone& CBtmcMobileLine::Phone() 
    {
    return iPhone;
    }

// ==========================================================
// CBtmcMobileLine::IsVoip()
// ==========================================================
TBool CBtmcMobileLine::IsVoip() 
    {
    return EFalse;
    }

// End of file
