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
* Description:  Declares authentication (PIN query) notifier class.
*
*/


#ifndef BTNPINNOTIFIER_H
#define BTNPINNOTIFIER_H

// INCLUDES

#include "btnpairnotifier.h" // Base class 

class MBTEngDevManObserver;

// CLASS DECLARATION
/**
* This class is used to ask PIN code from user.
*/
NONSHARABLE_CLASS(CBTPinNotifier): public CBTNPairNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTPinNotifier* NewL();  // Constructor (public)

        /**
        * Destructor.
        */
        virtual ~CBTPinNotifier();      // Destructor

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
        * Show queries to ask user response and return it to caller by completing message.
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);
        
        /**
        * From CBTNPairNotifierBase
        * Generate prompt in Pin query dialog.
        */      
        HBufC* GenerateQueryPromoptLC(); 

    private:

        /**
        * C++ default constructor.
        */
        CBTPinNotifier();               // Default constructor
        
        /**
        * Checks and sets iPinCode to '0000', used with automated pairing for headsets.
        */		        
        TBool CheckAndSetAutomatedPairing();        	

    private: // Data
        TUint                   iPasskeyLength;
    	TBTPinCode              iPinCode;
		TBTRegistryQueryState   iBTRegistryQueryState;
        TBool                   iLocallyInitiated;
        TBool                   iStrongPinRequired;
		        
    };

#endif

// End of File
