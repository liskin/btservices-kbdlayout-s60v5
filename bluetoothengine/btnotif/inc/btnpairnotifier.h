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
* Description:  Declares pair base notifier.
*
*/

#ifndef BTNPAIRNOTIFIER_H_
#define BTNPAIRNOTIFIER_H_

#include "btnotifier.h" // base class

// CLASS DECLARATION
/**
* This class is used as base class for all pairing purpose notifiers
*/
NONSHARABLE_CLASS(CBTNPairNotifierBase) : public CBTNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTNPairNotifierBase* NewL();  // Constructor (public)

        /**
        * Destructor.
        */
        virtual ~CBTNPairNotifierBase();      // Destructor
    
    protected: // From base class
    
        /**
        * From CBTNotifierBase
        */
        virtual TNotifierInfo RegisterL();
        
        /**
        * From CBTNotifierBase
        * Used in asynchronous notifier launch to start the actual processing
        * of parameters received in StartL. StartL just stores a copy of the
        * parameter buffer, schedules a callback to call this method and returns
        * ASAP, so the actual work begins here, safely outside of the StartL
        * context so that waiting dialogs can be freely used if need be.
        * @return None.
        */
        virtual void ProcessStartParamsL();
        
        virtual void UpdateL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);
        
        /**
        * From CBTNotifierBase Updates a currently active notifier.
        * @param aBuffer The updated data.
        * @return A pointer to return value.
        */
        virtual TPtrC8 UpdateL(const TDesC8& aBuffer);
        
    protected: // New functions

        void ProcessParamsGetDeviceL( const TBTDevAddr& aAddr, const TBTDeviceName& aName );
        
        /**
         * Check not to allow pairing attempt from banned devices,
         */
        TBool CheckBlockedDeviceL();
        
        /**
        * Query user to accept/reject when the pairing request is initiated 
        * from remote device.   
        * @param None.
        * @return ETrue if User accepted.
        */
        TBool AuthoriseIncomingPairingL();
        
        /**
         * Generate prompt based on subclass's own needs.
         * @param aRBuf the descriptor to which the prompt will be loaded.
         */
        virtual void GenerateQueryPromptL( RBuf& aRBuf );
        
    private:
        /**
        * From CBTNotifierBase Called when a notifier is first loaded 
        * to allow any initial construction that is required.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        //TNotifierInfo RegisterL();
        
    protected:
        /**
        * C++ default constructor.
        */    
        CBTNPairNotifierBase();
        
    protected: // Data
        TBool                   iLocallyInitiated;    
    };

#endif /* BTNPAIRNOTIFIER_H */

