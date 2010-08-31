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
* Description:    
*
*/


#ifndef COBEXUTILSDIALOGTIMER_H
#define COBEXUTILSDIALOGTIMER_H

//  INCLUDES
#include "obexutilsdialog.h"

// CLASS DECLARATION
/**
*  A timer class for updating progress dialog.
*/
NONSHARABLE_CLASS( CObexUtilsDialogTimer ) : public CTimer
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */                           
        static CObexUtilsDialogTimer* NewL( CObexUtilsDialog* aParent );
        
        /**
        * Destructor.
        */
        virtual ~CObexUtilsDialogTimer();
     
    public: // New functions

        /**
        * Sets the timeout of the timer.
        * @param aTimeout The timeout in microseconds.
        * @return None.
        */
        void SetTimeout( TTimeIntervalMicroSeconds32 aTimeout );

        /**
        * Restarts the timer.
        * @return None.
        */
        void Tickle();

    private: // Functions from base classes

        /**
        * From CTimer Get's called when the timer expires.        
        * @return None.
        */
        void RunL();

    private:

        /**
        * C++ default constructor.
        */
        CObexUtilsDialogTimer( CObexUtilsDialog* aParent );
        
        /**
        * By default Symbian OS constructor is private.
        */      
        void ConstructL();

    private: // Data
        TTimeIntervalMicroSeconds32 iTimeout;
        CObexUtilsDialog*           iParent;
    };

#endif      // COBEXUTILSDIALOGTIMER_H
            
// End of File
