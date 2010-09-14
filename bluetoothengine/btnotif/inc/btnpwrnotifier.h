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


#ifndef BTNPWRNOTIFIER_H
#define BTNPWRNOTIFIER_H

// INCLUDES

#include "btnotifier.h" // Base class

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
* This class is used to confirm Bluetooth power status change from user.
*/
NONSHARABLE_CLASS(CBTPwrNotifier): public CBTNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTPwrNotifier* NewL();

        /**
        * Destructor.
        */
        virtual ~CBTPwrNotifier();

    private: // Functions from base classes

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

    private:

        /**
        * C++ default constructor.
        */
        CBTPwrNotifier();
        
        /**
        * Show Information Note and complete message.
        * @param None.
        * @return None.
        */
        void ShowNoteAndCompleteMessageL(); 

    };

#endif

// End of File
