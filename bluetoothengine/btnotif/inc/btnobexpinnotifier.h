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
* Description:  Declares OBEX PIN query notifier class.
*
*/


#ifndef BTNOBEXPINNOTIFIER_H
#define BTNOBEXPINNOTIFIER_H

// INCLUDES

#include "btnotifier.h" // Base class

// CLASS DECLARATION

/**
* This class is used to ask OBEX passkey from user.
*/
NONSHARABLE_CLASS(CBTObexPinNotifier): public CBTNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTObexPinNotifier* NewL();

        /**
        * Destructor.
        */
        virtual ~CBTObexPinNotifier();

    private: // Functions from base classes
        /**
        * From CBTNotifierBase Called when a notifier is first loaded.        
        * @param None.
        * @return A structure containing priority and channel info.
        */
        TNotifierInfo RegisterL();

        /**
        * From CBTNotifierBase Used in asynchronous notifier launch to 
        * store received parameters into members variables and 
        * make needed initializations.
        * @param aBuffer A buffer containing received parameters
        * @param aReturnVal The return value to be passed back.
        * @param aMessage Should be completed when the notifier is deactivated.
        * @return None.
        */
        void GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);
               
    private:

        /**
        * C++ default constructor.
        */
        CBTObexPinNotifier();      

        /**
        * Show notes to ask user's response and return to caller.
        * @param None.
        * @return None.
        */
        void ShowNoteCompleteMessageL();

    private: // Data

    };

#endif

// End of File
