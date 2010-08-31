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
* Description:  Implementation of CBIPXMLWriter class
*
*/


// INCLUDE FILES
#include "BIPXMLWriter.h"
#include <obexutilsmessagehandler.h>


// CONSTANTS
_LIT8( KBIPXmlDocBegin," <imaging-capabilities version=\"1.0\">" );
_LIT8( KBIPXmlDocEnd, "</imaging-capabilities>" );
_LIT8( KBIPXmlImageFormatsBegin, "<image-formats encoding=\"" );
_LIT8( KBIPXmlImageFormatsEnd, "/>");
_LIT8( KBIPImageTypes, "JPEGBMPGIFWBMPPNGJPEG2000" );
_LIT8( KBIPUserSeries60, "USR-SERIES60-" );

const TInt KBIPImageTypesLength     = 30;
// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBIPXmlWriter* CBIPXmlWriter::NewL()
    {
    TRACE_FUNC
    CBIPXmlWriter* self = new ( ELeave ) CBIPXmlWriter();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return( self );
    }

// ---------------------------------------------------------
// CBIPXmlWriter()
// ---------------------------------------------------------
//
CBIPXmlWriter::CBIPXmlWriter()
    {
    }

// ---------------------------------------------------------
// OpenXmlDocumentL()
// ---------------------------------------------------------
//
void CBIPXmlWriter::OpenXmlDocumentL( TFileName& aFileName )
    {
    TRACE_FUNC
    User::LeaveIfError( iFileSession.Connect() );    
    TPath tempPath;        
    TInt drive = TObexUtilsMessageHandler::GetMessageCentreDriveL();    
    TDriveUnit driveString (drive);
    User::LeaveIfError(iFileSession.CreatePrivatePath( drive ));
    User::LeaveIfError(iFileSession.PrivatePath(tempPath));                    
	User::LeaveIfError(ifile.Temp( iFileSession, tempPath, aFileName, EFileWrite ));   
    User::LeaveIfError( ifile.Write( KBIPXmlDocBegin ) );
    }

// ---------------------------------------------------------
// CloseXmlDocumentL()
// ---------------------------------------------------------
//
void CBIPXmlWriter::CloseXmlDocumentL()
    {
    TRACE_FUNC
    User::LeaveIfError( ifile.Write( KBIPXmlDocEnd ) );
    User::LeaveIfError( ifile.Flush() );
    ifile.Close();
    iFileSession.Close();
    }

// ---------------------------------------------------------
// OpenXmlElementL()
// ---------------------------------------------------------
//
void CBIPXmlWriter::OpenXmlElementL( TElementType aElement )
    {
    switch( aElement )
        {
        case EImageFormats:
            {
            User::LeaveIfError( ifile.Write( KBIPXmlImageFormatsBegin ) );
            break;
            }
        case EPreferredFormat:
            {
            break;
            }
        case EAttachmentFormats:
            {
            break;
            }
        case EFilteringParameters:
            {
            break;
            }
        case EDPOFOptions:
            {
            break;
            }
        default:
            {
            }
        }
    TRACE_FUNC
    }

// ---------------------------------------------------------
// AddXmlAttributeL()
// ---------------------------------------------------------
//
void CBIPXmlWriter::AddXmlAttributeL( TAttributeType aAttributeType, TDesC8& aAttr )
    {
    TRACE_FUNC_ENTRY    
    TBuf8<KBIPImageTypesLength> attribute = KBIPImageTypes();
    switch( aAttributeType )
        {
        case EEncoding:
            {
            if( attribute.Find( aAttr ) == KErrNotFound )
                {
                User::LeaveIfError( ifile.Write( KBIPUserSeries60 ) );
                User::LeaveIfError( ifile.Write( aAttr ) );
                
                }
            else
                {
                User::LeaveIfError( ifile.Write( aAttr ) );
                }
            break;
            }
        case EPixel:
            {
            User::LeaveIfError( ifile.Write( aAttr ) );
            break;
            }
        case EMaxSize:
            {
            User::LeaveIfError( ifile.Write( aAttr ) );
            break;
            }
        case ETransformation:
            {
            break;
            }
        default:
            {
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CloseXmlElementL()
// ---------------------------------------------------------
//
void CBIPXmlWriter::CloseXmlElementL()
    {
    TRACE_FUNC
    User::LeaveIfError( ifile.Write( KBIPXmlImageFormatsEnd ) );
    }

// ---------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------
//
void CBIPXmlWriter::ConstructL()
    {
    TRACE_FUNC
    }

// ---------------------------------------------------------
// ~CBIPXmlWriter()
// ---------------------------------------------------------
//
CBIPXmlWriter::~CBIPXmlWriter()
    {
    TRACE_FUNC
    ifile.Close();
    iFileSession.Close();
    }

//  End of File
