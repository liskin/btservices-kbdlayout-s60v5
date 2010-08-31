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
* Description:  SRCS session definition.
*
*/



#ifndef _SRCSSESSION_H
#define _SRCSSESSION_H

//  INCLUDES

#include <e32svr.h>
#include "obexserviceman.h"
#include "SrcsServiceManager.h"

// FORWARD DECLARATIONS
class CObexSM;
class CSrcsMessage;

// CLASS DECLARATION

/**
 *  CSrcsSession class represent session on server side
 *
 */
class CSrcsSession : public CSession2, public MObexSMRequestObserver
    {
public:  // Constructors and destructor

   /**
    * Two-phased constructor.
    */
    static CSrcsSession* NewL(CObexSM* aServer );

   /**
    * Destructor.
    */
    virtual ~CSrcsSession();

   /**
    * 2nd phase construct for sessions    
    * @return   None
    */
    void CreateL( );

public: // Functions from base classes

   /**
    * From CSession2 ServiceL
    * Client-server framework calls this function
    * to handle messages from client.
    * @param    aMessage received message class
    * @return   None
    */
    virtual void ServiceL( const RMessage2 &aMessage );

   /**
	* From CSession2 DispatchMessageL
	* Dispatchs and calls appropriate methods to
	* handle messages.
	* @param    aMessage received message class
	* @return   None
	*/
    void DispatchMessageL( const RMessage2 &aMessage );

   /**
    * From CSession2 Server
    * Returns reference to server.
    * @param    None
    * @return   Reference to server
    */
    CObexSM& Server();

public: // New functions

   /**
    * Complete Request
    * @param    aMessage Request message to be completed    
    * @param    aError Error code
    * @return   None.
    */
    void RequestCompleted(const RMessage2 &aMessage, TInt aError);

private:  // New functions

   /**
    * OpCode function
    * Turns Services ON/OFF according to aState
    * @param    aState boolean to indicate status of service.
    * @return   None.
    */
    void ManageServices( TSrcsTransport aTransport, TBool aState, const RMessage2& aMessage );

   /**
    * Handles error situation. Panics client
    * or completes message with error status.
    * @param    aError  Error code
    * @return None
    */
    void HandleError( TInt aError, const RMessage2& aMessage );

   /**
    * Deletes message from message array
    * @param    aMessage    Pointer to message to be deleted.
    * @return   None
    */
    void DeleteMessage( CSrcsMessage* aMessage );

    /**
    * Finds a message from message array
    * @param    aMessage -reference to message.
    * @return   Pointer to CSrcsMessage -instance.
    */
    CSrcsMessage* FindMessage( const RMessage2& aMessage );

    /**
    * Finds the first message with the function from message array
    * @param    aFunction -the request function.
    * @return   Pointer to CSrcsMessage -instance, NULL if not found.
    */
    CSrcsMessage* FindMessage( TInt aFunction );

   /**
    * Completes a message to client
    * @param    aMessage -reference to message.
    * @param    aReason -code to be delivered to client.
    */
    void CompleteMessage( const RMessage2& aMessage, TInt aReason );

   /**
    * Completes a message to client
    * @param    aMessage -reference to message.
    * @param    aReason -code to be delivered to client.
    */
    void CompleteMessage( CSrcsMessage* aMessage, TInt aReason );

   /**
    * Completes any messages pending in the CBTManMessage array.    
    */
    void CompletePendingMessages();

   /**
    * Creates a new CSrcsMessage and puts it in
    * message array.
    * @param    aMessage -reference to message.
    * @return   None
    */
    void CreateSrcsMessageL( const RMessage2& aMessage );
    
   /**
    * Canceling previous request
    * @param    aMessage -reference to message.    
    */
    void CancelingRequest(const RMessage2& aMessage);    
  
   /**
    * Complete request when canceling
    * @param    aMessage Request message to be completed    
    * @param    aError Error code
    * @param    aPostponeCompletion Not completed yet
    * @return   None.
    */  
    void CompleteCanceling(const RMessage2& aMessage, TInt aError, TBool& aPostponeCompletion);
    
private:

   /**
    * C++ default constructor.
    */
    CSrcsSession( CObexSM* aServer );

   /**
    * Two-phase constructor.
    */
    void ConstructL();


private:    // Data
    CArrayPtr<CSrcsMessage>*    iMessageArray;	    // array of outstanding messages
    CObexSM                     *iServer;
    TBool                       iCanceling;
    };

#endif      // SRCSSESSION_H

// End of File
