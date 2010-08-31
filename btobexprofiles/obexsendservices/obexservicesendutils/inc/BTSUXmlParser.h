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
* Description:  XML parser
*
*/


#ifndef BTSU_XML_PARSER_H
#define BTSU_XML_PARSER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <badesca.h>
//#include <RXMLReader.h>
#include <f32file.h>
#include <xml/parser.h>
 
#include "BTServiceUtils.h"

// CLASS DECLARATION

/**
*  A class for executing simple predetermined xml parsing tasks.
*/
NONSHARABLE_CLASS (CBTSUXmlParser) : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSUXmlParser* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBTSUXmlParser();

     public: // New functions
        
        /**
        * Finds and collects image capabilites from the file.
        * @param aFileName The name of the file to be parsed.
        * @return A pointer to the list.
        */
        CDesCArrayFlat* GetCapabilityListL( const TDesC& aFileName );

		
        /**
        * Finds and collects image capabilites from the file.
        * @param aFileName The name of the file to be parsed.
        * @return A pointer to the list.
        */        
        RArray<TBTSUImageCap>* GetImgCapabilityListL( const TDesC& aFileName );

        /**
        * Finds and collects paths to objects referenced in the file.
        * @param aFileName The name of the file to be parsed.
        * @return A pointer to the list.
        */
        CDesCArrayFlat* GetRefObjectListL( const TDesC& aFileName );

    private:

        /**
        * C++ default constructor.
        */
        CBTSUXmlParser();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

     private: // Data

        RFs        iFileSession;
    };

#endif      // BTSU_XML_PARSER_H
            
// End of File
