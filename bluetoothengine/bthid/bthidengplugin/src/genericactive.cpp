/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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


#include "genericactive.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CGenericActive* CGenericActive::New(MGenericActiveObserver& aObserver,
        CActive::TPriority aPriority, TInt aRequestId)
    {
    return new CGenericActive(aObserver, aPriority, aRequestId);
    }

CGenericActive* CGenericActive::NewL(MGenericActiveObserver& aObserver,
        CActive::TPriority aPriority, TInt aRequestId)
    {
    CGenericActive* self = new (ELeave) CGenericActive(aObserver, aPriority,
            aRequestId);
    return self;
    }

CGenericActive::~CGenericActive()
    {
    Cancel();
        TRACE_FUNC
    }

void CGenericActive::GoActive()
    {
        TRACE_ASSERT(!IsActive(), -2);
    SetActive();
        TRACE_INFO((_L("CGenericActive: Service %d starts"), iRequestId))
    }

TInt CGenericActive::RequestId() const
    {
    return iRequestId;
    }

void CGenericActive::SetRequestId(TInt aRequestId)
    {
    iRequestId = aRequestId;
    }

TRequestStatus& CGenericActive::RequestStatus()
    {
    return iStatus;
    }

void CGenericActive::DoCancel()
    {
    iObserver.CancelRequest(*this);
        TRACE_INFO((_L("Service %d cancelled"), iRequestId))
    }

void CGenericActive::RunL()
    {
        TRACE_INFO((_L("Service %d completed with %d"), iRequestId, iStatus.Int()))
    iObserver.RequestCompletedL(*this);
    }

TInt CGenericActive::RunError(TInt aError)
    {
        TRACE_INFO((_L("Service %d RunError with %d"), iRequestId, aError))
    (void) aError;
    return KErrNone;
    }

CGenericActive::CGenericActive(MGenericActiveObserver& aObserver,
        CActive::TPriority aPriority, TInt aRequestId) :
    CActive(aPriority), iObserver(aObserver), iRequestId(aRequestId)
    {
    CActiveScheduler::Add(this);
        TRACE_FUNC
    }
