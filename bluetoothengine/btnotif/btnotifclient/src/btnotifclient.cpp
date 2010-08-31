/*
* ============================================================================
*  Name        : btnotifclient.cpp
*  Part of     : bluetoothengine / btnotifclient
*  Description : Session class for client-server interaction with btnotifserver.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.1
*/

#include "btnotifclient.h"
#include "btnotifclientserver.h"


// ---------------------------------------------------------------------------
// start btnotif server from client.
// ---------------------------------------------------------------------------
//
TInt StartBTNotifSrv()
    {
    const TUidType serverUid( KNullUid, KNullUid, KBTNotifServerUid3 );
        // Create a new server process. Simultaneous launching of two processes 
        // should be detected when the second one attempts to create the server 
        // object, failing with KErrAlreadyExists.
    RProcess server;
    TInt err = server.Create( KBTNotifServerName, KNullDesC, serverUid );
    if( err != KErrNone )
        {
        return err;
        }
    TRequestStatus status;
    server.Rendezvous( status );
    if( status != KRequestPending )
        {
        server.Kill( KErrCancel );  // Abort startup
        }
    else
        {
        server.Resume();
        }
    User::WaitForRequest( status ); // Wait for start or death
    err = status.Int();
    if( server.ExitType() == EExitPanic )
        {
        // The server actually panicked; inform the client.
        err = KErrDied;
        }
    server.Close();
    return err;
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
EXPORT_C RBTNotifier::RBTNotifier()
:   RSessionBase()
    {
    }

// ---------------------------------------------------------------------------
// Return the client-server version number we implement.
// ---------------------------------------------------------------------------
//
EXPORT_C TVersion RBTNotifier::Version()
    {
    return TVersion( KBTNotifServerVersionMajor, KBTNotifServerVersionMinor, 
                KBTNotifServerVersionBuild );
    }


// ---------------------------------------------------------------------------
// Connect to the notifier server. Start the server if necessary.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt RBTNotifier::Connect()
    {
    TInt err = CreateSession( KBTNotifServerName, Version() );
    if( err == KErrNotFound || err == KErrServerTerminated )
        {
        err = StartBTNotifSrv();
        if( err == KErrNone || err == KErrAlreadyExists )
            {
            err = CreateSession( KBTNotifServerName, Version() );
            }
        }
    return err;
    }


// ---------------------------------------------------------------------------
// Start the specified notifier, synchronous call.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt RBTNotifier::StartNotifier( TUid aNotifierUid, const TDesC8& aBuffer )
    {
    return SendReceive( EBTNotifStartSyncNotifier,
                TIpcArgs( (TInt) aNotifierUid.iUid, &aBuffer ) );
    }


// ---------------------------------------------------------------------------
// Start the specified notifier, asynchronous call.
// ---------------------------------------------------------------------------
//
EXPORT_C void RBTNotifier::StartNotifierAndGetResponse( TRequestStatus& aRs,
    TUid aNotifierUid, const TDesC8& aBuffer, TDes8& aResponse )
    {
    SendReceive( EBTNotifStartAsyncNotifier,
                TIpcArgs( (TInt) aNotifierUid.iUid, &aBuffer, &aResponse ), aRs );
    }


// ---------------------------------------------------------------------------
// Cancel the specified notifier.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt RBTNotifier::CancelNotifier( TUid aNotifierUid )
    {
    return SendReceive( EBTNotifCancelNotifier, TIpcArgs( (TInt) aNotifierUid.iUid ) );
    }


// ---------------------------------------------------------------------------
// Update the specified notifier, synchronous call.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt RBTNotifier::UpdateNotifier( TUid aNotifierUid, 
    const TDesC8& aBuffer, TDes8& aResponse )
    {
    return SendReceive( EBTNotifUpdateNotifier,
                TIpcArgs( (TInt) aNotifierUid.iUid, &aBuffer, &aResponse ) );
    }

// ---------------------------------------------------------------------------
// Issue a pairing request. asynchronous call.
// ---------------------------------------------------------------------------
//
EXPORT_C void RBTNotifier::PairDevice( const TBTDevAddrPckgBuf& aDevice, 
    TInt32 aDeviceClass, TRequestStatus& aStatus )
    {
    SendReceive( EBTNotifPairDevice,
                TIpcArgs( (TInt) EBTNotifPairDevice, &aDevice, aDeviceClass ), aStatus );
    }

// ---------------------------------------------------------------------------
// Cancel an ongoing pair request.
// ---------------------------------------------------------------------------
//
EXPORT_C void RBTNotifier::CancelPairDevice()
    {
    (void) SendReceive( EBTNotifCancelPairDevice );
    }
