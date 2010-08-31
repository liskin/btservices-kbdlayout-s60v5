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
* Description:  Capability object handler class declaration
*
*/


#ifndef _BIPCAPABILITYHANDLER_H
#define _BIPCAPABILITYHANDLER_H

//INCLUDES
#include <e32base.h>
#include <badesca.h>
#include "debug.h"

// FORWARD DECLARATIONS
class CBIPXmlWriter;
class CImageTypeDescription;

// CLASS DECLARATION

/**
*  Creates an xml capability object
*/
NONSHARABLE_CLASS (CBIPCapabilityHandler) : public CBase
    {
    public: //Constructors and destructors
        /**
        * Two-phased constructor.
        */
        static CBIPCapabilityHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBIPCapabilityHandler();

    public:     //New Functions
                      
        /**
        * Creates an array of supported image types.
        * @param    aFileName       On return contains the name
        *                           of the file that was created
        * @return                   None.
        */
        void CreateCapabilityObjectL( TFileName& aFileName );
        
        /**
        * Creates an array of supported image types.
        * @param                    None.
        * @return                   None.
        */
        void SupportedImageTypesL( );

    private:

        /**
        * C++ default constructor.
        */
        CBIPCapabilityHandler();

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();
    
    private:    // Data
        CBIPXmlWriter*          iBIPXmlWriter;
        CDesC8ArrayFlat*        iImageTypeArray;
    };

#endif      //_BIPCAPABILITYHANDLER_H

// End of File
