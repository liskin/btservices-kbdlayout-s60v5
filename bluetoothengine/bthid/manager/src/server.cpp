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


#include <e32svr.h>

#include "layoutmgr.h"
#include "server.h"
#include "session.h"
#include "shutdown.h"
#include "library.h"
#include "mapping.h"
#include "debug.h"


// ----------------------------------------------------------------------

CLayoutServer::CLayoutServer()
        : CGenericServer(CActive::EPriorityStandard),
        iIsNokiaSu8(EFalse), iFoundLayout(EFalse)
    {
    // nothing else to do
    }

CLayoutServer::~CLayoutServer()
    {
    TRACE_FUNC ( _L("[HID]\t~CLayoutServer") );
    delete iLayoutLibrary;
    delete iShutdown;
    }

CLayoutServer* CLayoutServer::NewL()
    {
    CLayoutServer* self = CLayoutServer::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CLayoutServer* CLayoutServer::NewLC()
    {
    CLayoutServer* self = new (ELeave) CLayoutServer;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ----------------------------------------------------------------------

void CLayoutServer::ConstructL()
    {
    TRACE_FUNC( (_L("[HID]\tCLayoutServer::ConstructL()") ));
    // Construct delayed shutdown timer:
    iShutdown = CLayoutMgrShutdown::NewL();

    // Need a keyboard layout library:
    iLayoutLibrary = CLayoutLibrary::NewL();

    // Start the server:
    StartL(KLayoutServerName);

    // Shut down after a few seconds unless a client connects:
    iShutdown->Start();
    }

// ----------------------------------------------------------------------
CSession2* CLayoutServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
    {
    // Version number of this server software:
    const TVersion KServerVersion(1, 0, 0);

    // Ensure we're compatible with the client:
    if (!User::QueryVersionSupported(KServerVersion, aVersion))
        {
        User::Leave(KErrNotSupported);
        }

    CLayoutSession* session = CLayoutSession::NewL();
    const_cast<CLayoutServer*>(this)->SessionAdded();

    return session;
    }

void CLayoutServer::SessionAdded()
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::SessionAdded() count %d"),iSessionCount+1));

    ++iSessionCount;
    iShutdown->Cancel();
    }

void CLayoutServer::SessionClosed()
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::SessionClosed(), count %d"),iSessionCount-1));

    --iSessionCount;
    __ASSERT_DEBUG(iSessionCount >= 0, PanicServer(ENegativeSessionCount));

    if (iSessionCount < 1)
        {
        iSessionCount = 0;

        // If that was the last session, shut down the server:
        iShutdown->Start();
        }
    }

// ----------------------------------------------------------------------

// ServiceL() leaves will end up here:
//
TInt CLayoutServer::RunError(TInt aErr)
    {
    if (aErr == KErrBadDescriptor)
        {
        PanicClient(Message(), EBadDescriptor);
        }
    else
        {
        // Report the error to the client:
        Message().Complete(aErr);
        }

    // The leave will result in an early return from CServer::RunL(),
    // skipping the call to request another message. So we do that now
    // in order to keep the server running:
    ReStart();

    return KErrNone; // handled the error fully
    }

// ----------------------------------------------------------------------
void CLayoutServer::PanicClient(const RMessage2& aMessage, TInt aPanic)
    {
    __DEBUGGER();
    aMessage.Panic(KLayoutServerPanic, aPanic);
    }

void CLayoutServer::PanicServer(TInt aPanic)
    {
    __DEBUGGER();
    User::Panic(KLayoutServerPanic, aPanic);
    }

// ----------------------------------------------------------------------

void CLayoutServer::ThreadFunctionL(TStartupRequest* /*aStartup*/)
    {
    // This is a new thread, so we require a new active scheduler:
    CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(activeScheduler);
    CActiveScheduler::Install(activeScheduler);

    // Construct our server
    CLayoutServer::NewLC();

    User::RenameThread(KLayoutServerName);
    RProcess::Rendezvous(KErrNone);

    // Start handling requests:
    CActiveScheduler::Start();

    // Server finished:
    CleanupStack::PopAndDestroy(2, activeScheduler);
    }

TInt CLayoutServer::ThreadFunction(TAny* aThreadArg)
    {
    __UHEAP_MARK;

    TInt err = KErrNone;
    TStartupRequest* startup = static_cast<TStartupRequest*>(aThreadArg);

    CTrapCleanup* cleanupStack = CTrapCleanup::New();
    if (cleanupStack)
        {
        TRAP(err, ThreadFunctionL(startup));
        delete cleanupStack;

        if (err != KErrNone)
            {
            // Failed, but we must still signal the client or it will
            // hang:
            RProcess::Rendezvous(err);
            }
        }
    else
        {
        // Again, failed, but we still need to signal the client:
        RProcess::Rendezvous(KErrNoMemory);
        }

    __UHEAP_MARKEND;

    return err;
    }

// ----------------------------------------------------------------------

void CLayoutServer::SetInitialLayoutL(TInt aCountry,
                                      TInt aVendor, TInt aProduct)
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::SetInitialLayoutL(%d, %d, %d)\n"),
                 aCountry, aVendor, aProduct));

    // Select an initial layout ID based on parameters read from the device:
    TFindLayoutId findLayout(aCountry, aVendor, aProduct);
    iInitialLayout = findLayout.LayoutId();
    iIsNokiaSu8 = findLayout.IsNokiaSu8();
    iFoundLayout = findLayout.FoundLayout();

    TRACE_INFO( (_L("[HID]\t  Layout ID is %d (0x%x)\n"),
                 iInitialLayout, iInitialLayout));

    // Set this as the system keyboard layout:
    User::LeaveIfError(SetLayout(iInitialLayout));
    }

// ----------------------------------------------------------------------

TInt CLayoutServer::SetLayout(TInt aLayoutId)
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::SetLayout(%d)\n"), aLayoutId) );

    // Change the single system keyboard layout:

    TInt err = iLayoutLibrary->SetLayout(aLayoutId);

    if (err == KErrNone)
        {
        // Notify each driver in turn:
        TRACE_INFO( (_L("[HID]\t iSessionCount = %d\n"), iSessionCount));
        TDblQueIter<CSession2> sessionList = iSessionIter;
        sessionList.SetToFirst();

        CSession2* session = sessionList++;

        while (session)
            {

            // We know that each session is actually a CLayoutSession:
            CLayoutSession* lsession = static_cast<CLayoutSession*>(session);
            lsession->SetLayout(iLayoutLibrary);
            session = sessionList++;
            }
        }

    return err;
    }

TInt CLayoutServer::Layout() const
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::Layout() = %d\n"),
                 iLayoutLibrary->Id()));
    return iLayoutLibrary->Id();
    }

TInt CLayoutServer::InitialLayout() const
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::InitialLayout() = %d\n"),
                 iInitialLayout));
    return iInitialLayout;
    }

TBool CLayoutServer::IsNokiaSu8() const
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::IsNokiaSu8() = %d\n"),
                 iIsNokiaSu8));
    return iIsNokiaSu8;
    }

TBool CLayoutServer::FoundLayout() const
    {
    TRACE_INFO( (_L("[HID]\tCLayoutServer::FoundLayout() = %d\n"),
                 iFoundLayout));
    return iFoundLayout;
    }

// ----------------------------------------------------------------------

TInt E32Main()
    {
    TStartupRequest startup;
    TInt error = CGenericServer::GetStartupFromCommandLine(startup);

    if (KErrNone == error)
        {
        error = CLayoutServer::ThreadFunction(&startup);
        }

    return error;
    }


// ----------------------------------------------------------------------

