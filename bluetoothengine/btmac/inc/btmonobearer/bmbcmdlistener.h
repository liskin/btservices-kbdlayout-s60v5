/*
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Remote connection observer interface declaration.
*
*/


#ifndef BMBDATALISTENER_H
#define BMBDATALISTENER_H


#include <e32std.h>
#include <e32property.h>
#include "bmbplugin.h"



class CBmbCmdListener : public CActive
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CBmbCmdListener* NewL(CBmbPlugin& aParent);

        /**
        * Destructor.
        */
        ~CBmbCmdListener();
		
        /**
        * Be informed of the completion of command handling.
        * 
        * @param aResp the response to be sent to client who
        *             request this command handling.
        */
        void HandlingDataCompleted( const TDesC8& aResp );
        
    private:
        /**
        * C++ default constructor.
        */
        CBmbCmdListener(CBmbPlugin& aParent);
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
        /**
        * Starts listening AT commands published by btmoncmdhandler.
        * @param None
        * @return None
        */
        void Subscribe();
        
    private:
        /**
        * From CActive. Called when asynchronous request completes.
        * @param None
        * @return None
        */
	    void RunL();

        /**
        * From CActive. Cancels asynchronous request.
        * @param None
        * @return None
        */
	    void DoCancel();

    private:
	
        CBmbPlugin& iParent;
        
        // for receiving AT commands from btmonocmdhandler
		RProperty iATCmdProperty;
		
        // for sending AT responses to btmonocmdhandler
        RProperty iATRespProperty;

		// At command buffer
		TBuf8<KMaxATSize> iAtCmdBuf;
    };

#endif
