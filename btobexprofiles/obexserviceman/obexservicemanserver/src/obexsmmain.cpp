/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of obexservicemanager core.
*
*/



// INCLUDE FILES

#include <e32base.h>
#include "obexserviceman.h"
#include "debug.h"
#include "obexsm.h"


// ---------------------------------------------------------
// PanicClient
//
// RMessage2::Panic() also completes the message.
//
// ---------------------------------------------------------
void PanicClient(const RMessage2& aMessage,TInt aPanic )
    {
    FTRACE(FPrint(_L("[SRCS]\tserver\tPanicClient: Reason = %d"), aPanic));
    aMessage.Panic(KSrcsPanic,aPanic);
    }

// ---------------------------------------------------------
// PanicServer
//
// Panic our own thread
//
// ---------------------------------------------------------
void PanicServer(TInt aPanic)
    {
    FTRACE(FPrint(_L("[SRCS]\tserver\tPanicServer: Reason = %d"), aPanic));
    User::Panic(KSrcsPanic, aPanic);
    }



// ================= OTHER EXPORTED FUNCTIONS ==============


// ---------------------------------------------------------
// RunServerL
//
// Perform all server initialisation, in particular creation
// of the scheduler and server and then run the scheduler.
// This is called from Srcs.exe (RunServer -method).
//
// ---------------------------------------------------------
EXPORT_C void RunServerL()
    {
    FLOG(_L("[SRCS]\tclient\tSrcs RunServerL"));

    // create and install the active scheduler we need
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);
    
    // create the server (leave it on the cleanup stack)
    CleanupStack::PushL( CObexSM::NewL() );

	
	// naming the server thread after the server helps to debug panics
	// ignore error - we tried the best we could
		
	User::RenameThread(KSrcsName); 
		
	
	RProcess::Rendezvous(KErrNone);
	
   
    FLOG(_L("[SRCS]\tclient\tSrcs RunServerL: Starting active scheduler..."));
	CActiveScheduler::Start();

    FLOG(_L("[SRCS]\tclient\tSrcs RunServerL: Done"));
    //
    // Cleanup the server and scheduler
    CleanupStack::PopAndDestroy(2);
    }





TInt E32Main() //used in all case( WINS and TARGET)
    {
    //TODO uncomment UHEAP macros after orbit memory leaks are resolved till then it should be commented.
//    __UHEAP_MARK;

    CTrapCleanup* cleanup=CTrapCleanup::New();
    TInt retVal = KErrNoMemory;
    if ( cleanup )
        {
        TRAP( retVal,RunServerL() );
        delete cleanup;
        }

 //   __UHEAP_MARKEND;
    return retVal;
    }


// End of file
