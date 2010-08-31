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
* Description:  XML writer class declaration.
*
*/


#ifndef _BIPXMLWRITER_H
#define _BIPXMLWRITER_H

//INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "debug.h"

// CONSTANTS
enum TElementType
    {
    EImageFormats,
    EPreferredFormat,
    EAttachmentFormats,
    EFilteringParameters,
    EDPOFOptions
    };

enum TAttributeType
    {
    EEncoding,
    EPixel,
    ETransformation,
    EMaxSize
    };

// CLASS DECLARATION

/**
* XML writer for BIP capability object
*
*/
NONSHARABLE_CLASS (CBIPXmlWriter) : public CBase
    {
    public: //Constructors and destructors

        /**
        * Two-phased constructor.
        */
        static CBIPXmlWriter* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBIPXmlWriter();

    public:     //New Functions               

		/**
        * Open temporary file
        * @param    TFileName       Contains the new temp file name.
        * @return                   None.
        */
        void OpenXmlDocumentL( TFileName& aFileName);
    
		/**
        * Close temporary file. So that it can be used by clients
        * @param                    None.
        * @return                   None.
        */
        void CloseXmlDocumentL();
    
		/**
        * Opens xml element for writing
        * @param    TElementType    Element to be written to file
        * @return                   None.
        */
        void OpenXmlElementL( TElementType aElement );

		/**
        * Close xml element 
        * @param                    None
        * @return                   None.
        */
        void CloseXmlElementL( );

		/**
        * Add xml attribute to the open element
        * @param    TAttributeType  Type of the attribute
        * @param    TDesC8          Attribute
        * @return                   None.
        */
        void AddXmlAttributeL( TAttributeType aAttributeType, TDesC8& aAttr );

    private:

        /**
        * C++ default constructor.
        */
        CBIPXmlWriter();

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

    private:    // Data
        
        RFile       ifile;
        RFs         iFileSession;
        TFileName   iFileName;
    };

#endif      //_BIPXMLWRITER_H

// End of File
