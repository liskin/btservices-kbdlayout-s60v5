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
* Description:  Tools for image conversion.
*
*/


// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTSUImageConverter.h"
#include "BTSUDebug.h"

#include <imageconversion.h>
#include <bitmaptransforms.h>

//CONSTANTS
_LIT8( KBTSUImageTypeJpeg, "image/jpeg" );
const TInt KBTSUJpegQualityFactor = 90;
const TInt KBTSUImageThumbWidth   = 160;  
const TInt KBTSUImageThumbHeight  = 120;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSUImageConverter::CBTSUImageConverter
// Constructor.
// -----------------------------------------------------------------------------
//
CBTSUImageConverter::CBTSUImageConverter(): CActive( EPriorityStandard )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CBTSUImageConverter::~CBTSUImageConverter()
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::~CBTSUImageConverter()"));

    Cancel();
    Reset();
    iFileSession.Close();

    FLOG(_L("[BTSU]\t CBTSUImageConverter::~CBTSUImageConverter() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSUImageConverter* CBTSUImageConverter::NewL()
    {
    CBTSUImageConverter* self = new (ELeave) CBTSUImageConverter;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::ConstructL()"));
 
    User::LeaveIfError( iFileSession.Connect() );
    iFileSession.ShareProtected();

    FLOG(_L("[BTSU]\t CBTSUImageConverter::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::GetImageSizeL
// -----------------------------------------------------------------------------
//
TSize CBTSUImageConverter::GetImageSizeL( RFile& aImageFile )
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::GetImageSizeL()"));
   
    CImageDecoder* decoder = CImageDecoder::FileNewL(aImageFile,ContentAccess::EPeek );
    TSize size = decoder->FrameInfo().iOverallSizeInPixels;
    delete decoder;

    FTRACE(FPrint(_L("[BTSU]\t CBTSUImageConverter::GetImageSizeL() completed w=%d h=%d"), size.iWidth, size.iHeight ));

    return size;
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::CreateThumbnailL
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::CreateThumbnailL( RFile& aSourceFile, 
                                            const TDesC& aDestFile )
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::CreateThumbnailL()"));
    

    Reset();
    DecodeImageL( aSourceFile );
    ScaleImageL();
    EncodeImageL( aDestFile, ETrue );
    Reset();

    FLOG(_L("[BTSU]\t CBTSUImageConverter::CreateThumbnailL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::GetDisplayNameL
// -----------------------------------------------------------------------------
//
HBufC* CBTSUImageConverter::GetDisplayNameL( const TDesC8& aMimeType )
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::GetDisplayNameL()"));

    if ( &aMimeType == NULL || 
         aMimeType.Length() == 0 || 
         aMimeType.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        }

    HBufC* name = NULL;

    // Get file types from decoder
    //
    RFileExtensionMIMETypeArray fileTypes;
    CleanupResetAndDestroyPushL( fileTypes );
    CImageDecoder::GetFileTypesL( fileTypes );
    
    // Find the display name for the mime type
    //
    for( TInt index = 0; index < fileTypes.Count(); index++ )
        {
        if ( fileTypes[index]->MIMEType() == aMimeType )
            {
            name = fileTypes[index]->DisplayName().AllocL();
            break;
            }
        }

    CleanupStack::PopAndDestroy(); // fileTypes 

    if ( name == NULL )
        {
        // No display name found, so the image type is not supported.
        //
        User::Leave( KErrNotSupported );
        }

    FLOG(_L("[BTSU]\t CBTSUImageConverter::GetDisplayNameL() completed"));

    return name;
    }


// -----------------------------------------------------------------------------
// CBTSUImageConverter::DoCancel
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::DoCancel()
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::DoCancel()"));

    if ( iDecoder )
        {
        iDecoder->Cancel();
        }
    if ( iScaler )
        {
        iScaler->Cancel();
        }
    if ( iEncoder )
        {
        iEncoder->Cancel();
        }

    FLOG(_L("[BTSU]\t CBTSUImageConverter::DoCancel() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::RunL
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::RunL()
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTSUImageConverter::RunL() %d"), iStatus.Int() ));

    iWait.AsyncStop();

    FLOG(_L("[BTSU]\t CBTSUImageConverter::RunL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::DecodeImageL
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::DecodeImageL( RFile& aSourceFile )
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::DecodeImageL( )"));

    __ASSERT_DEBUG( iDecoder == NULL, BTSUPanic( EBTSUPanicExistingObject ) );
    __ASSERT_DEBUG( iFrameBitmap == NULL, BTSUPanic( EBTSUPanicExistingObject ) );

    // Create decoder
    //    
    iDecoder = CImageDecoder::FileNewL(aSourceFile, ContentAccess::EPeek );

    // Create a bitmap
    //
    iFrameBitmap = new ( ELeave ) CFbsBitmap;
    User::LeaveIfError( iFrameBitmap->Create( 
        iDecoder->FrameInfo().iOverallSizeInPixels,
        iDecoder->FrameInfo().iFrameDisplayMode ) );
    
    // Start decoding
    //
    iDecoder->Convert( &iStatus, *iFrameBitmap );
    SetActive();

    iWait.Start(); // Wait here until decoding is completed
    User::LeaveIfError( iStatus.Int() );

    FLOG(_L("[BTSU]\t CBTSUImageConverter::DecodeImageL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::EncodeImageL
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::EncodeImageL( const TDesC& aDestFile, 
                                        const TBool& aThumbnail )
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::EncodeImageL()"));

    __ASSERT_DEBUG( iDecoder != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
    __ASSERT_DEBUG( iEncoder == NULL, BTSUPanic( EBTSUPanicExistingObject ) );
    __ASSERT_DEBUG( iFrameImageData == NULL, BTSUPanic( EBTSUPanicExistingObject ) );

    // Create encoder
    //
    iEncoder = CImageEncoder::FileNewL( iFileSession, aDestFile, KBTSUImageTypeJpeg() );

    // Create frame image data
    //
    iFrameImageData = CFrameImageData::NewL();
    TJpegImageData* jpegFormat = new ( ELeave ) TJpegImageData;
    CleanupStack::PushL( jpegFormat );

    jpegFormat->iSampleScheme = TJpegImageData::EColor422;
    jpegFormat->iQualityFactor = KBTSUJpegQualityFactor;
    User::LeaveIfError( iFrameImageData->AppendImageData( jpegFormat ) );
    CleanupStack::Pop( jpegFormat );

    // Start encoding
    //
    if ( aThumbnail )
        {
        __ASSERT_DEBUG( iScaledBitmap != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
        iEncoder->Convert( &iStatus, *iScaledBitmap, iFrameImageData );
        }
    else
        {
        __ASSERT_DEBUG( iFrameBitmap != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
        iEncoder->Convert( &iStatus, *iFrameBitmap, iFrameImageData );
        }
    
    SetActive();
    iWait.Start(); // Wait here until encoding is completed
    User::LeaveIfError( iStatus.Int() );

    FLOG(_L("[BTSU]\t CBTSUImageConverter::EncodeImageL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::ScaleImageL
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::ScaleImageL()
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::ScaleImageL( )"));

    __ASSERT_DEBUG( iDecoder != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
    __ASSERT_DEBUG( iFrameBitmap != NULL, BTSUPanic( EBTSUPanicNullPointer ) );
    __ASSERT_DEBUG( iScaler == NULL, BTSUPanic( EBTSUPanicExistingObject ) );
    __ASSERT_DEBUG( iScaledBitmap == NULL, BTSUPanic( EBTSUPanicExistingObject ) );

    // Create scaler
    //
    iScaler = CBitmapScaler::NewL();

    // Create a bitmap
    //
    iScaledBitmap = new ( ELeave ) CFbsBitmap;
    TSize size;
    size.iWidth = KBTSUImageThumbWidth;
    size.iHeight = KBTSUImageThumbHeight;
    User::LeaveIfError( iScaledBitmap->Create( size,
        iDecoder->FrameInfo().iFrameDisplayMode ) );
    
    // Start scaling
    //
    iScaler->Scale( &iStatus, *iFrameBitmap, *iScaledBitmap, EFalse );
    SetActive();

    iWait.Start(); // Wait here until scaling is completed
    User::LeaveIfError( iStatus.Int() );

    FLOG(_L("[BTSU]\t CBTSUImageConverter::ScaleImageL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUImageConverter::Reset
// -----------------------------------------------------------------------------
//
void CBTSUImageConverter::Reset()
    {
    FLOG(_L("[BTSU]\t CBTSUImageConverter::Reset()"));

    delete iDecoder; iDecoder = NULL;
    delete iEncoder; iEncoder = NULL;
    delete iScaler; iScaler = NULL;
    
    delete iFrameImageData; iFrameImageData = NULL;
    delete iFrameBitmap; iFrameBitmap = NULL;
    delete iScaledBitmap; iScaledBitmap = NULL;

    FLOG(_L("[BTSU]\t CBTSUImageConverter::Reset() completed"));
    }

//  End of File
