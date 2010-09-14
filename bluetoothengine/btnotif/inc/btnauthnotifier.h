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
* Description:  Declares authorisation notifier class.
*
*/


#ifndef BTNAUTHNOTIFIER_H
#define BTNAUTHNOTIFIER_H

// INCLUDES

#include "btnotifier.h" // Base class
#include "btnotiflock.h"
#include "btnotifactive.h"
#include <btdevice.h>
#include <e32property.h>

// FORWARD DECLARATIONS

// CLASS DECLARATION
NONSHARABLE_CLASS(CBTAuthNotifier): public CBTNotifierBase, public MBTNotifActiveObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTAuthNotifier* NewL();

        /**
        * Destructor.
        */
        virtual ~CBTAuthNotifier();

    private: // Functions from base classes

        /**
        * From CBTNotifierBase Called when a notifier is first loaded.        
        * @param None.
        * @return A structure containing priority and channel info.
        */
        TNotifierInfo   RegisterL();

        /**
        * From CBTNotifierBase Updates a currently active notifier.
        * @param aBuffer The updated data.
        * @return A pointer to return value.
        */
        TPtrC8 UpdateL(const TDesC8& aBuffer);

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
        * From CBTNotifierBase
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);        
        
        /**
         * From MBTNotifActiveObserver
         * Gets called when P&S key notifies change.
         */
        void RequestCompletedL( CBTNotifActive* aActive, TInt aId, TInt aStatus );
        
        /**
         * From MBTNotifActiveObserver
         */
        void HandleError( CBTNotifActive* aActive, TInt aId, TInt aError );
        
        /**
         * From MBTNotifActiveObserver
         */
        void DoCancelRequest( CBTNotifActive* aActive, TInt aId );

    private:

        /** Checks if the notifier should be replied automatically and not shown to user.
		 *
		 *@the device
		 *@ return TRUE if the notifier should be replied immediately yes or no, without showing query.
		 */
		TBool GetByPassAudioNotifier(const TBTDevAddr& aDevice,TBool& aAccept);
		
		/**
		 *@param aAccepted. ETrue if user accepted the incoming audio connection. EFalse if not.
		 *@param aDeviceAddress. The address of the device that attempted audio connection. 
		 */
		void MemorizeCurrentAudioAttempt(const TBool aAccept,const TBTDevAddr& aDeviceAddress);
		
        /**
        * C++ default constructor.
        */
        CBTAuthNotifier();

        /**
         * Ask user's response for authorization query
         */
        void ShowAuthoQueryL();
        
        /**
         * After user rejects authorization, provide user the chance to turn BT off
         * if it is a within a threshold. Query block the device according to logic of 
         * "second time for the same paired device". 
         */
        void DoRejectAuthorizationL();
        
        /**
         * Subscribe to P&S Key and check its value
         */
        void CheckAndSubscribeNotifLocks();
        
    private: // Data definitions

        enum TBTAuthState
            {
            EBTNormalAuthorisation,                     // Normal authorisation
            EBTAutoAuthorisation,                       // Automated Authorisation
            EBTAutoAuthorisationNotAuthorisedOnPhone,   
            EBTObexAuthorisation,                   // Obex authorisation for non paired devices
            EBTObexAuthForPairedDevice             // Obex authorisation for paired devices
           
            };

    private: // Data

        TBTAuthState    iAuthState;             // Authorisation state
        TInt            iStrResourceId;
        TSecondaryDisplayBTnotifDialogs iCoverUiDlgId;
		TUint			iServiceUid;
		TBool           iSuspended;
		RProperty       iNotifLockProp;
		CBTNotifActive* iLockActive;    
    };

#endif

// End of File
