/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Client-side implementation of BTEng
*
*/



#include <e32base.h>
#include <btmanclient.h>

#include "btengclient.h"
#include "debug.h"

/**  Number of retries for connecting to BTEng server side */
const TInt KBTEngConnectAttempts = 3;


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ?description
// ---------------------------------------------------------------------------
//
TInt ClientStart()
    {
    TRACE_FUNC_ENTRY
    const TUidType serverUid( KNullUid, KNullUid, KBTEngServerUid3 );
        // Create a new server process. Simultaneous launching of two processes 
        // should be detected when the second one attempts to create the server 
        // object, failing with KErrAlreadyExists.
    RProcess server;
    TInt err = server.Create( KBTEngServerName, KNullDesC, serverUid );
    if( err != KErrNone )
        {
        TRACE_INFO( ( _L( "[BTEng]\t Server process creation failed %d" ), err ) );
        return err;
        }
    TRequestStatus status;
    server.Rendezvous( status );
    if( status != KRequestPending )
        {
        TRACE_INFO( ( _L( "[BTEng]\t wrong status (%d), kill process"), 
                            status.Int() ) )
        server.Kill( KErrCancel );  // Abort startup
        }
    else
        {
        TRACE_INFO( ( _L( "[BTEng]\t Server process launched") ) )
        server.Resume();
        }
    User::WaitForRequest( status );     // Wait for start or death
    err = status.Int();
    if( server.ExitType() == EExitPanic )
        {
        // The server actually panicked; inform the client.
        err = KErrDied;
        }
    server.Close();
    TRACE_FUNC_RES( ( _L( "result %d" ), err ) )
    return err;
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
RBTEng::RBTEng()
:   RSessionBase()
    {
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::Connect()
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    for( TInt i = 0; i < KBTEngConnectAttempts; i++ )
        {
        err = CreateSession( KBTEngServerName, Version() );
        if( err == KErrNone )
            {
            break;  // Success
            }
        else if( err == KErrNotFound || err == KErrServerTerminated )
            {
                // Session could not be created, 
                // first the server needs to be started.
            err = ClientStart();
            }
        if( err != KErrNone && err != KErrAlreadyExists )
            {
                // If the server cannot be started for other reasons than 
                // KErrAlreadyExists, give up...
            break;
            }
        }
    TRACE_FUNC_RES( ( _L("result %d"), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TVersion RBTEng::Version()
    {
    return TVersion( KBTEngServerVersionMajor, KBTEngServerVersionMinor, 
                      KBTEngServerVersionBuild );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::SetPowerState( const TBTPowerStateValue aState, const TBool aTemp )
    {
    return SendReceive( EBTEngSetPowerState, TIpcArgs( aState, aTemp ) );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::SetVisibilityMode( const TBTVisibilityMode aMode, const TInt aTime )
    {
    return SendReceive( EBTEngSetVisibilityMode, TIpcArgs( aMode, aTime ) );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::ConnectDevice( const TBTDevAddr& aAddr, 
    const TBTDeviceClass& aDeviceClass  )
    {
    TRACE_BDADDR(aAddr);
    TBTDevAddrPckgBuf addrPkg( aAddr );
    TBTEngDevClassPkg codPkg( aDeviceClass.DeviceClass() );
    return SendReceive( EBTEngConnectDevice, TIpcArgs( &addrPkg, &codPkg ) );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::CancelConnectDevice( const TBTDevAddr& aAddr )
    {
    TRACE_BDADDR(aAddr);
    TBTDevAddrPckgBuf addrPkg( aAddr );
    return SendReceive( EBTEngCancelConnectDevice, TIpcArgs( &addrPkg ) );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::DisconnectDevice( const TBTDevAddr& aAddr, 
    TBTDisconnectType aDiscType )
    {
    TRACE_BDADDR(aAddr);
    TBTDevAddrPckgBuf addrPkg( aAddr );
    TBTEngParamPkg discPkg( aDiscType );
    return SendReceive( EBTEngDisconnectDevice, TIpcArgs( &addrPkg, &discPkg ) );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::IsDeviceConnected( const TBTDevAddr& aAddr, 
    TBTEngConnectionStatus& aConnected )
    {
    TRACE_BDADDR(aAddr);
    TBTDevAddrPckgBuf addrPkg( aAddr );
    TBTEngParamPkg connPkg( aConnected );
    TInt err = SendReceive( EBTEngIsDeviceConnected, TIpcArgs( &addrPkg, &connPkg ) );
    aConnected = (TBTEngConnectionStatus) connPkg();
    return err;
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::IsDeviceConnectable( const TBTDevAddr& aAddr,
    const TBTDeviceClass& aDeviceClass, TBool& aConnectable )
    {
    TRACE_BDADDR(aAddr);
    TBTDevAddrPckgBuf addrPkg( aAddr );
    TBTEngDevClassPkg codPkg( aDeviceClass.DeviceClass() );
    TBTEngParamPkg connPkg( aConnectable );
    TInt err = SendReceive( EBTEngIsDeviceConnectable, TIpcArgs( &addrPkg, &codPkg, &connPkg ) );
    aConnectable = connPkg();
    return err;
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::GetConnectedAddresses( TDes8& aArrayPkg, TBTEngParamPkg& aProfilePkg )
    {
    return SendReceive( EBTEngGetConnectedAddresses, 
                         TIpcArgs( &aArrayPkg, &aProfilePkg ) );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::NotifyConnectionEvents( TDes8& aEvent, TRequestStatus& aStatus )
    {
        // Use the third slot for the event package.
    SendReceive( EBTEngNotifyConnectionEvents, 
                  TIpcArgs( NULL, NULL, &aEvent ), aStatus );
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::CancelNotifyConnectionEvents()
    {
    return SendReceive( EBTEngCancelEventNotifier, TIpcArgs() );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt RBTEng::PrepareDiscovery()
    {
    return SendReceive( EBTEngPrepareDiscovery, TIpcArgs() );
    }
