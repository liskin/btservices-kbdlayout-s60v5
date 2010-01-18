/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include "btrccActive.h"
#include "debug.h"


// -----------------------------------------------------------------------------
// CBTRCCActive::NewL
// -----------------------------------------------------------------------------
CBTRCCActive* CBTRCCActive::NewL(MBTRCCActiveObserver& aObserver, 
        TInt aServiceId, CActive::TPriority aPriority)
    {
    CBTRCCActive* self = CBTRCCActive::NewLC(aObserver, aServiceId, aPriority);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTRCCActive::NewLC
// -----------------------------------------------------------------------------
CBTRCCActive* CBTRCCActive::NewLC(MBTRCCActiveObserver& aObserver,  
    TInt aServiceId, CActive::TPriority aPriority)
    {
    CBTRCCActive* self = new (ELeave) CBTRCCActive(aObserver, aServiceId, aPriority);
    CleanupStack::PushL(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTRCCActive::~CBTRCCActive
// -----------------------------------------------------------------------------
CBTRCCActive::~CBTRCCActive()
    {
    Cancel();
    TRACE_FUNC
    }

// -------------------------------------------------------------------------------
// CBTRCCActive::GoActive
// -------------------------------------------------------------------------------
void CBTRCCActive::GoActive()
    {
    TRACE_ASSERT(!IsActive(), KErrGeneral);
    SetActive();
    TRACE_FUNC
    }

TInt CBTRCCActive::ServiceId() const
    {
    return iServiceId;
    }

void CBTRCCActive::SetServiceId(TInt aServiceId)
    {
    iServiceId = aServiceId;
    }

// -------------------------------------------------------------------------------
// CBTRCCActive::RunL
// -------------------------------------------------------------------------------
void CBTRCCActive::RunL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Completed service %d, status %d"), iServiceId, iStatus.Int()))
    iObserver.RequestCompletedL(*this, iStatus.Int());
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBTRCCActive::DoCancel
// -------------------------------------------------------------------------------
void CBTRCCActive::DoCancel()
    {
    TRACE_INFO((_L("DoCancel service %d"), iServiceId))
    iObserver.CancelRequest(iServiceId);
    }

// -----------------------------------------------------------------------------
// CBTRCCActive::CBTRCCActive
// -----------------------------------------------------------------------------
CBTRCCActive::CBTRCCActive(MBTRCCActiveObserver& aObserver, 
    TInt aServiceId, CActive::TPriority aPriority)
    : CActive(aPriority), iObserver(aObserver), iServiceId(aServiceId) 
    {
    CActiveScheduler::Add(this);
    TRACE_FUNC
    }

MBTRCCActiveObserver& CBTRCCActive::Observer()
    {
    return iObserver;
    }

// End of File
