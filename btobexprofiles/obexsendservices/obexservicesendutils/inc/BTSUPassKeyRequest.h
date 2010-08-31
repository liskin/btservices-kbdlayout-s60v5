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
* Description:  CBTSUPasskeyRequest Header definitions
*
*/


#ifndef BTSU_PASSKEY_REQUEST_H
#define BTSU_PASSKEY_REQUEST_H

//  INCLUDES
#include <e32base.h>
#include <obexclient.h>
#include <btnotif.h>

// CLASS DECLARATION

/**
*  Class to handle BT Obex Passkey events
*/
NONSHARABLE_CLASS (CBTSUPasskeyRequest) : public CActive
    {

    public: // Constructors and destructor

        /**
        * C++ default constructor.
        */
        CBTSUPasskeyRequest();

        /**
        * Destructor.
        * Closes RNotifier
        */
        ~CBTSUPasskeyRequest();

    public: // New functions
        /**
        * Starts BT ObexPasskey notifier
	    * @param aObexServer OBEX server
        */
        void StartPassKeyRequestL( CObexClient* aObexclient );

    private: // Functions from base classes
        /**
        * From CActive Get's called when a request is cancelled.
        * @param None.
        * @return None.
        */
        void DoCancel();

        /**
        * From CActive Get's called when a request is completed.
        * @param None.
        * @return None.
        */
        void RunL();

        /**
        * From CActive A leave occurred in RunL.
        * @param aError The leave error.
        * @return Symbian OS error code.
        */
        TInt RunError();

    private: // Data

        RNotifier iNotif;               // For Obex passkey notifier
        TObexPasskeyBuf iObexPasskey;   // Obex passkey
        CObexClient* iObexClient;       // Obex client
    };

#endif

// End of File
