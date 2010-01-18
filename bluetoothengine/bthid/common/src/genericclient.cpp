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


#include <e32math.h>

#include "genericclient.h"
#include "genericserver.h"

// ----------------------------------------------------------------------

TInt RGenericSession::StartSession(const TDesC& aServer,
        const TVersion& aVersion, const TDesC& aFilename,
        const TUidType& aUid, TInt aAsyncMessageSlots)
    {
    // The server may be running already, so try connecting immediately
    TInt error = CreateSession(aServer, aVersion, aAsyncMessageSlots);

    if (KErrNone != error)
        {
        // Failed to connect so create a new server instance and try again.
        // If creating the server fails, it may be due to another client
        // creating a server in the meantime, so ignore any error here.
        DBG(RDebug::Print(_L("RGenericSession::StartSession(): calling CreateServer()...")));
        CreateServer(aServer, aFilename, aUid);
        DBG(RDebug::Print(_L("RGenericSession::StartSession(): calling CreateSession()...")));
        error = CreateSession(aServer, aVersion, aAsyncMessageSlots);
        }

    DBG(RDebug::Print(_L("RGenericSession::StartSession(): returning %d"),error));

    return error;
    }

TInt RGenericSession::CreateServer(const TDesC& /*aServer*/,
        const TDesC& aFilename, const TUidType& aUid)
    {
    TInt result;
    TRequestStatus startup = KRequestPending;
    TStartupRequest startupRequest(&startup, RThread().Id());

    RProcess server;
    TPtrC startupPtr(reinterpret_cast<TText*> (&startupRequest),
            KStartupTextLength);

    DBG(RDebug::Print(_L("RGenericSession::CreateServer(): calling server.Create()")));
    result = server.Create(aFilename, startupPtr, aUid);

    DBG(RDebug::Print(_L("RGenericSession::CreateServer(): server.Create() result: %d"),result));

    if (KErrNone == result)
        {
        TRequestStatus stat;
        server.Rendezvous(stat);
        if (stat != KRequestPending)
            {
            DBG(RDebug::Print(_L("RGenericSession::CreateServer(): __SECURE_API__ ClientStart, stat != KRequestPending, Kill process ")));
            server.Kill(0); // abort startup
            }
        else
            {
            DBG(RDebug::Print(_L("RGenericSession::CreateServer(): __SECURE_API__ ClientStart, logon OK - start the server ")));
            server.Resume(); // logon OK - start the server
            }
        User::WaitForRequest(stat); // wait for start or death
        // we can't use the 'exit reason' if the server panicked as this
        // is the panic 'reason' and may be '0' which cannot be distinguished
        // from KErrNone
        result = (server.ExitType() == EExitPanic) ? KErrGeneral : stat.Int();
        DBG(RDebug::Print(_L("RGenericSession::CreateServer(): __SECURE_API__ ClientStart, server exit result: %d"),result));

        server.Close();
        }

    return result;
    }
