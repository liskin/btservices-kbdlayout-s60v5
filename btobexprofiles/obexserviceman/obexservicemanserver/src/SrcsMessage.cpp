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
* Description:  obexserviceman message handling.
*
*/


// INCLUDE FILES
#include    "SrcsMessage.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CSrcsMessage::CSrcsMessage(const RMessage2& aMessage)
    {
    iMessagePtr = aMessage;
    iCancelPtr = aMessage.Ptr2();
    }
// ---------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------
//
CSrcsMessage* CSrcsMessage::NewL(const RMessage2& aMessage)
    {
    return (new(ELeave) CSrcsMessage(aMessage));
    }

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CSrcsMessage::~CSrcsMessage()
    {
    }

// ---------------------------------------------------------
// Complete
// Completes the RMessage2 and deletes any associated helper.
// ---------------------------------------------------------
//
void CSrcsMessage::Complete(TInt aReason)
    {
    //complete the message
    iMessagePtr.Complete(aReason);
    }

// ---------------------------------------------------------
// MessagePtr
// Returns reference to RMessage2.
// ---------------------------------------------------------
//
const RMessage2& CSrcsMessage::MessagePtr()
    {
    return iMessagePtr;
    }

// ---------------------------------------------------------
// CancelPtr
// The CancelPtr is the address of the TRequestStatus of the
// client-side active object dealing with the request.
// When a request is cancelled, it is located server-side using
// this address.
// ---------------------------------------------------------
//
const TAny* CSrcsMessage::CancelPtr()
    {
    return iCancelPtr;
    }

// ---------------------------------------------------------
// operator==
// Compares Message pointers, if they are same, return value
// is ETrue, else EFalse.
// ---------------------------------------------------------
//
TBool CSrcsMessage::operator==(CSrcsMessage& aMessage) const
    {
    if (iMessagePtr == aMessage.MessagePtr())
        {
        return ETrue;
        }
    return EFalse;
    }

//  End of File
