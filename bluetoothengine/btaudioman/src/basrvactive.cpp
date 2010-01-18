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
* Description:  The base active class definition 
*
*/


#include "basrvactive.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBasrvActive* CBasrvActive::New(MBasrvActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    return new CBasrvActive(aObserver, aPriority, aRequestId);
    }

CBasrvActive* CBasrvActive::NewL(MBasrvActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    CBasrvActive* self = CBasrvActive::NewLC(aObserver, aPriority, aRequestId);
    CleanupStack::Pop(self);
    return self;
    }

CBasrvActive* CBasrvActive::NewLC(MBasrvActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    CBasrvActive* self = new (ELeave) CBasrvActive(aObserver, aPriority, aRequestId);
    CleanupStack::PushL(self);
    return self;
    }

CBasrvActive::~CBasrvActive()
    {
    Cancel();
    TRACE_FUNC
    }

void CBasrvActive::GoActive()
    {
    TRACE_ASSERT(!IsActive(), -2);
    SetActive();
    TRACE_INFO((_L("CBasrvActive: Service %d starts"), iRequestId))
    }

TInt CBasrvActive::RequestId() const
    {
    return iRequestId;
    }

void CBasrvActive::SetRequestId(TInt aRequestId)
    {
    iRequestId = aRequestId;
    }

void CBasrvActive::DoCancel()
    {
    iObserver.CancelRequest(*this);
    TRACE_INFO((_L("Service %d cancelled"), iRequestId))
    }
    
void CBasrvActive::RunL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("Service %d completed with %d"), iRequestId, iStatus.Int()))
    iObserver.RequestCompletedL(*this);
    TRACE_FUNC_EXIT
    }
    
TInt CBasrvActive::RunError(TInt aError)
    {
    TRACE_INFO((_L("Service %d RunError with %d"), iRequestId, aError))
    (void) aError;
    return KErrNone;
    } 

CBasrvActive::CBasrvActive(MBasrvActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    : CActive(aPriority), iObserver(aObserver), iRequestId(aRequestId)
    {
    CActiveScheduler::Add(this);
    TRACE_FUNC
    }

MBasrvActiveObserver& CBasrvActive::Observer()
    {
    return iObserver;
    }
