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
* Description:  SRCS session handler. Dispatchs requests from
*                clients and serves them.
*
*/



// INCLUDE FILES

#include "SrcsSession.h"
#include "obexsm.h"
#include "SrcsMessage.h"
#include "debug.h"
#include <btfeaturescfg.h>


// CONSTANTS

// The granularity of the array used to hold CSrcsMessage objects
static const TInt KMessageArrayGranularity = 4;



// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// constructor - must pass client to CSession2
// ---------------------------------------------------------
//
CSrcsSession::CSrcsSession(CObexSM* aServer) : CSession2(),
                                            iServer(aServer),
                                            iCanceling(EFalse)
    {
    if(iServer)
        {    
        iServer->IncSessionCount();
        }
    }
// ---------------------------------------------------------
// destructor
// ---------------------------------------------------------
//
CSrcsSession::~CSrcsSession()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsSession closed."));
    if ( iMessageArray )
        {
        // Cleanup the array
        iMessageArray->ResetAndDestroy();
        }
    delete iMessageArray;
    if(iServer)
        {    
        iServer->DecSessionCount();
        } 
    }
// ---------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------
//
CSrcsSession* CSrcsSession::NewL(CObexSM* aServer)
    {
    return new(ELeave) CSrcsSession(aServer);
    }

// ---------------------------------------------------------
// Server
// Return a reference to CSrcs
// ---------------------------------------------------------
CObexSM& CSrcsSession::Server()
    {
    return *iServer;
    }

// ---------------------------------------------------------
// CreateL
// 2nd phase construct for sessions
// - called by the CServer framework
// ---------------------------------------------------------
void CSrcsSession::CreateL()
    {
    // Add session to server first.
    // If anything leaves, it will be removed by the destructor
    ConstructL();
    }

// ---------------------------------------------------------
// ConstructL
// 2nd phase construct for sessions
// - called by the CServer framework
//
// ---------------------------------------------------------
void CSrcsSession::ConstructL()
    {
    iMessageArray = new(ELeave) CArrayPtrFlat<CSrcsMessage>(KMessageArrayGranularity);
    }

// ---------------------------------------------------------
// HandleError
// Handle an error from ServiceL()
// A bad descriptor error implies a badly programmed client, so panic it;
// otherwise report the error to the client
//
// ---------------------------------------------------------
void CSrcsSession::HandleError(TInt aError, const RMessage2& aMessage)
    {
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession::HandleError(): %d"), aError));
    if (aError==KErrBadDescriptor)
        {
        PanicClient( aMessage, ESrcsBadDescriptor);
        }
    else
        {
        CompleteMessage( aMessage, aError);
        }
    }


// ---------------------------------------------------------
// ServiceL
// Calls DispatchMessage under trap harness
//
// ---------------------------------------------------------
void CSrcsSession::ServiceL(const RMessage2& aMessage)
    {
    // First make our own message from RMessage2
    // TRAP because message needs to be completed
    // in case of Leave.        
    TRAPD( err, CreateSrcsMessageL( aMessage ));
    if ( err )
        {
        FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: ServiceL. Error %d when creating message."), err));
        aMessage.Complete( err );
        }
    else
        {
        err = KErrNone;
        TRAP( err ,DispatchMessageL( aMessage ));

        // If we have an error, try to handle it with handle-error.
        if ( err )
            {
            HandleError( err, aMessage );
            }
        }
    }

// ---------------------------------------------------------
// CreateSrcsMessageL
// Creates a CSrcsMessage and appends it to iMessageArray.
// ---------------------------------------------------------
//
void CSrcsSession::CreateSrcsMessageL(const RMessage2& aMessage)
    {
    __ASSERT_DEBUG(iMessageArray!=NULL, PanicServer(ESrcsClassMemberVariableIsNull));
    CSrcsMessage* message = CSrcsMessage::NewL(aMessage);
    CleanupStack::PushL( message );
    iMessageArray->AppendL( message );
    CleanupStack::Pop();
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: MessageArray count: %d"), iMessageArray->Count()));
    }

// ---------------------------------------------------------
// CompleteMessage
// Finds a message based on aMessage and completes it.
// ---------------------------------------------------------
//
void CSrcsSession::CompleteMessage(const RMessage2& aMessage, TInt aReason)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteMessage."));
    CSrcsMessage* message = FindMessage(aMessage);
    __ASSERT_DEBUG(message!=NULL, PanicServer(ESrcsBadMessage));
    message->Complete(aReason);
    DeleteMessage( message );
    }

// ---------------------------------------------------------
// CompleteMessage
// Completes given message.
// ---------------------------------------------------------
//
void CSrcsSession::CompleteMessage(CSrcsMessage* aMessage, TInt aReason)
    {
    __ASSERT_DEBUG(aMessage!=NULL, PanicServer(ESrcsBadMessage));
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteMessage."));
    aMessage->Complete(aReason);
    DeleteMessage(aMessage);
    }

// ---------------------------------------------------------
// CompletePendingMessages
// Completes any messages pending in the CBTManMessage array.
// ---------------------------------------------------------
void CSrcsSession::CompletePendingMessages()
    {
    __ASSERT_DEBUG(iMessageArray!=NULL, PanicServer(ESrcsClassMemberVariableIsNull));
    CSrcsMessage* messagePtr = NULL;
    TInt count = iMessageArray->Count();
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession::CompletePendingMessages. Message count: %d"), count));

    // Messages can be completed from array
    // and deleted after that.
    for ( TInt index = ( count-1 ) ; index >= 0 ; index-- )
        {
        messagePtr = iMessageArray->At(index);
        messagePtr->Complete(KErrDied);
        iMessageArray->Delete(index);
        delete messagePtr;
        messagePtr = NULL;
        }
    }

// ---------------------------------------------------------
// FindMessage
// Searches the array of CBTManMessages for the one dealing with aMessage.
// ---------------------------------------------------------
//
CSrcsMessage* CSrcsSession::FindMessage(const RMessage2& aMessage)
    {
    __ASSERT_DEBUG(iMessageArray!=NULL, PanicServer(ESrcsClassMemberVariableIsNull));
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: FindMessage."));
    RMessagePtr2 messagePtr = aMessage;
    CSrcsMessage* ptr = NULL;

    for ( TInt index = 0 ; index < iMessageArray->Count() ; index++ )
        {
        ptr = iMessageArray->At( index );
        if( ptr->MessagePtr() == messagePtr )
            {
            FLOG(_L("[SRCS]\tserver\tCSrcsSession: FindMessage. Message found"));
            return ptr;
            }
        }
    return NULL;
    }

// ---------------------------------------------------------
// FindMessage
// Searches the array for the 1st message with the function.
// ---------------------------------------------------------
//
CSrcsMessage* CSrcsSession::FindMessage(TInt aFunction)
    {
    __ASSERT_DEBUG(iMessageArray!=NULL, PanicServer(ESrcsClassMemberVariableIsNull));
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: FindMessage aFunction %d"), aFunction));
    CSrcsMessage* ptr = NULL;
    for ( TInt index = 0 ; index < iMessageArray->Count() ; index++ )
        {
        ptr = iMessageArray->At( index );
        if ( ptr->MessagePtr().Function() == aFunction )
            {
            FLOG(_L("[SRCS]\tserver\tCSrcsSession: FindMessage. Message found"));
            return ptr;
            }
        }
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: FindMessage. Message not found"));
    return NULL;
    }

// ---------------------------------------------------------
// DeleteMessage
// Find the CSrcsMessage in the message array and delete it.
// ---------------------------------------------------------
//
void CSrcsSession::DeleteMessage(CSrcsMessage* aMessage)
    {
    __ASSERT_DEBUG(iMessageArray!=NULL, PanicServer(ESrcsClassMemberVariableIsNull));
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: DeleteMessage. "));
    CSrcsMessage* ptr = NULL;
    TInt count = iMessageArray->Count();
    for ( TInt index = ( count-1 ) ; index >= 0 ; index-- )
        {
        ptr = iMessageArray->At( index );
        if( ptr == aMessage )
            {
            FLOG(_L("[SRCS]\tserver\tCSrcsSession: Message deleting. "));
            //Delete the message first before removing from the array since a helper associated
            //with the message will try to find the message by parsing the array as part of the
            //destruction the message.
            delete ptr;
            iMessageArray->Delete( index );
            ptr = NULL;
            break;
            }
        }
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: MessageArray count: %d"), iMessageArray->Count()));
    //compress the array if the count is less than the length - granularity AND if the count != 0
    if ( iMessageArray->Count() )
        {
        if (iMessageArray->Length() - iMessageArray->Count() >= KMessageArrayGranularity)
            iMessageArray->Compress();
        }
    }


// ---------------------------------------------------------
// DispatchMessageL
// service a client request; test the opcode and then do
// appropriate servicing
// ---------------------------------------------------------
//
void CSrcsSession::DispatchMessageL(const RMessage2 &aMessage)
    {
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: DispatchMessageL function %d"), aMessage.Function()));
    iCanceling=EFalse;
    switch (aMessage.Function())
        {
        // Turn BT services ON (with service handler).
    case ESrcsBTServicesON:
            {
			if ( BluetoothFeatures::EnterpriseEnablementL() != BluetoothFeatures::EEnabled )
				{
				CompleteMessage(aMessage, KErrNotSupported);
				}
			else
				{			
	            ManageServices(ESrcsTransportBT, ETrue, aMessage);
				}
            break;
            }
        // Turn BT services OFF.
    case ESrcsBTServicesOFF:
            {
            ManageServices(ESrcsTransportBT, EFalse, aMessage);
            break;
            }

        // Turn IrDA services ON (with service handler).
    case ESrcsIrDAServicesON:
            {
            ManageServices(ESrcsTransportIrDA, ETrue, aMessage);
            break;
            }
        // Turn IrDA services OFF.
    case ESrcsIrDAServicesOFF:
            {
            ManageServices(ESrcsTransportIrDA, EFalse, aMessage);
            break;
            }

    case ESrcsStartUSB:
            {
            ManageServices(ESrcsTransportUSB, ETrue, aMessage);
            break;
            }
    case ESrcsStopUSB:
            {
            ManageServices(ESrcsTransportUSB, EFalse, aMessage);
            break;
            }    
     case ESrcsCancelRequest:
            {            
            FLOG(_L("[SRCS]\tserver\tCSrcsSession: DispatchMessageL: ESrcsCancelRequest"));
            CancelingRequest(aMessage);
            break;
            }
    default:
            {
            PanicClient( aMessage, ESrcsBadRequest);
            break;
            }
        }
    }

// ---------------------------------------------------------
// ManageServicesL
// OpCode function; Manages Services.
// ---------------------------------------------------------
//
void CSrcsSession::ManageServices(TSrcsTransport aTransport, TBool aState, 
                                  const RMessage2& aMessage)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: ManageServicesL"));
    TInt retVal = Server().ManageServices(aTransport, aState, this, aMessage);
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: ManageServicesL return %d"), retVal));
    }

// ---------------------------------------------------------
// CancelingRequest
// Canceling the ongoing request.
// ---------------------------------------------------------
//
void CSrcsSession::CancelingRequest(const RMessage2& aMessage)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: CancelingRequest"));
    iCanceling=ETrue;          
    if (iMessageArray->Count() == 1)
        {
        FLOG(_L("[SRCS]\tserver\tCSrcsSession: CancelingRequest completing"));
        CompleteMessage(aMessage, KErrGeneral);        
        }
    else
        {
        //The cancel handling continues, when another request completes.
        }
    }
    
// ---------------------------------------------------------
// RequestCompleted
// ---------------------------------------------------------
//   
void CSrcsSession::RequestCompleted(const RMessage2 &aMessage, TInt aError)
    {          
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: RequestCompleted"));
    TBool postponeCompletion = EFalse;
    if ( !iCanceling )
        {
        FLOG(_L("[SRCS]\tserver\tCSrcsSession: RequestCompleted completing"));
        CompleteMessage( aMessage, aError ); 
        }
    else
        {
        CompleteCanceling(aMessage, aError, postponeCompletion);
        }
    if ( !postponeCompletion && iMessageArray->Count() > 0 )
        {
        FLOG(_L("[SRCS]\tserver\tCSrcsSession: RequestCompleted dispatching"));
        CSrcsMessage* messagePtr = NULL; 
        messagePtr=iMessageArray->At( 0 ); 
        RMessage2 message=messagePtr->MessagePtr(); 
        TRAP_IGNORE( DispatchMessageL( message ) );
        }    
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: RequestCompleted exit"));
    }

// ---------------------------------------------------------
// CompleteCanceling
// Only USB service cancel is handled, since Locod Service 
// Plugin API does not allow cancel for BT or IrDA.
// ---------------------------------------------------------
//
void CSrcsSession::CompleteCanceling(const RMessage2& aMessage, TInt aError, 
                                     TBool& aPostponeCompletion)
    {          
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteCanceling"));
    TBool cancelMsgFound = EFalse;
    CSrcsMessage* messagePtr = NULL;
    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsSession: CompleteCanceling function %d"), 
        aMessage.Function()));
    switch ( aMessage.Function() )
        {    
        case ESrcsStartUSB:
            messagePtr = FindMessage(ESrcsCancelRequest);
            if ( messagePtr )
                {
                FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteCanceling ESrcsStartUSB found cancel req"));
                RMessage2 message=messagePtr->MessagePtr();                
                ManageServices(ESrcsTransportUSB, EFalse,  message);
                aPostponeCompletion = ETrue;
                cancelMsgFound = ETrue;
                }
            break;
        case ESrcsStopUSB:
            messagePtr = FindMessage(ESrcsCancelRequest);
            if ( messagePtr )
                {
                CompleteMessage(aMessage, KErrCancel); 
                CompleteMessage(messagePtr, aError); 
                cancelMsgFound = ETrue;
                }
            break;
        case ESrcsCancelRequest:
            //either start or stop USB
            messagePtr = FindMessage(ESrcsStartUSB);
            if ( !messagePtr )
                {
                messagePtr = FindMessage(ESrcsStopUSB);
                }
            if ( messagePtr )
                {
                CompleteMessage(messagePtr, KErrCancel); 
                CompleteMessage(aMessage, aError);
                cancelMsgFound = ETrue;
                }
            break;
        default:
            FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteCanceling ESrcsCancelRequest unhandled msg"));
            break;
        }
    if ( !cancelMsgFound )
        {
        FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteCanceling no message"));
        CompleteMessage(aMessage, KErrGeneral);
        }
    FLOG(_L("[SRCS]\tserver\tCSrcsSession: CompleteCanceling exit"));    
    }
// End of File
