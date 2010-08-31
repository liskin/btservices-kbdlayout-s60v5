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
* Description:  Implementation of CBIPCapabilityHandler
*
*/


// INCLUDE FILES
#include "BIPCapabilityHandler.h"
#include "BIPXMLWriter.h"
#include <imageconversion.h>

// CONSTANTS
_LIT8( KBIPXmlImagePixel, "\" pixel=\"0*0-65535*65535\"");
_LIT8( KBIPXmlImageMaxSize, " maxsize=\"50000000\"");

const TInt KBIPMaxTypeLenght = 256;
const TInt KBIPMaxAttrLength = 32;

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBIPCapabilityHandler* CBIPCapabilityHandler::NewL()
    {
    TRACE_FUNC_ENTRY
    CBIPCapabilityHandler* self = new ( ELeave ) CBIPCapabilityHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return( self );
    }

// ---------------------------------------------------------
// CBIPCapabilityHandler()
// ---------------------------------------------------------
//
CBIPCapabilityHandler::CBIPCapabilityHandler()
    {
    TRACE_FUNC_ENTRY
    }

// ---------------------------------------------------------
// CreateCapabilityObjectL()
// ---------------------------------------------------------
//
void CBIPCapabilityHandler::CreateCapabilityObjectL( TFileName& aFileName )
    {
    TRACE_FUNC_ENTRY
    iBIPXmlWriter = CBIPXmlWriter::NewL();
    iBIPXmlWriter->OpenXmlDocumentL( aFileName );
    SupportedImageTypesL();
    TBufC8<KBIPMaxAttrLength> pixelRange( KBIPXmlImagePixel );
    TBufC8<KBIPMaxAttrLength> maxSize( KBIPXmlImageMaxSize );
    TBufC8<KBIPMaxAttrLength> encoding;
    for( TInt index = 0; index < iImageTypeArray->Count(); index++ )
        {
        encoding = iImageTypeArray->MdcaPoint( index );
        iBIPXmlWriter->OpenXmlElementL( EImageFormats );
        iBIPXmlWriter->AddXmlAttributeL( EEncoding, encoding );
        iBIPXmlWriter->AddXmlAttributeL( EPixel, pixelRange );
        iBIPXmlWriter->AddXmlAttributeL( EMaxSize, maxSize );
        iBIPXmlWriter->CloseXmlElementL();
        }
    iBIPXmlWriter->CloseXmlDocumentL();
    TRACE_FUNC_EXIT
    }    

// ---------------------------------------------------------
// SupportedImageTypesL()
// ---------------------------------------------------------
//
void CBIPCapabilityHandler::SupportedImageTypesL( )
    {   
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNone;
	RFileExtensionMIMETypeArray theFileExtensionArray;
	iImageTypeArray->Reset();

    TRAP( retVal, CImageDecoder::GetFileTypesL( theFileExtensionArray ) );	
	if( retVal != KErrNone )
		{
		theFileExtensionArray.ResetAndDestroy();
        User::Leave( retVal );
		}
    
    HBufC8* name = HBufC8::NewLC( KBIPMaxTypeLenght );
	for( TInt index = 0; index < theFileExtensionArray.Count(); index++ )
		{
		CFileExtensionMIMEType& fileExtAndMIMEType = *theFileExtensionArray[index];
        name->Des().Copy( fileExtAndMIMEType.DisplayName() );

        TInt dummy;
        retVal = iImageTypeArray->Find( name->Des(), dummy );
        if( retVal != KErrNone )
            {
            iImageTypeArray->AppendL( name->Des() );
            }
        name->Des().Zero();
        }
    CleanupStack::PopAndDestroy(name);  

	theFileExtensionArray.ResetAndDestroy();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------
//
void CBIPCapabilityHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iImageTypeArray = new (ELeave) CDesC8ArrayFlat(1);
    }

// ---------------------------------------------------------
// ~CBIPCapabilityHandler()
// ---------------------------------------------------------
//
CBIPCapabilityHandler::~CBIPCapabilityHandler()
    {
    TRACE_FUNC
    delete iBIPXmlWriter;
    delete iImageTypeArray;
    }

//  End of File
