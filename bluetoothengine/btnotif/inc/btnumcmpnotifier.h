/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Numeric Comparison notifier for Secure Simle Pairing 
*
*/


#ifndef BTNUMCMPNOTIFIER_H
#define BTNUMCMPNOTIFIER_H

// INCLUDES

#include "btnpairnotifier.h" // Base class

// CLASS DECLARATION
/**
* This class is used to ask user to compare passcode in two devices.
*/
const TInt Klength = 6; 
const TInt KMaxPassKeyLength = 20;
const TInt KBTDeviceShortNameLength = 7;


NONSHARABLE_CLASS(CBTNumCmpNotifier): public CBTNPairNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTNumCmpNotifier* NewL();  // Constructor (public)

        /**
        * Destructor.
        */
        virtual ~CBTNumCmpNotifier();      // Destructor

    private:
        /**
        * From CBTNotifierBase Called when a notifier is first loaded 
        * to allow any initial construction that is required.
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
        void GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage); // Get input parameters
                                         
        /**
        * From CBTNotifierBase
        * Show notes to ask user response and return to Notifier caller by completing message.
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);        
        
        /**
        * From CBTNPairNotifierBase
        * @param aRBuf the descriptor to which the prompt will be loaded.
        */
        void GenerateQueryPromptL( RBuf& aRBuf );

    private:

        /**
        * C++ default constructor.
        */
        CBTNumCmpNotifier();               // Default constructor
		
    private: // Data
		TBuf<20> iPasskeyToShow;
        
    };

#endif

// End of File
