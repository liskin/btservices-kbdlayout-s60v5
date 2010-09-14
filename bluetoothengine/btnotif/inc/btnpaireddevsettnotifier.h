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
* Description:  Declares power mode setting notifier class.
*
*/


#ifndef BTNPAIREDDEVSETTNOTIFIER_H
#define BTNPAIREDDEVSETTNOTIFIER_H

// INCLUDES
#include "btnotifier.h" // Base class

// FORWARD DECLARATIONS

// CLASS DECLARATION
NONSHARABLE_CLASS(CBTPairedDevSettNotifier) : public CBTNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTPairedDevSettNotifier* NewL();

        /**
        * Destructor.
        */
        virtual ~CBTPairedDevSettNotifier();

    private: // Functions from base classes

        /**
        * From CBTNotifierBase Called when a notifier is first loaded 
        * to allow any initial construction that is required.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        TNotifierInfo RegisterL();

         /**
        * From CBTNotifierBase The notifier has been deactivated 
        * so resources can be freed and outstanding messages completed.
        * @param None.
        * @return None.
        */
        void Cancel();        

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
        * From CBTNotifierBase Updates a currently active notifier.
        * @param aBuffer The updated data.
        * @return A pointer to return value.
        */
        TPtrC8 UpdateL(const TDesC8& aBuffer);
        
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
		void ProcessParamBufferL(const TDesC8& aBuffer);
	
    	/**
    	 * Query to assign the friendly name when adding it into paired device list
    	 * if the current device name is not unique in the paired devices view.
    	 */
		void QueryRenameDeviceL(const CBTDevice& aDevice);
    	
        /**
        * C++ default constructor.
        */
        CBTPairedDevSettNotifier();        
        
        /**
         * Process pairing error code to show corresponding notes.
         * @param aErr The pairing error code from HCI error.
         * @return The resouce ID of notes
         */
        TInt ProcessPairingErrorCode( const TInt aErr );
                
    private:
	
        TSecondaryDisplayBTnotifDialogs	iSecondaryDisplayCommand;
		TBool 	iIsMessageQuery;
		TInt 	iPairingStatus; 
		
    };

#endif //BTNPAIREDDEVSETTNOTIFIER_H

// End of File
