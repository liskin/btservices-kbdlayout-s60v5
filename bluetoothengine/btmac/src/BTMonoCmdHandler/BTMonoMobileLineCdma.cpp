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

#include "BTMonoMobileLine.h"
#include "BTMonoCdmaIncomingFlash.h"
#include "BTMonoEventObserver.h"
#include "BTMonoCmdHandlerDefs.h"
#include "Debug.h"

const TInt KMobileLineActiveService = 1;
const TInt KMobileCallActiveService = 2;
const TInt KMobileCdmaActiveService = 3;

// -----------------------------------------------------------------------------
// CBTMonoMobileLine::NewL
// -----------------------------------------------------------------------------
CBTMonoMobileLine* CBTMonoMobileLine::NewL(
    MBTMonoEventObserver& aObserver, RMobilePhone& aPhone, const TDesC& aLineName) 
    {
    CBTMonoMobileLine* self = CBTMonoMobileLine::NewLC(aObserver, aPhone, aLineName);
	CleanupStack::Pop(self);
	return self;
    }

// -----------------------------------------------------------------------------
// CBTMonoMobileLine::NewLC
// -----------------------------------------------------------------------------
CBTMonoMobileLine* CBTMonoMobileLine::NewLC(
    MBTMonoEventObserver& aObserver, RMobilePhone& aPhone, const TDesC& aLineName) 
    {
    CBTMonoMobileLine* self = new(ELeave) CBTMonoMobileLine(aObserver, aPhone, aLineName);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
    }

// -----------------------------------------------------------------------------
// CBTMonoMobileLine::~CBTMonoMobileLine
// -----------------------------------------------------------------------------
CBTMonoMobileLine::~CBTMonoMobileLine()
    {
	TRACE_FUNC_ENTRY_THIS
    iCallActives.ResetAndDestroy();
    iCallActives.Close();
    iCdmaActives.ResetAndDestroy();
    iCdmaActives.Close();
    delete iLineActive;
    iLine.Close();
    }

TInt CBTMonoMobileLine::ActiveCallCount() const
    {
    TInt count = iCallActives.Count();
    TInt activecount = 0;
    RMobileCall::TMobileCallStatus sta;
    for (TInt i = 0; i < count; i++)
        {
        sta = iCallActives[i]->CallStatus();
        if (sta == RMobileCall::EStatusConnected || sta == RMobileCall::EStatusHold)
            {
            activecount++;
            }
        }
    return activecount;
    }

RMobileCall::TMobileCallStatus CBTMonoMobileLine::CallStatus(const TName& aCallName) const
    {
    TInt count = iCallActives.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iCallActives[i]->CallName().Compare(aCallName) == 0)
            {
            return iCallActives[i]->CallStatus();
            }
        }
    return RMobileCall::EStatusUnknown;
    }


void CBTMonoMobileLine::RequestCompletedL(CBTMonoActive& aActive, TInt aErr)
    {
    switch (aActive.ServiceId())
        {
        case KMobileLineActiveService:
            {
            if (aErr == KErrNone)
        		{
                CBTMonoCallActive* callActive = CBTMonoCallActive::NewLC(
                    *this, CActive::EPriorityStandard, KMobileCallActiveService, 
                    iLine, iName);
                iCallActives.AppendL(callActive);
                
                CBTMonoCdmaIncomingFlash* cdmaActive = CBTMonoCdmaIncomingFlash::NewLC(
                    *this, CActive::EPriorityStandard, KMobileCdmaActiveService, 
                    iLine, iName);
                iCdmaActives.AppendL(cdmaActive);
                iObserver.HandleMobileCallEventL(iName, callActive->CallStatus());
                callActive->GoActive();
                cdmaActive->GoActive();
                CleanupStack::Pop(callActive);
                CleanupStack::Pop(cdmaActive);
        		}
        	iLine.NotifyCallAdded(aActive.iStatus, iName);
            aActive.GoActive();
            break;
            }
        case KMobileCallActiveService:
            {
    		CBTMonoCallActive& calla = reinterpret_cast<CBTMonoCallActive&>(aActive);
    	    iObserver.HandleMobileCallEventL(calla.CallName(), calla.CallStatus());
    	    RMobileCall::TMobileCallStatus status = calla.CallStatus();
    	    if (status == RMobileCall::EStatusIdle)
    	        {
    	        TInt idx = iCallActives.Find(&calla);
    	        if (idx >= 0)
    	            {
    	            delete iCallActives[idx];
    	            iCallActives.Remove(idx);
    	            delete iCdmaActives[idx];
    	            iCdmaActives.Remove(idx);
    	            }
    	        }
    		else
    		    {
    		    aActive.GoActive();
    		    }
            break;
            }
        case KMobileCdmaActiveService:
            {
            iObserver.HandleCdmaIncomingFlashEventL();
            aActive.GoActive();
            break;
            }
        default:
            break;
        }
    }

void CBTMonoMobileLine::CancelRequest(TInt aServiceId)
    {
    if (aServiceId == KMobileLineActiveService)
        {
        iLine.NotifyCallAddedCancel();
        }
    }

// -----------------------------------------------------------------------------
// CBTMonoMobileLine::CBTMonoMobileLine
// -----------------------------------------------------------------------------
CBTMonoMobileLine::CBTMonoMobileLine(
    MBTMonoEventObserver& aObserver, RMobilePhone& aPhone, const TDesC& aLineName)
    : iObserver(aObserver), iPhone(aPhone), iLineName(aLineName)
    {
    }

// -----------------------------------------------------------------------------
// CBTMonoMobileLine::ConstructL
// -----------------------------------------------------------------------------
void CBTMonoMobileLine::ConstructL()
    {
    LEAVE_IF_ERROR(iLine.Open(iPhone, iLineName))
    
	TInt count = 0;
	LEAVE_IF_ERROR(iLine.EnumerateCall(count))

	for (TInt i = 0; i < count; i++)
	    {
		RLine::TCallInfo info;
        LEAVE_IF_ERROR(iLine.GetCallInfo(i, info))
        CBTMonoCallActive* callActive = CBTMonoCallActive::NewLC(
            *this, CActive::EPriorityStandard, KMobileCallActiveService, 
            iLine, info.iCallName);
        iCallActives.AppendL(callActive);
        
        CBTMonoCdmaIncomingFlash* cdmaActive = CBTMonoCdmaIncomingFlash::NewLC(
            *this, CActive::EPriorityStandard, KMobileCdmaActiveService, 
            iLine, info.iCallName);
        iCdmaActives.AppendL(cdmaActive);
        
        callActive->GoActive();
        cdmaActive->GoActive();
        CleanupStack::Pop(callActive);
        CleanupStack::Pop(cdmaActive);
        }

    iLineActive = CBTMonoActive::NewL(*this, CActive::EPriorityStandard, KMobileLineActiveService);
	iLine.NotifyCallAdded(iLineActive->iStatus, iName);
    iLineActive->GoActive();
    TRACE_FUNC_THIS
    }
    
// End of file
