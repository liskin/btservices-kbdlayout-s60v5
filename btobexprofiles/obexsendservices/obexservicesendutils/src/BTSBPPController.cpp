/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Basic printing profile implementation
*
*/



// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTSBPPController.h"
#include "BTSUDebug.h"

#include <obexheaders.h>

// CONSTANTS
_LIT8( KBTSDirectPrintingUUID, "\x00\x00\x11\x18\x00\x00\x10\x00\x80\x00\x00\x80\x5F\x9B\x34\xFB" );
_LIT8( KBTSXHTMLPrintType,     "application/vnd.pwg-xhtml-print+xml\0" );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBPPController::CBTSBPPController
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSBPPController::CBTSBPPController( MBTServiceObserver* aObserver,
                                      const CBTServiceParameterList* aList ) : 
                                      iServerState( EBTSBPPSrvIdle ), 
                                      iClientDone( EFalse ), 
                                      iObserverPtr( aObserver ), 
                                      iListPtr( aList )
    {
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSBPPController::ConstructL( const TBTDevAddr& aRemoteDevice,
                                    const TUint aRemotePort,
                                    CBTEngDiscovery* aBTConnectionPtr )
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::ConstructL()"));

    // Create an array of obex headers
    //
    RArray<CObexHeader*> headers;
    CleanupClosePushL( headers );

    CObexHeader* targetHdr = CObexHeader::NewL();
    CleanupStack::PushL( targetHdr );
    targetHdr->SetByteSeqL( KBTSUTargetHeader, KBTSDirectPrintingUUID );
    headers.Append( targetHdr );

    // Create obex client
    //    
    CreateClientL ( this, aRemoteDevice, aRemotePort, headers );        

    CleanupStack::Pop( 2 ); // targetHdr, headers
    headers.Close();

    // Start the object server if there were referenced objects
    //
    if ( iListPtr->HasAnyReferencedObjects() )
        {
        iServer = CBTSBPPObjectServer::NewL( this,
                                             aBTConnectionPtr,
                                             aRemoteDevice );
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSBPPController* CBTSBPPController::NewL( MBTServiceObserver* aObserver,                                                                              
                                            const TUint aRemotePort,
                                            const TBTDevAddr& aRemoteDevice,
                                            const CBTServiceParameterList* aList,
                                            CBTEngDiscovery* aBTConnectionPtr )
    {
    CBTSBPPController* self = new( ELeave ) CBTSBPPController( aObserver, aList );
    CleanupStack::PushL( self );
    self->ConstructL( aRemoteDevice, aRemotePort, aBTConnectionPtr );
    CleanupStack::Pop(self);
    return self;
    }

    
// Destructor
CBTSBPPController::~CBTSBPPController()
    {
    if ( iServer )
        {
        delete iServer;
        iServer = NULL;
        }
    if ( iServerWait )
        {            
        delete iServerWait;
        iServerWait = NULL;
        }
    }


// -----------------------------------------------------------------------------
// CBTSBPPController::ConnectCompleted
// -----------------------------------------------------------------------------
//
void CBTSBPPController::ConnectCompleted( TInt aStatus )
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::ConnectCompleted() %d"), aStatus ));

    if ( aStatus )
        {
        // The connect operation failed.
        //
        iObserverPtr->ControllerComplete( EBTSConnectingFailed );
        }
    else
        {
        TRAPD( error, SelectAndSendL() );

        if ( error )
            {
            FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::SelectAndSendL() leaved with %d"), error ));
            iObserverPtr->ControllerComplete( EBTSPuttingFailed );
            }
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::ConnectCompleted() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::PutCompleted
// -----------------------------------------------------------------------------
//
void CBTSBPPController::PutCompleted( TInt aStatus, const CObexHeaderSet* /*aPutResponse*/ )
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::PutCompleted()"));

	if ( aStatus )
        {
        // The put operation failed.
        //
        iObserverPtr->ControllerComplete( aStatus );
        }
    else
        {
        iClientDone = ETrue;

        if ( iServer == NULL                  || // Server was not needed at all or
             !iServer->HasReferencedObjects() || // no referenced objects in current file or
             iServerState == EBTSBPPSrvDone )    // server is done.
            {
            // Object sent and server done, check if there are 
            // more files to send.
            //
            TRAPD( error, SelectAndSendL() );

            if ( error )
                {
                FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::SelectAndSendL() leaved with %d"), error ));
                iObserverPtr->ControllerComplete( EBTSPuttingFailed );
                }
            }
        else if ( iServerState == EBTSBPPSrvIdle )
            {
            __ASSERT_DEBUG( iServerWait == NULL, BTSUPanic( EBTSUPanicExistingObject ) );

            // Object sent but server isn't serving yet. 
            // Wait for a connection attempt.
            //

            TRAPD(error, iServerWait = CBTSBPPServerWait::NewL( this ));
			
			if ( error )
                {
                FTRACE(FPrint(_L("[BTSU]\t CBTSBPPServerWait::NewL() leaved with %d"), error ));
                iObserverPtr->ControllerComplete( EBTSPuttingFailed );
                }

            }
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::PutCompleted() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::GetCompleted
// -----------------------------------------------------------------------------
//
void CBTSBPPController::GetCompleted( TInt /*aStatus*/, CObexBufObject* /*aGetResponse*/ )
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::GetCompleted() ERROR: unsolicited callback"));
    __ASSERT_DEBUG( EFalse, BTSUPanic( EBTSUPanicInternalError ) );
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::ClientConnectionClosed
// -----------------------------------------------------------------------------
//
void CBTSBPPController::ClientConnectionClosed()
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::ClientConnectionClosed()"));

    // Everything is now ready.
    //
    iObserverPtr->ControllerComplete( EBTSNoError );

    FLOG(_L("[BTSU]\t CBTSBPPController::ClientConnectionClosed() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::ServerError
// -----------------------------------------------------------------------------
//
void CBTSBPPController::ServerError( TInt aError )
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::ServerError() %d"), aError ) );

    // Mark server as ready and if client is ready, complete the controller.
    //
    iServerState = EBTSBPPSrvDone;

    if ( iClientDone )
        {
        iObserverPtr->ControllerComplete( aError );
        }
    else
        {
        // Server ready, no need to wait anymore
        //
        if ( iServerWait )
            {            
            delete iServerWait;
            iServerWait = NULL;
            }
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::ServerError() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::ServerConnectionEstablished
// -----------------------------------------------------------------------------
//
void CBTSBPPController::ServerConnectionEstablished()
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::ServerConnectionEstablished()"));

    iServerState = EBTSBPPSrvServing;

    if ( iServerWait )
        {
        // The wait for server connection can now be stopped.
        //
        delete iServerWait;
        iServerWait = NULL;
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::ServerConnectionEstablished() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::ServerConnectionClosed
// -----------------------------------------------------------------------------
//
void CBTSBPPController::ServerConnectionClosed()
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::ServerConnectionClosed()"));

    iServerState = EBTSBPPSrvDone;

    if ( iClientDone )
        {
        // The client was waiting for server, but now another object 
        // can be sent.
        //
        TRAPD( error, SelectAndSendL() );

        if ( error )
            {
            iObserverPtr->ControllerComplete( error );
            }
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::ServerConnectionClosed() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::WaitComplete
// -----------------------------------------------------------------------------
//
void CBTSBPPController::WaitComplete()
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::WaitComplete()"));

    // For some reason the printer didn't establish server connection although
    // there were referenced objects
    //
    if ( iServerWait )
        {   
        delete iServerWait;
        iServerWait = NULL;
        }

    // The client was waiting for server, but now another object 
    // can be sent.
    //
    TRAPD( error, SelectAndSendL() );

    if ( error )
        {
        FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::SelectAndSendL() leaved with %d"), error ));
        iObserverPtr->ControllerComplete( EBTSPuttingFailed );
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::WaitComplete() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPController::SelectAndSendL
// -----------------------------------------------------------------------------
//
void CBTSBPPController::SelectAndSendL()
    {
    FLOG(_L("[BTSU]\t CBTSBPPController::SelectAndSendL()"));

    TBTSUXhtmlParam param;
    param.iFileName = NULL;
    param.iRefObjectList = NULL;
   
    if ( iListPtr->XhtmlCount() > 0 && iSendIndex < iListPtr->XhtmlCount() )
        {
        // Get next xhtml parameter
        //
        param = iListPtr->XhtmlAt( iSendIndex );
        iSendIndex++;
        }

    if ( param.iFileName != NULL )
        {
        FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::SelectAndSendL() fileName='%S'"), param.iFileName ));

        if ( iServer )
            {
            // Pass the list of referenced objects in the file to object server.
            //
            iServer->SetReferencedObjectList( param.iRefObjectList );
            }

        // Create an array of obex headers
        //
        RArray<CObexHeader*> headers;
        CleanupClosePushL( headers );

        CObexHeader* typeHdr = CObexHeader::NewL();
        CleanupStack::PushL( typeHdr );
        typeHdr->SetByteSeqL( KBTSUTypeHeader, KBTSXHTMLPrintType );
        headers.Append( typeHdr );

        // Send object.
        //
        iClient->PutObjectL( headers, *param.iFileName );
        
        CleanupStack::Pop( 2 ); // typeHdr, headers
        headers.Close();

        // A new object is being sent. Reset states.
        //
        iClientDone = EFalse;
        iServerState = EBTSBPPSrvIdle;
        }
    else
        {
        FLOG(_L("[BTSU]\t CBTSBPPController::SelectAndSendL() all objects sent, closing connection"));

        // All objects sent, close client connection.
        //
        iClient->CloseClientConnection();
        }

    FLOG(_L("[BTSU]\t CBTSBPPController::SelectAndSendL() completed"));
    }

//-----------------------------------------------------------------------------
// void CBTSBPPController::ResetFileHandleL
// -----------------------------------------------------------------------------
//    
void CBTSBPPController::ResetFileHandleL()
    {    
    }

//-----------------------------------------------------------------------------
// void CBTSBPPController::ConnectTimedOut()
// -----------------------------------------------------------------------------
//        
void CBTSBPPController::ConnectTimedOut()    
    {
    iObserverPtr->ConnectTimedOut();
    }
//  End of File  
