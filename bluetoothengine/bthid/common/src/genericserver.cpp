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


#include "genericserver.h"

// ----------------------------------------------------------------------
#include "genericserversecuritypolicy.h"
CGenericServer::CGenericServer(TInt aPriority) : /*CServer2(aPriority),*/
    CPolicyServer(/*EPriorityStandard*/aPriority, KGenericServerPolicy,
            ESharableSessions)
    {
    }

TInt CGenericServer::GetStartupFromCommandLine(TStartupRequest& startup)
    {

    // Check the command line is the expected length
    if (User::CommandLineLength() == KStartupTextLength)
        {
        // Copy the data to the supplied structure
        TPtr ptr(reinterpret_cast<TText*> (&startup), KStartupTextLength);
        User::CommandLine(ptr);
        return KErrNone;
        }

    return KErrGeneral;
    }

void CGenericServer::SignalStartup(TStartupRequest* startup, TInt aReason)
    {
    RThread client;

    // Open a handle to the client's thread
    if (startup && KErrNone == client.Open(startup->iId))
        {
        // Signal the client
        client.RequestComplete(startup->iStatus, aReason);
        client.Close();
        }
    }
