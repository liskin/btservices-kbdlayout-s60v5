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
* Description:  CSrcsMessage definition
*
*/


#ifndef _SRCSMESSAGE_H
#define _SRCSMESSAGE_H

//  INCLUDES
#include <e32base.h>


// CLASS DECLARATION

/**
 *  RMessage2 container class.
 *  Ties together client requests and helper objects dealing with them.
 *  Messages must be completed via this class.
 */
class CSrcsMessage : public CBase
    {
public:  // Constructors and destructor

   /**
    * Two-phased constructor.
    * @param aMessage the message to handled
    */
    static CSrcsMessage* NewL( const RMessage2& aMessage );

   /**
    * Destructor.
    */
    virtual ~CSrcsMessage();

public: // New functions

   /**
    * Completes a message with given reason to client.
    * @param aReason    Reason value for complete.
    * @return none
    */
    void Complete( TInt aReason );

   /**
    * Returns RMessage2 reference to message.
    * @return Reference to message
    */
    const RMessage2& MessagePtr();

   /**
    * Returns pointer which can be used to cancel request.
    * @return Pointer to be used for cancellation
    */
    const TAny* CancelPtr();

   /**
    * Overload of comparison between CSrcsMessages
    * @param  aMessage  Reference to another CSrcsMessage
    * @return Boolean   True = Messages are same
    *                   False = Messaged differ.
    */
    TBool operator==( CSrcsMessage& aMessage ) const;      

private:

   /**
    * C++ default constructor.
    */
    CSrcsMessage( const RMessage2& aMessage );

private:    // Data
    RMessage2   iMessagePtr;    // The message to be completed
    const TAny* iCancelPtr;	    // Address of the TRequestStatus of the client-side active object - used for cancelling purposes    

    };

#endif      // SRCSMESSAGE_H

// End of File
