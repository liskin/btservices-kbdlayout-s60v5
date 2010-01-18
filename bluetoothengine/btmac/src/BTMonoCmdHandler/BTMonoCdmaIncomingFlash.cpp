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
* Description:  General Active Object offering asynchronous service
*
*/


#include "BTMonoCdmaIncomingFlash.h"
#include "BTMonoCallActive.h"
#include "Debug.h"

_LIT(KDefaultRemoteNumber, "n/a");

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::NewL
// -----------------------------------------------------------------------------
//
CBTMonoCdmaIncomingFlash* CBTMonoCdmaIncomingFlash::NewL(
    MBTMonoActiveObserver& aObserver, 
    CActive::TPriority aPriority,
    TInt aServiceId,
	RMobileLine& aLine, 
	const TName& aName)
    {
	CBTMonoCdmaIncomingFlash * self = 
	    CBTMonoCdmaIncomingFlash::NewLC(aObserver, aPriority, aServiceId, aLine, aName);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::NewLC
// -----------------------------------------------------------------------------
CBTMonoCdmaIncomingFlash* CBTMonoCdmaIncomingFlash::NewLC(
    MBTMonoActiveObserver& aObserver, 
    CActive::TPriority aPriority, 
    TInt aServiceId,
    RMobileLine& aLine,
    const TName& aName)
    {
    CBTMonoCdmaIncomingFlash* self = 
        new (ELeave) CBTMonoCdmaIncomingFlash(aObserver, aPriority, aServiceId, aLine, aName);
    CleanupStack::PushL(self);
    self->ConstructL();
	return self;
    }

// Destructor
CBTMonoCdmaIncomingFlash::~CBTMonoCdmaIncomingFlash()
	{
	Cancel();
	iCdmaCall.Close();
	TRACE_FUNC_THIS
    }

void CBTMonoCdmaIncomingFlash::GoActive()
    {
    if (!IsActive())
        {
    	iCdmaCall.NotifyIncomingNetworkFlashWithInfo(iStatus, iFlashMessagePckg);
        SetActive();
        }
    TRACE_FUNC_THIS
    }

// -----------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::DoCancel
// -----------------------------------------------------------------------------
//
void CBTMonoCdmaIncomingFlash::DoCancel()
    {
	iCdmaCall.CancelAsyncRequest(ECdmaMobileCallNotifyIncomingNetworkFlashWithInfo);
    }

// -----------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::RunL
// 
// -----------------------------------------------------------------------------
//
void CBTMonoCdmaIncomingFlash::RunL()
    {
    TRACE_FUNC_ENTRY_THIS
	Observer().RequestCompletedL(*this, iStatus.Int());
	TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::RunError
// -------------------------------------------------------------------------------
TInt CBTMonoCdmaIncomingFlash::RunError(TInt /*aErr*/)
    {
	Cancel();
	iCdmaCall.Close();
    TRACE_FUNC_THIS
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::CBTMonoCdmaIncomingFlash
// -----------------------------------------------------------------------------
//
CBTMonoCdmaIncomingFlash::CBTMonoCdmaIncomingFlash(
    MBTMonoActiveObserver& aObserver,
    CActive::TPriority aPriority,
    TInt aServiceId,
    RMobileLine& aLine,
    const TName& aName)
    : CBTMonoActive(aObserver, aPriority, aServiceId),
	  iCdmaLine(aLine),
	  iCallName(aName),
	  iFlashMessagePckg(RCdmaMobileCall::TMobileCallIncomingFlashMessageV1())    
    {
    }

// -----------------------------------------------------------------------------
// CBTMonoCdmaIncomingFlash::ConstructL
// -----------------------------------------------------------------------------
//
void CBTMonoCdmaIncomingFlash::ConstructL()
    {
	LEAVE_IF_ERROR(iCdmaCall.OpenExistingCall(iCdmaLine, iCallName))
	TRACE_FUNC_THIS
    }

// End of File
