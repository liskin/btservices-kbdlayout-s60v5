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
        * From CBTNotifierBase
        * Used in asynchronous notifier launch to start the actual processing
        * of parameters received in StartL. StartL just stores a copy of the
        * parameter buffer, schedules a callback to call this method and returns
        * ASAP, so the actual work begins here, safely outside of the StartL
        * context so that waiting dialogs can be freely used if need be.
        * @return None.
        */
        void ProcessStartParamsL();

        /**
        * From CBTNotifierBase
        * Show queries to ask user response and return it to caller by completing message.
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);
        
        /**
        * From CBTNPairNotifierBase
        * Generate prompt in Pin query dialog.
        * @param aRBuf the descriptor to which the prompt will be loaded.
        */      
        void GenerateQueryPromptL(RBuf& aRBuf); 

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
