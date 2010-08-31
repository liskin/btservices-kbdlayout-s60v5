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
* Description:  Object push controller implementation
*
*/



// INCLUDE FILES
#include "BTSOPPController.h"
#include "BTSUDebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSOPPController::CBTSOPPController
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSOPPController::CBTSOPPController( MBTServiceObserver* aObserver,
                                     CBTServiceParameterList* aList ) : 
                                     iListPtr( aList ), 
                                     iObserverPtr( aObserver )
    {
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSOPPController::ConstructL( const TUint aRemotePort, const TBTDevAddr& aRemoteDeviceAddr )
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::ConstructL()"));
    
    RArray<CObexHeader*> headerList;
    CleanupClosePushL( headerList );
    CreateClientL ( this, aRemoteDeviceAddr, aRemotePort, headerList );
    CleanupStack::Pop();   
    headerList.Close();
   
    FLOG(_L("[BTSS]\t CBTSOPPController::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSOPPController* CBTSOPPController::NewL( MBTServiceObserver* aObserver,
                                           const TUint aRemotePort,
                                           const TBTDevAddr& aRemoteDeviceAddr,
                                           CBTServiceParameterList* aList )
    {
    CBTSOPPController* self = new( ELeave ) CBTSOPPController( aObserver, aList );
    CleanupStack::PushL( self );
    self->ConstructL( aRemotePort, aRemoteDeviceAddr );
    CleanupStack::Pop();
    return self;
    }

// Destructor
CBTSOPPController::~CBTSOPPController()
    {
    FLOG(_L("[BTSU]\t CBTSOPPController::Destructor()"));    
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::ConnectCompleted
// -----------------------------------------------------------------------------
//
void CBTSOPPController::ConnectCompleted( TInt aStatus )
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::ConnectCompleted()"));
    
    if ( aStatus == KErrNone )
        {
        TRAPD( error, HandleConnectCompleteIndicationL() );
        if ( error != KErrNone )
            {
            // Sending initialisation has failed
            //
            iObserverPtr->ControllerComplete( EBTSConnectingFailed );
            }
        }
    else
        {
        //Error on Obex level
        //
        iObserverPtr->ControllerComplete( EBTSConnectingFailed );
        }

    FLOG(_L("[BTSS]\t CBTSOPPController::ConnectCompleted() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::ClientConnectionClosed
// -----------------------------------------------------------------------------
//
void CBTSOPPController::ClientConnectionClosed()
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::ClientConnectionClosed()"));

    // Everything ready, stop service
    //
    iObserverPtr->ControllerComplete( EBTSNoError );

    FLOG(_L("[BTSS]\t CBTSOPPController::ClientConnectionClosed() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::PutCompleted
// -----------------------------------------------------------------------------
//
void CBTSOPPController::PutCompleted( TInt aStatus, const CObexHeaderSet* aPutResponse )
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::PutCompleted()"));
	aPutResponse=aPutResponse;

  
    if ( aStatus == KErrNone )
        {       
        iFileIndex++;
        Send();        
        }
    else
        {
        iObserverPtr->ControllerComplete( EBTSPuttingFailed );
        }
    
    FLOG(_L("[BTSS]\t CBTSOPPController::PutCompleted() done"));
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::GetCompleted
// -----------------------------------------------------------------------------
//
void CBTSOPPController::GetCompleted( TInt /*aStatus*/, CObexBufObject* /*aGetResponse*/ )
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::GetCompleted() Not used with OPP"));
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::Send
// -----------------------------------------------------------------------------
//
void CBTSOPPController::Send()
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::Send()"));
    
    TInt error(KErrNone);

    if ( iListPtr->ObjectCount() > 0 && iFileIndex < iListPtr->ObjectCount() )
        {    
        RArray<CObexHeader*> headerList; // the array does not need to be closed    

        TRAP( error, {
                UpdateProgressNoteL();
                iClient->PutObjectL( headerList, iListPtr->ObjectAtL( iFileIndex ));  });

        if ( error )
            {
            FTRACE(FPrint(_L("[BTSU]\t CBTSOPPController::Send() putting leaved with %d"), error ));
            iObserverPtr->ControllerComplete( EBTSPuttingFailed );
            }
        }
    else
        {        
        FLOG(_L("[BTSU]\t CBTSOPPController::Send() all objects sent, closing connection"));

        // All objects sent, close client connection.
        //
        iClient->CloseClientConnection();
        }

    FLOG(_L("[BTSS]\t CBTSOPPController::Send() Done"));
    }

//-----------------------------------------------------------------------------
// void CBTSOPPController::ConnectTimedOut()
// -----------------------------------------------------------------------------
//        
void CBTSOPPController::ConnectTimedOut()    
    {
    iObserverPtr->ConnectTimedOut();
    }

// -----------------------------------------------------------------------------
// CBTSOPPController::HandleConnectCompleteIndicationL
// -----------------------------------------------------------------------------
//
void CBTSOPPController::HandleConnectCompleteIndicationL()
    {
    FLOG(_L("[BTSS]\t CBTSOPPController::HandleConnectCompleteIndicationL()"));
    TBTSUImageParam imgparam;
    if ( iListPtr->ImageCount() > 0 )
        {
        for( TInt index = 0; index < iListPtr->ImageCount(); index++ )
            {
            // Copy files from Image list to Object list
            // from now on we use only object list.
            imgparam = iListPtr->ImageAtL( index );
            if(!imgparam.iSend)
                {
                iListPtr->AddObjectL( imgparam.iFile);
                }            
            }
        }
    iObserverPtr->LaunchProgressNoteL( iClient, iListPtr->ObjectCount() );
    iFileIndex = 0;
    UpdateProgressNoteL();    
    Send();
    }


void CBTSOPPController::UpdateProgressNoteL()
    {
    TInt size;
    RBuf filename;
    iListPtr->ObjectAtL( iFileIndex ).Size(size);
    filename.CreateL(KMaxFileName);
    iListPtr->ObjectAtL( iFileIndex ).Name(filename);
    iObserverPtr->UpdateProgressNoteL(size,iFileIndex,filename);
    filename.Close();
    }

//  End of File  
