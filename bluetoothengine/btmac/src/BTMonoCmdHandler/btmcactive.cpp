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


#include "btmcactive.h"
#include "debug.h"


// -----------------------------------------------------------------------------
// CBtmcActive::NewL
// -----------------------------------------------------------------------------
CBtmcActive* CBtmcActive::NewL(MBtmcActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aServiceId)
    {
    CBtmcActive* self = CBtmcActive::NewLC(aObserver, aPriority, aServiceId);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcActive::NewLC
// -----------------------------------------------------------------------------
CBtmcActive* CBtmcActive::NewLC(MBtmcActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aServiceId)
    {
    CBtmcActive* self = new (ELeave) CBtmcActive(aObserver, aPriority, aServiceId);
    CleanupStack::PushL(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtmcActive::~CBtmcActive
// -----------------------------------------------------------------------------
CBtmcActive::~CBtmcActive()
    {
    Cancel();
    TRACE_FUNC
    }

// -------------------------------------------------------------------------------
// CBtmcActive::GoActive
// -------------------------------------------------------------------------------
void CBtmcActive::GoActive()
    {
    TRACE_ASSERT(!IsActive(), KErrGeneral);
    SetActive();
    TRACE_FUNC
    }

TInt CBtmcActive::ServiceId() const
    {
    return iServiceId;
    }

void CBtmcActive::SetServiceId(TInt aServiceId)
    {
    iServiceId = aServiceId;
    }

// -------------------------------------------------------------------------------
// CBtmcActive::RunL
// -------------------------------------------------------------------------------
void CBtmcActive::RunL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Completed service %d, status %d"), iServiceId, iStatus.Int()))
    iObserver.RequestCompletedL(*this, iStatus.Int());
    TRACE_FUNC_EXIT
    }

// -------------------------------------------------------------------------------
// CBtmcActive::RunError
// This is really bad as we are just 'eating' the error not doing anything with it
// This needs to re-evaluated and done better as later date.
// -------------------------------------------------------------------------------
TInt CBtmcActive::RunError(TInt aError)
    {
    TRACE_INFO((_L("Service %d RunError with %d"), iServiceId, aError))
    (void) aError;
    return KErrNone;
    } 

// -------------------------------------------------------------------------------
// CBtmcActive::DoCancel
// -------------------------------------------------------------------------------
void CBtmcActive::DoCancel()
    {
    TRACE_INFO((_L("DoCancel service %d"), iServiceId))
    iObserver.CancelRequest(iServiceId);
    }

// -----------------------------------------------------------------------------
// CBtmcActive::CBtmcActive
// -----------------------------------------------------------------------------
CBtmcActive::CBtmcActive(MBtmcActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aServiceId)
    : CActive(aPriority), iObserver(aObserver), iServiceId(aServiceId)
    {
    CActiveScheduler::Add(this);
    TRACE_FUNC
    }

MBtmcActiveObserver& CBtmcActive::Observer()
    {
    return iObserver;
    }

// End of File
