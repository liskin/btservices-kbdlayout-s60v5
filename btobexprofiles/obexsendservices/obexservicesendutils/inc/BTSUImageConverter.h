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
* Description:  Image conversion class
*
*/


#ifndef BTSU_IMAGE_CONVERTER_H
#define BTSU_IMAGE_CONVERTER_H

// INCLUDES
#include <f32file.h>
#include <badesca.h>

// CONSTANTS

// FORWARD DECLARATIONS
class CImageDecoder;
class CImageEncoder;
class CFrameImageData;
class CBitmapScaler;
class CFbsBitmap;

// CLASS DECLARATION

/**
* A class for executing simple image conversion tasks.
*/
NONSHARABLE_CLASS (CBTSUImageConverter) : public CActive
    {

    public:  // Constructor and destructor

        /**
        * Two-phased constructor.        
        * @param None.
        */
        static CBTSUImageConverter* NewL();

        /**
        * Destructor.
        */
        ~CBTSUImageConverter();

    public: // New functions        

        /**
        * Returns the pixel size of the given image.
        * @param aImageFile A file handle of image file.
        * @return The pixel size.
        */
        TSize GetImageSizeL( RFile&  aImageFile );        

        /**
        * Create a thumbnail version of the source image.
        * @param aSourceFile A filehandle of sourcefile.
        * @param aDestFile   Full path for the destination file.
        * @return None.
        */
        void CreateThumbnailL( RFile& aSourceFile, 
                               const TDesC& aDestFile );

        /**
        * Finds a display name for the given mime type.
        * @param aMimeType Mime type
        * @return A pointer to the name.
        */
        static HBufC* GetDisplayNameL( const TDesC8& aMimeType );

    private: // Functions from base classes

        /**
        * From CActive Get's called when a request is cancelled.
        * @return None.
        */
        void DoCancel();

        /**
        * From CActive Get's called when a request is completed.
        * @return None.
        */
        void RunL();
    
    private:

        /**
        * C++ default constructor.
        */
        CBTSUImageConverter();

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

        /**
        * Decode image.
        * @param aSourceFile A filehandle of source file.
        * @return None
        */
        void DecodeImageL( RFile& aSourceFile );

        /**
        * Encode image.
        * @param aDestFile Full path for the destination file.
        * @param aThumbnail ETrue enables thumbnail creation
        * @return None
        */
        void EncodeImageL( const TDesC& aDestFile,
                           const TBool& aThumbnail = EFalse );

        /**
        * Scale image.
        * @return None
        */
        void ScaleImageL();

        /**
        * Reset internal state.
        * @return None.
        */
        void Reset();

    private: // Data

        CImageDecoder*              iDecoder;
        CImageEncoder*              iEncoder;
        CBitmapScaler*              iScaler;

        CFrameImageData*            iFrameImageData;
        CFbsBitmap*                 iFrameBitmap;
        CFbsBitmap*                 iScaledBitmap;
        RFs                         iFileSession;

        CActiveSchedulerWait        iWait;
    };


#endif // BTSU_IMAGE_CONVERTER_H
