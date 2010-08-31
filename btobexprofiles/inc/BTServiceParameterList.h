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
* Description:  Parameter list
*
*/


#ifndef BT_SERVICE_PARAMETER_LIST_H
#define BT_SERVICE_PARAMETER_LIST_H

// INCLUDES
#include <e32std.h>
#include <badesca.h>    // CDesCArray
#include <f32file.h>

// CONSTANTS

// DATA TYPES

struct TBTSUXhtmlParam
    {
    TDesC* iFileName;
    CDesCArray* iRefObjectList;
    };

struct TBTSUImageParam
    {    
    RFile   iFile; 
    TDesC*  iDisplayName;
    TDesC8* iMimeType;
    TSize   iPixelSize;
    TInt    iFileSize;    
    TBool   iSend;
    
    };
    

// CLASS DECLARATION

/**
*  A class holding paths to different types of files. 
*
*  In case of an XHTML file the list collects paths 
*  to objects referenced in the file.
*
*  In case of an image the list resolves the MIME types and
*  displayable names of the images.
*
*  @lib BtServiceUtils.lib
*  @since Series 60 2.6
*/
class CBTServiceParameterList : public CBase
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CBTServiceParameterList* NewL();
        IMPORT_C static CBTServiceParameterList* NewLC();
        
        /**
        * Destructor.
        */
        virtual ~CBTServiceParameterList();

    public: // New functions

        /**
        * Adds an xhtml file path to the list.
        * @since Series 60 2.6
        * @param aFilePath A full path and file name.
        * @return None.
        */
        IMPORT_C void AddXhtmlL( const TDesC& aFilePath );

        /**
        * Adds an image file path to the list.
        * @since Series 60 2.6
        * @param aFilePath A full path and file name.
        * @return None.
        */
        IMPORT_C void AddImageL( const TDesC& aFilePath );
        
        /**
        * Adds an image file path to the list.
        * @since Series 60 2.6
        * @param aFile file handle to be send.
        * @return None.
        */
        IMPORT_C void AddImageL( RFile aFile );

        /**
        * Adds an object file path to the list.
        * @since Series 60 2.6
        * @param aFilePath A full path and file name.
        * @return None.
        */
        IMPORT_C void AddObjectL( const TDesC& aFilePath );
        
        /**
        * Adds an object file path to the list.
        * @since Series 60 2.6
        * @param aFile file handle to be send .
        * @return None.
        */
        IMPORT_C void AddObjectL( RFile aFile );

    public: // New functions (not exported)
        
        /**
        * Return the number of xhtml file paths in the list.
        * @return The count.
        */
        TInt XhtmlCount() const;

        /**
        * Return the number of image file paths in the list.
        * @return The count.
        */
        TInt ImageCount() const;

        /**
        * Return the number of object file paths in the list.
        * @return The count.
        */
        TInt ObjectCount() const;
        
        /**
        * Returns a copy of an element of the list from the given index.
        * @param aIndex The index.
        * @return The element.
        */
        TBTSUXhtmlParam XhtmlAt( TInt aIndex ) const;

        /**
        * Returns a copy of an element of the list from the given index.
        * @param aIndex The index.
        * @return The element.
        */
        TBTSUImageParam ImageAtL( TInt aIndex ) const;

        /**
        * Returns a copy of an element of the list from the given index.
        * @param aIndex The index.
        * @return The element.
        */
        RFile& ObjectAtL( TInt aIndex );

        /**
        * Checks whether the list has any xhtml-files with referenced objects.
        * @return A boolean according to result.
        */
        TBool HasAnyReferencedObjects() const;

        /**
        * Return the size of objects in the list.
        * @return The count.
        */
        TInt ObjectListSizeL() const;
        
        /**
        * Return the size of images in the list.
        * @return The count.
        */
        
        TInt ImageListSize() const;
        
        /**
        * Remove image from list.
        * @param aIndex The index.
        * @return None.
        */
        void RemoveImageL(TInt aIndex);
        
        /**
        * Return count of removed image        
        * @return The count of removed images.
        */
        TInt RemovedImageCount();
        
        /**
        * Reset image filehandle
        * @param aIndex The index.
        * @return None.
        */
        void ResetImageHandleL(TInt aIndex);
        
        /**
        * Reset object filehandle
        * @param aIndex The index.
        * @return None.
        */
        void ResetObjectHandleL(TInt aIndex);  
        
        
        /**
        * Reset image filehandle
        * @param aIndex The index.
        * @return None.
        */
        void MarkAsSendL(TInt aIndex);        
                

    private:

        /**
        * C++ default constructor.
        */
        CBTServiceParameterList();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private: // Data

        RArray<TBTSUXhtmlParam> iXhtmlList;
        RArray<TBTSUImageParam> iImageList;
        RArray<RFile> iObjectList;
        RFs iFileSession;        
        TInt iRemovedImageCount;
    };

#endif      // BT_SERVICE_PARAMETER_LIST_H
            
// End of File
