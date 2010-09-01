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
* Description:  Declares generic information notifier class.
*
*/


#ifndef BTNGENERICINFONNOTIFIER_H
#define BTNGENERICINFONNOTIFIER_H

// INCLUDES

#include "btnotifier.h" // Base class

// FORWARD DECLARATIONS
class CAknInformationNote;

// CLASS DECLARATION
NONSHARABLE_CLASS(CBTGenericInfoNotifier) : public CBTNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTGenericInfoNotifier* NewL();

        /**
        * Destructor.
        */
        virtual ~CBTGenericInfoNotifier();

    private: // Functions from base classes

        /**
        * From CBTNotifierBase Called when a notifier is first loaded 
        * to allow any initial construction that is required.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        TNotifierInfo RegisterL();
        
        /** From CBTNotifierBase Synchronic notifier launch.        
        * @param aBuffer Received parameter data.
        * @return A pointer to return value.
        */
        TPtrC8 StartL(const TDesC8& aBuffer );

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
 
        /**
        * From CBTNotifierBase
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);        

    private:
    
		/**
        * Parse the data out of the message that is sent by the client of the notifier. 
		* @param aBuffer A package buffer containing received parameters.
        * @return None.
        */
		void ProcessParamBufferL(const TDesC8& aBuffer, TBool aSyncCall);
		
        /**
        * C++ default constructor.
        */
        CBTGenericInfoNotifier();

        /**
         * Show Information note and complete message. 
         */
        void ShowNoteAndCompleteL(TBool aSyncCall);
        
    private:
        RBuf	iQueryMessage;
	    TInt 	iMessageResourceId;
	    TSecondaryDisplayBTnotifDialogs iSecondaryDisplayCommand;
    };

#endif

// End of File
