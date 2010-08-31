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
* Description:  Obex client implementation
*
*/



// INCLUDE FILES
#include <apgcli.h> // RApaLSession

#include "BTServiceClient.h"
#include "BTServiceUtils.h"
#include "BTConnectionTimer.h"
#include "BTSUDebug.h"

const TUint16 KMtuSizeReceiv    = 0xFFFF;	// 64kB - 1 (65535)
const TUint16 KMtuSizeTrans     = 0x8000;	// 32kB
const TInt    KBufferSize       = 0x8000;	// 32kB

const TInt KBTConnectionTimeout = 20 * 1000 * 1000;	// 20 seconds
const TInt KBTAbortTimeout      = 2 * 1000 * 1000;	// 2 seconds

// CONSTANTS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTServiceClient::CBTServiceClient
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTServiceClient::CBTServiceClient( MBTServiceClientObserver* aObserver ) 
    : CActive( EPriorityStandard ), 
      iClientState( EBTSCliIdle ), 
      iObserver( aObserver )                           
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTServiceClient::ConstructL( const TBTDevAddr& aRemoteDevice,
                                   const TUint aRemotePort,
                                   RArray<CObexHeader*> aHeaderList )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::ConstructL()"));

    iTotalBytesSent = 0;
    // Create Obex Client
    //
    TObexBluetoothProtocolInfo info;
    info.iTransport = KBTSProtocol;
    info.iAddr.SetBTAddr( aRemoteDevice );
    info.iAddr.SetPort( aRemotePort );
    
    TObexProtocolPolicy obexProtocolPolicy;
    obexProtocolPolicy.SetReceiveMtu( KMtuSizeReceiv  );
    obexProtocolPolicy.SetTransmitMtu( KMtuSizeTrans  );
	
    iClient = CObexClient::NewL( info, obexProtocolPolicy );
    iClient->SetCallBack( *this );
    iPasskeyRequest = new (ELeave) CBTSUPasskeyRequest();

    // Create Connect-object
    //
    iConnectObject = CObexNullObject::NewL();

    if ( aHeaderList.Count() > 0 )
        {
        for ( TInt index = 0; index < aHeaderList.Count(); index++ )
            {
            iConnectObject->AddHeaderL( *aHeaderList[index] );
            }
        }

    // Establish client connection
    //
    iClient->Connect( *iConnectObject, iStatus );
    SetActive();
    iClientState = EBTSCliConnecting;
    iConnectionTimer = CBTConnectionTimer::NewL(this);
    iConnectionTimer -> SetTimeOut ( TTimeIntervalMicroSeconds32( KBTConnectionTimeout ) );    
    iConnectionTimer -> Start();

    FLOG(_L("[BTSU]\t CBTServiceClient::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTServiceClient* CBTServiceClient::NewL( MBTServiceClientObserver* aObserver,
                                          const TBTDevAddr& aRemoteDevice,
                                          const TUint aRemotePort,
                                          RArray<CObexHeader*> aHeaderList )
    {
    CBTServiceClient* self = new( ELeave ) CBTServiceClient( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL( aRemoteDevice, aRemotePort, aHeaderList );
    CleanupStack::Pop();
    return self;
    }

    
// Destructor
CBTServiceClient::~CBTServiceClient()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::~CBTServiceClient()"));
    
    if(iConnectionTimer)
        {
        iConnectionTimer->Cancel();
        delete iConnectionTimer;
        iConnectionTimer=NULL;
        }    
    Cancel();
    if(iClient)
        {
        delete iClient;
        iClient = NULL;
        }    
    delete iPasskeyRequest;
    iPasskeyRequest = NULL;

    if ( iConnectObject )
        {
        iConnectObject->Reset();
        delete iConnectObject;
        iConnectObject = NULL;
        }
    if ( iObjectBuffer )
        {
        iObjectBuffer->Reset();
        delete iObjectBuffer;
        iObjectBuffer = NULL;
        }
    if ( iGetObject )
        {
        delete iGetObject;
        iGetObject = NULL;
        }
    if ( iPutObject )
        {
        iPutObject->Reset();
        delete iPutObject;
        iPutObject = NULL;
        }
     if ( iPutBufObject )
        {
        iPutBufObject->Reset();
        delete iPutBufObject;
        iPutBufObject = NULL;
        }
           
    if(iBuffer)
        {
        delete iBuffer;
        iBuffer = NULL;
        }

    FLOG(_L("[BTSU]\t CBTServiceClient::~CBTServiceClient() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::GetObjectL
// -----------------------------------------------------------------------------
//
void CBTServiceClient::GetObjectL( RArray<CObexHeader*>& aHeaderList,
                                   const TDesC& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::GetObjectL()"));

    if ( iGetObject )
        {
        iGetObject->Reset();
        delete iGetObject;
        iGetObject = NULL;
        }

    // Create object
    //
    if ( aFileName == KNullDesC )
        {
        iObjectBuffer = CBufFlat::NewL( KBTSUDataBufferExpandSize );
        iGetObject = CObexBufObject::NewL( iObjectBuffer );
        }
    else
        {        
        iGetObject = CObexBufObject::NewL( NULL );
        iGetObject->SetDataBufL( aFileName );
        }

    // Set headers
    //
    if ( aHeaderList.Count() > 0 )
        {
        for ( TInt index = 0; index < aHeaderList.Count(); index++ )
            {
            iGetObject->AddHeaderL( *aHeaderList[index] );
            }
        }

    // Send get request
    //
    iClient->Get( *iGetObject, iStatus );
    SetActive();
    iClientState = EBTSCliGetting;

    FLOG(_L("[BTSU]\t CBTServiceClient::GetObjectL() completed"));
    }


// -----------------------------------------------------------------------------
// CBTServiceClient::PutObjectL
// -----------------------------------------------------------------------------
//
void CBTServiceClient::PutObjectL( RArray<CObexHeader*>& aHeaderList,
                                   const TDesC& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::PutObjectL()"));

    if ( iPutObject )
        {
        iPutObject->Reset();
        delete iPutObject;
        iPutObject = NULL;
        }

    // Create object
    //
    iPutObject = CObexFileObject::NewL();

    // Set body
    //
    
    if ( aFileName != KNullDesC )
        {
        iPutObject->InitFromFileL ( aFileName );
        }
    
    // Set headers
    //
    if ( aHeaderList.Count() > 0 )
        {
        for ( TInt index = 0; index < aHeaderList.Count(); index++ )
            {
            iPutObject->AddHeaderL( *aHeaderList[index] );
            }
        }

    // Send object
    //
    iClient->Put( *iPutObject, iStatus );
	SetActive();
    iClientState = EBTSCliPutting;

    FLOG(_L("[BTSU]\t CBTServiceClient::PutObjectL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::PutObjectL
// -----------------------------------------------------------------------------
//
void CBTServiceClient::PutObjectL( RArray<CObexHeader*>& aHeaderList,
                                   RFile& aFile )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::PutObjectL()"));

    if ( iPutObject )
        {
        iPutObject->Reset();
        delete iPutObject;
        iPutObject = NULL;
        }

    if ( iPutBufObject )
        {
        iPutBufObject->Reset();
        delete iPutBufObject;
        iPutBufObject = NULL;
        }
    // Create object
    //
    iPutBufObject = CObexBufObject::NewL(NULL);

    // Resolve MIME type
    //
    
    RApaLsSession session;
    HBufC8* mimeType = NULL;
    TDataType type;
    
    TUid uid;
    // Set headers
    //
    if ( aHeaderList.Count() > 0 )
        {
        for ( TInt index = 0; index < aHeaderList.Count(); index++ )
            {
            iPutBufObject->AddHeaderL( *aHeaderList[index] );
            }
        }
        
    TDesC8 typeheader=iPutBufObject->Type();   
    if ( typeheader == KNullDesC8 )    
        {        
        User::LeaveIfError( session.Connect() );
        CleanupClosePushL( session );
        TInt error = session.AppForDocument( aFile, uid, type );    
        if ( error == KErrNone )
            {
            mimeType = type.Des8().AllocLC();
            iPutBufObject->SetTypeL(*mimeType);    
            CleanupStack::PopAndDestroy();
            }          
        CleanupStack::Pop(); // session
        session.Close();       
        }    
    
    //Set object information
    //
    TFileName filename;    
    aFile.Name(filename);
    
    TInt size;
    aFile.Size(size);
    iPutBufObject->SetLengthL(size);			
    iPutBufObject->SetNameL(filename);
    
    TTime time;
	if ( aFile.Modified(time) == KErrNone )
	    {
		iPutBufObject->SetTimeL(time);
	    }
	    
    RFile file;    
    file.Duplicate(aFile);
    
    iBuffer = CBufFlat::NewL(KBufferSize);
    iBuffer ->ResizeL(KBufferSize);
    
    TObexRFileBackedBuffer bufferdetails(*iBuffer,file,CObexBufObject::ESingleBuffering);  
    iPutBufObject->SetDataBufL(bufferdetails);         
    
    // Send object
    //        
    iClient->Put( *iPutBufObject, iStatus );
	SetActive();
    iClientState = EBTSCliPutting;

    FLOG(_L("[BTSU]\t CBTServiceClient::PutObjectL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::CloseClientConnection
// -----------------------------------------------------------------------------
//
void CBTServiceClient::CloseClientConnection()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::CloseClientConnection()"));

    iClient->Disconnect( iStatus );
   	SetActive();
    iClientState = EBTSCliDisconnecting;

    FLOG(_L("[BTSU]\t CBTServiceClient::CloseClientConnection() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::GetProgressStatus
// -----------------------------------------------------------------------------
//
TInt CBTServiceClient::GetProgressStatus()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::GetProgressStatus()"));

    TInt bytesSent = 0;
    if ( iPutBufObject )
        {
        bytesSent = iPutBufObject->BytesSent();
        }
    if ( iPutObject )
        {
        bytesSent = iPutObject->BytesSent();
        }

    FTRACE(FPrint(_L("[BTSU]\t CBTServiceClient::GetProgressStatus() completed, bytes sent %d"), iTotalBytesSent + bytesSent ) );

   // return iTotalBytesSent + bytesSent;
    return bytesSent;
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::GetUserPasswordL
// -----------------------------------------------------------------------------
//
void CBTServiceClient::GetUserPasswordL( const TDesC& /*aRealm*/ )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::GetUserPasswordL()"));

    iPasskeyRequest->StartPassKeyRequestL( iClient );

    FLOG(_L("[BTSU]\t CBTServiceClient::GetUserPasswordL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::DoCancel
// -----------------------------------------------------------------------------
//
void CBTServiceClient::DoCancel()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::DoCancel()"));
    
    if ( iConnectionTimer )
        {
        iConnectionTimer->Cancel();
        delete iConnectionTimer;
        iConnectionTimer=NULL;
        }    
    // Deleting obexclient is the only way to cancel active requests
    //                
    if ( iClient )
        {
        delete iClient;
        iClient = NULL;
        }    
    
    FLOG(_L("[BTSU]\t CBTServiceClient::DoCancel() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::RunL
// -----------------------------------------------------------------------------
//
void CBTServiceClient::RunL()
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTServiceClient::RunL() status %d"), iStatus.Int() ) );
    if ( !iObserver )
        {
        return;    
        }
    switch ( iClientState )
        {
        case EBTSCliConnecting:
            {
            FLOG(_L("[BTSU]\t CBTServiceClient::RunL() EBTSCliConnecting"));
            iConnectObject->Reset();
            if(iConnectionTimer)
                {
                iConnectionTimer->Cancel();
                delete iConnectionTimer;
                iConnectionTimer=NULL;
                }    
            iObserver->ConnectCompleted( iStatus.Int() );
            break;
            }

        case EBTSCliPutting:
            {
            FLOG(_L("[BTSU]\t CBTServiceClient::RunL() EBTSCliPutting"));
            if(iPutBufObject)
                {
                iTotalBytesSent += iPutBufObject->BytesSent();    
                }
            else
                {
                 iTotalBytesSent += iPutObject->BytesSent();         
                }
			   
			const CObexHeaderSet& response=iClient->GetPutFinalResponseHeaders();		
			if ( iPutBufObject )
				{
				iPutBufObject->Reset();
				delete iPutBufObject;
				iPutBufObject = NULL;
				}
				
			if ( iPutObject )
				{
				iPutObject->Reset();
				delete iPutObject;
				iPutObject = NULL;
				}
			if(iBuffer)
                {
                delete iBuffer;
                iBuffer = NULL;
                }
			//put there call getEnv
            iObserver->PutCompleted( iStatus.Int(), &response);
            FLOG(_L("[BTSU]\t CBTServiceClient::RunL() EBTSCliPutting done"));
            break;
            }

        case EBTSCliGetting:
            {
            iObserver->GetCompleted( iStatus.Int(), iGetObject );
            break;
            }

        case EBTSCliDisconnecting:
            {
            // Any errors are ignored
            //
            iObserver->ClientConnectionClosed();
            break;
            }

        case EBTSCliIdle:
        default:
           {
           FLOG(_L("[BTSU]\t CBTServiceClient::RunL() ERROR, unhandled case"));           
           break;
           }
        }

    FLOG(_L("[BTSU]\t CBTServiceClient::RunL() completed"));
    }
    
// -----------------------------------------------------------------------------
// CBTServiceClient::ConnectionTimedOut
// -----------------------------------------------------------------------------
//	
void CBTServiceClient::ConnectionTimedOut()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::ConnectionTimedOut"));    
    switch ( iClientState )
        {
        case EBTSCliConnecting:
            {
            iObserver->ConnectTimedOut();      
            break;
            }
        case EBTSCliGetting:
            {
            iObserver->GetCompleted( KErrAbort, iGetObject );    
            break;
            }    
        case EBTSCliPutting:
            {
            const CObexHeaderSet& response=iClient->GetPutFinalResponseHeaders();	    
            iObserver->PutCompleted( KErrAbort, &response );    
			break;
            }        
        default:    
        FLOG(_L("[BTSU]\t CBTServiceClient::ConnectionTimedOut unhandled client state "));
        }
          
    FLOG(_L("[BTSU]\t CBTServiceClient::ConnectionTimedOut"));    
    }
// -----------------------------------------------------------------------------
// CBTServiceClient::Abort
// -----------------------------------------------------------------------------
//	    
void CBTServiceClient::Abort()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::Abort"));        
    if ( iClient && ( iClientState == EBTSCliPutting || iClientState == EBTSCliGetting )  )
        {    
        if ( iConnectionTimer )    
            {
            delete iConnectionTimer;
            iConnectionTimer = NULL;    
            }
        TRAPD(trapErr, iConnectionTimer = CBTConnectionTimer::NewL(this) );
        if ( trapErr  != KErrNone)
            {
            iObserver->ConnectCompleted( KErrAbort );        
            return;
            }        
        iConnectionTimer -> SetTimeOut ( TTimeIntervalMicroSeconds32( KBTAbortTimeout ) );    
        iConnectionTimer -> Start();    
        iClient->Abort();    
        }
    else if ( iClient && iClientState == EBTSCliConnecting)
        {
        iObserver->ConnectCompleted( KErrAbort );    
        }
    FLOG(_L("[BTSU]\t CBTServiceClient::Abort"));            
    
    }

//  End of File  
