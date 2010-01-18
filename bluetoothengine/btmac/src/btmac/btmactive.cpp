/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The BTMAC base Active Object definition
*
*/


#include "btmactive.h"
#include "btmdefs.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmActive* CBtmActive::NewL(MBtmActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    CBtmActive* self = new (ELeave) CBtmActive(aObserver, aPriority, aRequestId);
    return self;
    }

CBtmActive::~CBtmActive()
    {
    Cancel();
    TRACE_FUNC
    }

void CBtmActive::GoActive()
    {
    TRACE_ASSERT(!IsActive(), EBTPanicRequestOutstanding);
    SetActive();
    TRACE_FUNC
    }

TInt CBtmActive::RequestId() const
    {
    return iRequestId;
    }

void CBtmActive::SetRequestId(TInt aRequestId)
    {
    iRequestId = aRequestId;
    }

void CBtmActive::DoCancel()
    {
    iObserver.CancelRequest(*this);
    TRACE_INFO((_L("Service %d cancelled"), iRequestId))
    }
    
void CBtmActive::RunL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Service %d completed with %d"), iRequestId, iStatus.Int()))
    iObserver.RequestCompletedL(*this);
    TRACE_FUNC_EXIT
    }
    
TInt CBtmActive::RunError(TInt aError)
    {
    TRACE_INFO((_L("Service %d RunError with %d"), iRequestId, aError))
    (void) aError;
    return KErrNone;
    } 

CBtmActive::CBtmActive(MBtmActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    : CActive(aPriority), iObserver(aObserver), iRequestId(aRequestId)
    {
    CActiveScheduler::Add(this);
    TRACE_FUNC
    }

MBtmActiveObserver& CBtmActive::Observer()
    {
    return iObserver;
    }
