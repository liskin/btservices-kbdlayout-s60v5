/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Server part of basic printing profile
*
*/



// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTSBPPObjectServer.h"
#include "BTSBPPObjectRequest.h"
#include "BTSUDebug.h"

// CONSTANTS
const TInt KBTSBPPObjectChannelStart = 15; // Start looking for free RFCOMM channel
const TInt KBTSBPPObjectChannelStop  = 30; // Last free RFCOMM channel

_LIT8( KBTSBPPReferencedObjectsUUID, "\x00\x00\x11\x20\x00\x00\x10\x00\x80\x00\x00\x80\x5F\x9B\x34\xFB" );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::CBTSBPPObjectServer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSBPPObjectServer::CBTSBPPObjectServer( 
    MBTSBPPObjectServerObserver* aObserver, CBTEngDiscovery* aBTEngDiscoveryPtr,
    const TBTDevAddr& aExpectedClient ) : iExpectedClientPtr( &aExpectedClient ),
                                          iBTEngDiscoveryPtr( aBTEngDiscoveryPtr ),                                          
                                          iObserverPtr( aObserver )
    {
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::ConstructL()"));

    __ASSERT_DEBUG( iObserverPtr != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
    __ASSERT_DEBUG( iBTEngDiscoveryPtr != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
    __ASSERT_DEBUG( iExpectedClientPtr != NULL, BTSUPanic( EBTSUPanicNullPointer ) );

    TObexBluetoothProtocolInfo info;
    info.iTransport = KBTSProtocol;

    iObjectChannel = KBTSBPPObjectChannelStart;
        for ( ; iObjectChannel < KBTSBPPObjectChannelStop; iObjectChannel++ )
            {
        // Register security settings
        //
        TBTServiceSecurity sec;
        sec.SetAuthentication( EFalse );
        sec.SetAuthorisation( EFalse );
        sec.SetEncryption( EFalse );

        info.iAddr.SetSecurity( sec );
        info.iAddr.SetPort( iObjectChannel );
        
        // Try to start Obex Server on selected channel
        //
        TRAPD( error, iServer = CObexServer::NewL ( info ) );
            
        if ( !error )
            {
            error = iServer->Start( this );
            
            if ( !error )
                {
                // Initialize server
                //
                iServer->SetCallBack( *this );
                User::LeaveIfError ( iServer->SetLocalWho( KBTSBPPReferencedObjectsUUID ) );                

                // Register SDP settings
                //
                User::LeaveIfError ( iBTEngDiscoveryPtr->RegisterSdpRecord( 
                    TUUID(KBTSdpBasicPrinting), iObjectChannel, iSDPHandle ));

                FTRACE(FPrint(_L("[BTSU]\t CBTSBPPObjectServer::ConstructL() services running on channel %d"), iObjectChannel ));
                break;
                }
            else
                {
                // Failed to start Obex server. Delete server and try again
                //
                delete iServer;
                iServer = NULL;
                }
            }
        }

    if ( !iServer )
        {
        FLOG(_L("[BTSU]\t CBTSBPPObjectServer::ConstructL() ERROR, server did not start."));
        User::Leave( KErrGeneral );
        }

    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSBPPObjectServer* CBTSBPPObjectServer::NewL( 
    MBTSBPPObjectServerObserver* aObserver, CBTEngDiscovery* aBTEngDiscoveryPtr,
    const TBTDevAddr& aExpectedClient )
    {
    CBTSBPPObjectServer* self = new( ELeave ) 
        CBTSBPPObjectServer( aObserver, aBTEngDiscoveryPtr, aExpectedClient );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

    
// Destructor
CBTSBPPObjectServer::~CBTSBPPObjectServer()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::~CBTSBPPObjectServer()"));

    if ( iServer )
        {
        iServer->Stop();
        delete iServer;
        iServer = NULL;
        }

    delete iObjectRequest;
    //delete iPasskeyRequest;

    if ( iBTEngDiscoveryPtr )
        {        
        iBTEngDiscoveryPtr->DeleteSdpRecord( iSDPHandle );        
        }

    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::~CBTSBPPObjectServer() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::SetReferencedObjectList
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::SetReferencedObjectList( const CDesCArray* aRefObjectList )
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::SetReferencedObjectList()"));

    __ASSERT_DEBUG( aRefObjectList != NULL, BTSUPanic( EBTSUPanicNullPointer ) );

    iObjectListPtr = aRefObjectList;

    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::SetReferencedObjectList() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::HasReferencedObjects
// -----------------------------------------------------------------------------
//
TBool CBTSBPPObjectServer::HasReferencedObjects()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::HasReferencedObjects()"));
    __ASSERT_DEBUG( iObjectListPtr != NULL, BTSUPanic( EBTSUPanicNullPointer ) );

    return iObjectListPtr->MdcaCount() > 0;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::GetUserPasswordL
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::GetUserPasswordL( const TDesC& /*aRealm*/ )
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::GetUserPasswordL() NOT NEEDED"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::ErrorIndication
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::ErrorIndication( TInt aError )
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTSBPPObjectServer::ErrorIndication() error %d"), aError ));

    iObserverPtr->ServerError( aError );
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::TransportUpIndication
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::TransportUpIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::TransportUpIndication()"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::TransportDownIndication
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::TransportDownIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::TransportDownIndication()"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::ObexConnectIndication
// -----------------------------------------------------------------------------
//
TInt CBTSBPPObjectServer::ObexConnectIndication( 
        const TObexConnectInfo& /*aRemoteInfo*/, const TDesC8& /*aInfo*/ )
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::ObexConnectIndication()"));

    TInt error = KErrAccessDenied;

    // Check that remote device is the expected one.
    //
    TBTSockAddr remoteAddr;
    iServer->RemoteAddr( remoteAddr );
    if ( remoteAddr.BTAddr() == *iExpectedClientPtr )
        {
        error = KErrNone;
        iObserverPtr->ServerConnectionEstablished();
        }

    FTRACE(FPrint(_L("[BTSU]\t CBTSBPPObjectServer::ObexConnectIndication() completed with %d"), error ));
    return error;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::ObexDisconnectIndication
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::ObexDisconnectIndication( const TDesC8& /*aInfo*/ )
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::ObexDisconnectIndication()"));

    iObserverPtr->ServerConnectionClosed();

    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::ObexDisconnectIndication() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::PutRequestIndication
// -----------------------------------------------------------------------------
//
CObexBufObject* CBTSBPPObjectServer::PutRequestIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::PutRequestIndication()"));

    return NULL;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::PutPacketIndication
// -----------------------------------------------------------------------------
//
TInt CBTSBPPObjectServer::PutPacketIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::PutPacketIndication()"));

    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::PutCompleteIndication
// -----------------------------------------------------------------------------
//
TInt CBTSBPPObjectServer::PutCompleteIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::PutCompleteIndication()"));

    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::GetRequestIndication
// -----------------------------------------------------------------------------
//
CObexBufObject* CBTSBPPObjectServer::GetRequestIndication(
        CObexBaseObject* aRequiredObject )
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::GetRequestIndication()"));
    __ASSERT_DEBUG( iObjectRequest == NULL, BTSUPanic( EBTSUPanicExistingObject ) );

    TInt error = KErrNone;
    CObexBufObject* object = NULL;

    TRAP( error, iObjectRequest = CBTSBPPObjectRequest::NewL( aRequiredObject, iObjectListPtr ) );

    if ( !error )
        {
        object = iObjectRequest->GetResponse();
        }

    FTRACE(FPrint(_L("[BTSU]\t CBTSBPPObjectServer::GetRequestIndication() completed with %d"), error ) );

    return object;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::GetPacketIndication
// -----------------------------------------------------------------------------
//
TInt CBTSBPPObjectServer::GetPacketIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::GetPacketIndication()"));

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::GetCompleteIndication
// -----------------------------------------------------------------------------
//
TInt CBTSBPPObjectServer::GetCompleteIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::GetCompleteIndication()"));

    delete iObjectRequest;
    iObjectRequest = NULL;

    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::GetCompleteIndication() completed"));

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::SetPathIndication
// -----------------------------------------------------------------------------
//
TInt CBTSBPPObjectServer::SetPathIndication( const CObex::TSetPathInfo& /*aPathInfo*/, 
                                             const TDesC8& /*aInfo*/ )
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::SetPathIndication()"));

    return KErrNotSupported;
    }

// -----------------------------------------------------------------------------
// CBTSBPPObjectServer::AbortIndication
// -----------------------------------------------------------------------------
//
void CBTSBPPObjectServer::AbortIndication()
    {
    FLOG(_L("[BTSU]\t CBTSBPPObjectServer::AbortIndication()"));

    iObserverPtr->ServerError( KErrCancel );
    }

//  End of File  
