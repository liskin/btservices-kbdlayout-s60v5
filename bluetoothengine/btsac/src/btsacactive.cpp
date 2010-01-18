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


#include "btsacactive.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtsacActive* CBtsacActive::NewL(MBtsacActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    CBtsacActive* self = CBtsacActive::NewLC(aObserver, aPriority, aRequestId);
    CleanupStack::Pop(self);
    return self;
    }

CBtsacActive* CBtsacActive::NewLC(MBtsacActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    CBtsacActive* self = new (ELeave) CBtsacActive(aObserver, aPriority, aRequestId);
    CleanupStack::PushL(self);
    return self;
    }

CBtsacActive::~CBtsacActive()
    {
    TRACE_FUNC
    Cancel();
    }

void CBtsacActive::GoActive()
    {
    TRACE_FUNC
    if( !IsActive() )
    	SetActive();
    else
    	{
    	TRACE_INFO((_L("CBtsacActive::GoActive, Already active")))
    	}
    }

TInt CBtsacActive::RequestId() const
    {
    return iRequestId;
    }

void CBtsacActive::SetRequestId(TInt aRequestId)
    {
    iRequestId = aRequestId;
    }

void CBtsacActive::DoCancel()
    {
    TRACE_FUNC
    iObserver.CancelRequest(*this);
    }
    
void CBtsacActive::RunL()
    {
    TRACE_FUNC
    iObserver.RequestCompletedL(*this);
    }
    
TInt CBtsacActive::RunError(TInt aError)
    {
    TRACE_FUNC
    (void) aError;
    return KErrNone;
    } 

CBtsacActive::CBtsacActive(MBtsacActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    : CActive(aPriority), iObserver(aObserver), iRequestId(aRequestId)
    {
    TRACE_FUNC
    CActiveScheduler::Add(this);
    }

MBtsacActiveObserver& CBtsacActive::Observer()
    {
    return iObserver;
    }
