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
* Description:  Bip image handler class declaration.
*
*/


#ifndef _BIPIMAGEHANDLER_H
#define _BIPIMAGEHANDLER_H

//INCLUDES
#include <e32base.h>
#include <obex.h>
#include "debug.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Generates image handles and sets them to obex response packets.
*/
NONSHARABLE_CLASS (CBIPImageHandler) : public CBase
    {
    public: //Constructors and destructors

        /**
        * Two-phased constructor.
        */
        static CBIPImageHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBIPImageHandler();

    public:     //New Functions

		/**
        * Adds Image handle to Obex response packet
        * @param    aObexServer         Current obex server
        * @return                       None.
        */
        void AddImageHandleHeaderL( CObexServer* aObexServer );

    private:

        /**
        * C++ default constructor.
        */
        CBIPImageHandler();

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();
    
    private:    // Data
        TUint           iImageHandleValue;
        
    };

#endif      //_BIPIMAGEHANDLER_H

// End of File
