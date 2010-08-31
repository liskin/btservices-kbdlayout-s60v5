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
* Description:  Contains global dll entry of BT Accessory Server which also creates 
*                first instance of the server.
*
*/


#include "debug.h"
#include "BTAccServer.h"        // create instance of server
#include "BTAccClientSrv.h"        // server name

static void RunServerL();

/**
* Main entry-point for the server process. 
* Its a global definition, so GLDEF_C keyword can be omitted though.
**/
GLDEF_C TInt E32Main()
    {
    TRACE_STATIC_FUNC
    //TODO uncomment UHEAP macros after orbit & Qt memory leaks are resolved till then it should be commented.
//    __UHEAP_MARK;
    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt r=KErrNoMemory;
    if (cleanup)
        {
        TRAP(r,RunServerL());
        delete cleanup;
        }
    //
//    __UHEAP_MARKEND;
    return r;
    }

/**
Create the active scheduler and server instances, and start the 
server.
*/
static void RunServerL()
    {
    
    User::LeaveIfError(User::RenameThread(KBTAudioManName));    

    // Create and install the active scheduler.
    CActiveScheduler* scheduler = new(ELeave) CActiveScheduler;
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);

    // Create the server. NB Odd syntax in this line because we never refer to 
    // it directly- it's always owned by the cleanup stack!
    (void)CBTAccServer::NewLC();

    // Initialisation complete, now signal the client
    RProcess::Rendezvous(KErrNone);

    // Ready to run. This only returns when the server is closing down.
    CActiveScheduler::Start();

    // Clean up the server and scheduler.
    CleanupStack::PopAndDestroy(2, scheduler);
    
    }

// End of file
