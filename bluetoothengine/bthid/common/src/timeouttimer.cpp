/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of application class
 *
*/


#include "timeouttimer.h"
#include "timeoutnotifier.h"

CTimeOutTimer* CTimeOutTimer::NewL(const TInt aPriority,
        MTimeOutNotifier& aTimeOutNotify)
    {
    CTimeOutTimer* self = CTimeOutTimer::NewLC(aPriority, aTimeOutNotify);
    CleanupStack::Pop(self);
    return self;
    }

CTimeOutTimer* CTimeOutTimer::NewLC(const TInt aPriority,
        MTimeOutNotifier& aTimeOutNotify)
    {
    CTimeOutTimer* self = new (ELeave) CTimeOutTimer(aPriority,
            aTimeOutNotify);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CTimeOutTimer::CTimeOutTimer(const TInt aPriority,
        MTimeOutNotifier& aTimeOutNotify) :
    CTimer(aPriority), iNotify(aTimeOutNotify)
    {
    }

CTimeOutTimer::~CTimeOutTimer()
    {
    }

void CTimeOutTimer::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add(this);
    }

void CTimeOutTimer::RunL()
    {
    // Timer request has completed, so notify the timer's owner
    if (iStatus == KErrNone)
        {
        iNotify.TimerExpired();
        }
    else
        {
        User::Panic(KTimeOutTimerPanic, ETimerFailed);
        }
    }
