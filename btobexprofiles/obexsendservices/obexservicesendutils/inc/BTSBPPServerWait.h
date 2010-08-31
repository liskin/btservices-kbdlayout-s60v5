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
* Description:  Waiter class
*
*/



#ifndef BTSBPP_SERVER_WAIT_H
#define BTSBPP_SERVER_WAIT_H

//  INCLUDES
#include <e32base.h> // CTimer

// CLASS DECLARATION

/**
*  An interface used to inform about CBTSBPPServerWait completion.
*/
class MBTSBPPServerWaitObserver
    {
    public:

        /**
        * Informs the observer that the waiting has been completed.
        * @param None.
        * @return None.
        */
        virtual void WaitComplete() = 0;
    };

// CLASS DECLARATION

/**
*  A timer object for waiting server connections.
*/
NONSHARABLE_CLASS (CBTSBPPServerWait) : public CTimer
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSBPPServerWait* NewL( MBTSBPPServerWaitObserver* aObserver );
        
        /**
        * Destructor.
        */
        virtual ~CBTSBPPServerWait();

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
        CBTSBPPServerWait( MBTSBPPServerWaitObserver* aObserver );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // Data

        // Not owned
        //
        MBTSBPPServerWaitObserver* iObserver;
    };

#endif      // BTSBPP_SERVER_WAIT_H
            
// End of File
