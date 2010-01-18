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


#ifndef BTNSSPPASSKEYENTRYNOTIFIER_H_
#define BTNSSPPASSKEYENTRYNOTIFIER_H_


// INCLUDES

#include "btnpairnotifier.h" // Base class

const TInt KPassKeylength = 6; 
// CLASS DECLARATION
/**
* This class is used to confirm PIN code from remote input devices. such as bluetooth keyboard.
*/
NONSHARABLE_CLASS(CBTSSPPasskeyEntryNotifier): public CBTNPairNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTSSPPasskeyEntryNotifier* NewL();  // Constructor (public)

        /**
        * Destructor.
        */
        virtual ~CBTSSPPasskeyEntryNotifier();      // Destructor

    private:
       /**
        * From CBTNotifierBase Called when a notifier is first loaded 
        * to allow any initial construction that is required.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        TNotifierInfo RegisterL();

        /**
        * From CBTNotifierBase Updates a currently active notifier.
        * @param aBuffer The updated data.
        * @return A pointer to return value.
        */
        TPtrC8 UpdateL(const TDesC8& aBuffer);                         
               
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
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);        
    
        /**
        * Generate prompt for passkey entry query.
        * @param None.
        * @return None.
        */
        HBufC* GenerateQueryPromoptLC();
        
    private:

        /**
        * C++ default constructor.
        */
        CBTSSPPasskeyEntryNotifier();               // Default constructor
        
    private: // Data
        TInt                    iAnswer;
        TBuf<100>               iBuf;
        TBuf<10>                iPasskey;
    };
#endif
// End of File
