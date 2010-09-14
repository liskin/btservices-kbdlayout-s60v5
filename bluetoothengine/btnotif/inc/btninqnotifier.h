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
* Description:  Declares Bluetooth device inquiry notifier class.
*
*/


#ifndef BTNINQNOTIFIER_H
#define BTNINQNOTIFIER_H

// INCLUDES

#include "btnotifier.h" // Base class
#include "btninqui.h"
#include <btextnotifiers.h> // Bluetooth notifiers API

// FORWARD DECLARATIONS
  
// CLASS DECLARATION
/**
* This class is used in Bluetooth device search.
*/
NONSHARABLE_CLASS(CBTInqNotifier): public CBTNotifierBase, public MBTNDeviceSearchObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTInqNotifier* NewL();

        /**
        * Destructor.
        */
        virtual ~CBTInqNotifier();
               
        
    private: // Functions from base classes
        /**
        * From CBTNotifierBase Called when a notifier is first loaded.        
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
         * From MBTNDeviceSearchObserver Called when device search and selection completes.
         * @param aErr Error code
         * @param aDevice Response parameters to caller
         *                  defined in epoc32\include\btextnotifiers.h
         */
        void NotifyDeviceSearchCompleted(TInt aErr, const TBTDeviceResponseParams& aDevice = TBTDeviceResponseParams());
        
    private:

        /**
        * C++ default constructor.
        */
        CBTInqNotifier();

        void LaunchInquiryL(const TBTDeviceClass& aDesiredDevice);
        
    private: // Data

        CBTInqUI*       iUi;                    // User interface module
    };

#endif

// End of File
