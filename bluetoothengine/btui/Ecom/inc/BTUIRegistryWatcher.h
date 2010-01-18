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
* Description:  Declares Registry watcher class for BTUI.
*
*/


#ifndef CBTUIREGISTRYWATCHERAO_H
#define CBTUIREGISTRYWATCHERAO_H

// INCLUDES

#include <bt_subscribe.h>

// CLASS DECLARATION

/**
* A callback interface for informing content change of BT Registry.
*/
class MBTUIRegistryObserver
    {
    public: // New functions

        /**
        * Informs the observer that contents of Registry has been changed.
        * @param None.
        * @return None.
        */
        virtual void RegistryContentChangedL()=0;
    };


// CLASS DECLARATION

/**
* An active object based class which can used to listen
* Registry changes
*/
class CBTUIRegistryWatcherAO : public CActive
    {
    public: // Constructors and destructor

        /**
        * C++ default constructor.
        */
        CBTUIRegistryWatcherAO( MBTUIRegistryObserver* aParent );

        /**
        * Destructor.
        */
        virtual ~CBTUIRegistryWatcherAO();

    public: // New functions

        /**
        * Starts / continues Registry watching
        *
        * @param None.
        * @return None.
        */
        void WatchL();

        /**
        * Starts / continues Registry watching
        *
        * @param None.
        * @return None.
        */
        void StopWatching();

    private: // Functions from base classes

        /**
        * From CActive Gets called when CActive::Cancel is called, 
        *              cancels Registry watching.
        *
        * @param None.
        * @return None.
        */
        void DoCancel();

        /**
        * From CActive Gets called when content of Registry is changed, 
        *              calls MBTUIRegistryObserver::RegistryContentChangedL.
        *
        * @param None.
        * @return None.
        */
        void RunL();       

        /**
        * From CActive Handles a leave occurring in the request completion event.        
        *
        * @param aError The leave code.
        * @return Symbian OS error code.
        */
        TInt RunError(TInt aError);

    private: // Data

        // Reference to observer
        MBTUIRegistryObserver* iParent;
        // Database handle
        RProperty iProperty;
    };

#endif

// End of File
