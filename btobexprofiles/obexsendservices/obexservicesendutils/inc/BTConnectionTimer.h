/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Waiter class
*
*/



#ifndef BTCONNECTION_TIMER_H
#define BTCONNECTION_TIMER_H

//  INCLUDES
#include <e32base.h> // CTimer
#include "BTServiceClient.h"

// CLASS DECLARATION



// CLASS DECLARATION

/**
*  A timer object for waiting server connections.
*/
NONSHARABLE_CLASS (CBTConnectionTimer) : public CTimer
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTConnectionTimer* NewL( MBTConTimeObserver* aObserver );
        
        /**
        * Destructor.
        */
        virtual ~CBTConnectionTimer();

    private: // Functions from base classes

        /**
        * From CActive Get's called when the timer expires.
        * @param None.
        * @return None.
        */
        void RunL();

    private:

        /**
        * C++ default constructor.
        */
        CBTConnectionTimer( MBTConTimeObserver* aObserver );        
        

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
    public:
        /**
         * Set timeout value
         * @aTimeOutValue timeout value
         * @return None.
         */
        void SetTimeOut(TTimeIntervalMicroSeconds32 aTimeOutValue);      
        
        /**
         * Start
         * @aTimeOutValue timeout value
         * @return None.
         */
        void Start();      

    private: // Data

        // Not owned
        //
        MBTConTimeObserver* iObserver;
        TTimeIntervalMicroSeconds32 iTimeOutValue;
    };

#endif      // BTCONNECTION_TIMER_H
            
// End of File
