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

CGenericActive* CGenericActive::New(MGenericActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    return new CGenericActive(aObserver, aPriority, aRequestId);
    }

CGenericActive* CGenericActive::NewL(MGenericActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    {
    CGenericActive* self = new (ELeave) CGenericActive(aObserver, aPriority, aRequestId);
    return self;
    }

CGenericActive::~CGenericActive()
    {
    Cancel();
    }

void CGenericActive::GoActive()
    {
    SetActive();
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
    }
    
void CGenericActive::RunL()
    {
    iObserver.RequestCompletedL(*this);
    }
    
TInt CGenericActive::RunError(TInt aError)
    {
    return iObserver.HandleRunError(aError);
    }

CGenericActive::CGenericActive(MGenericActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId)
    : CActive(aPriority), iObserver(aObserver), iRequestId(aRequestId)
    {
    CActiveScheduler::Add(this);
    }
