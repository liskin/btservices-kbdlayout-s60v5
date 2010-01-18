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


#include <e32std.h>
#include <e32base.h>

#include "shutdown.h"
#include "debug.h"

// ----------------------------------------------------------------------

const TInt CLayoutMgrShutdown::KShutdownInterval = 2000000; // 2 seconds

// ----------------------------------------------------------------------

CLayoutMgrShutdown* CLayoutMgrShutdown::NewL()
    {
    TRACE_INFO( (_L("[HID]\tCLayoutMgrShutdown::NewL()")));

    CLayoutMgrShutdown* self = new (ELeave) CLayoutMgrShutdown;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

void CLayoutMgrShutdown::ConstructL()
    {
    TRACE_INFO( (_L("[HID]\tCLayoutMgrShutdown::ConstructL() 0x%08x"), this));

    CActiveScheduler::Add(this);
    User::LeaveIfError(iTimer.CreateLocal());
    }

CLayoutMgrShutdown::CLayoutMgrShutdown()
        : CActive(CActive::EPriorityStandard)
    {}

CLayoutMgrShutdown::~CLayoutMgrShutdown()
    {
    TRACE_INFO( (_L("[HID]\tCLayoutMgrShutdown::~CLayoutMgrShutdown()")));

    Cancel();
    iTimer.Close();
    }

void CLayoutMgrShutdown::Start()
    {
    iTimer.After(iStatus, KShutdownInterval);
    SetActive();
    }

void CLayoutMgrShutdown::DoCancel()
    {
    iTimer.Cancel();
    }

void CLayoutMgrShutdown::RunL()
    {
    // Timer has expired, so cause the server to close down:
    CActiveScheduler::Stop();
    }

// ----------------------------------------------------------------------
