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
* Description:  parameter list implementation
*
*/


// INCLUDE FILES
#include "BTServiceUtils.h"
#include "BTServiceParameterList.h"
#include "BTSUImageConverter.h"
#include "BTSUXmlParser.h"
#include "BTSUDebug.h"

#include <apgcli.h> // RApaLSession
#include <apmstd.h> // TDataType
#include <imageconversion.h> //´Type solving
#include <caf/content.h>

// CONSTANTS

_LIT( KBIPImageTypes, "JPEGBMPGIFWBMPPNGJPEG2000" );


const TInt KBIPImageTypesLength     = 30;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTServiceParameterList::CBTServiceParameterList
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTServiceParameterList::CBTServiceParameterList()
    {
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTServiceParameterList::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::ConstructL()"));    
    iRemovedImageCount=0;
    User::LeaveIfError( iFileSession.Connect() );
    iFileSession.ShareProtected();

    FLOG(_L("[BTSU]\t CBTServiceParameterList::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CBTServiceParameterList* CBTServiceParameterList::NewL()
    {
    CBTServiceParameterList* self = new( ELeave ) CBTServiceParameterList;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CBTServiceParameterList* CBTServiceParameterList::NewLC()
    {
    CBTServiceParameterList* self = new( ELeave ) CBTServiceParameterList;
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// Destructor
CBTServiceParameterList::~CBTServiceParameterList()
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::~CBTServiceParameterList()"));

    TInt index = 0;

    for ( ; index < iXhtmlList.Count(); index++ )
        {
        delete iXhtmlList[index].iFileName;
        delete iXhtmlList[index].iRefObjectList;
        }

    for ( index = 0; index < iImageList.Count(); index++ )
        {  
                   
        delete iImageList[index].iMimeType;
        delete iImageList[index].iDisplayName;
        if(iImageList[index].iFile.SubSessionHandle())
            {
            iImageList[index].iFile.Close();
            }
        
        }
        
    for ( index = 0; index < iObjectList.Count(); index++ )
    	{
    	if(iObjectList[index].SubSessionHandle())
    		{
    		iObjectList[index].Close();
    		}
    	}

    iXhtmlList.Close();
    iImageList.Close();    
    iObjectList.Close();
    iFileSession.Close();
    
    FLOG(_L("[BTSU]\t CBTServiceParameterList::~CBTServiceParameterList() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::AddXhtmlL
// -----------------------------------------------------------------------------
//
EXPORT_C void CBTServiceParameterList::AddXhtmlL( const TDesC& aFilePath )
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddXhtmlL()"));

    if ( &aFilePath == NULL || 
         aFilePath.Length() == 0 ||
         aFilePath.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        }

    // Allocate memory for filename
    //
    HBufC* file = aFilePath.AllocL();
    CleanupStack::PushL( file );

    // Resolve referenced objects
    //
    CBTSUXmlParser* parser = CBTSUXmlParser::NewL();
    CleanupStack::PushL( parser );

    CDesCArrayFlat* list = parser->GetRefObjectListL( aFilePath );
    CleanupStack::PushL( list );

    // Store parameter into list
    //
    TBTSUXhtmlParam param;
    param.iFileName = file;
    param.iRefObjectList = list;
    User::LeaveIfError( iXhtmlList.Append( param ) );

    CleanupStack::Pop( 3 ); // list, parser, file
    delete parser;

    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddXhtmlL() completed"));
    }    

// -----------------------------------------------------------------------------
// CBTServiceParameterList::AddImageL
// -----------------------------------------------------------------------------
//
EXPORT_C void CBTServiceParameterList::AddImageL( const TDesC& aFilePath )
    {   
    if ( &aFilePath == NULL || 
         aFilePath.Length() == 0 ||
         aFilePath.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        } 
    RFile file;
    User::LeaveIfError( file.Open( iFileSession, aFilePath, EFileShareReadersOnly) );
    AddImageL(file);
    file.Close();
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::AddImageL
// -----------------------------------------------------------------------------
//
EXPORT_C void CBTServiceParameterList::AddImageL( RFile aFile )
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddImageL()"));
	TBool found;
	TInt  retVal;
	TBool isprotected = EFalse;
   
    //chekc that file handle is correct
    if(!aFile.SubSessionHandle())
        {
        User::Leave( KErrArgument );
        }
        
    //check if files are drm protected 
        
	ContentAccess::CContent* drm = ContentAccess::CContent::NewL(aFile);
    drm->GetAttribute(ContentAccess::EIsProtected, isprotected);
    delete drm;       
    
    
    if(isprotected)
   		{
   		User::Leave( KErrNotSupported );
   		}
    

    CBTSUImageConverter * imageConverter = CBTSUImageConverter::NewL();
    CleanupStack::PushL( imageConverter );

    // Allocate memory for filename
    //

    
    // Resolve MIME type
    //
    RApaLsSession session;
    HBufC8* mimeType = NULL;
    TDataType type;
    
    TUid uid;

    User::LeaveIfError( session.Connect() );
    CleanupClosePushL( session );
   
    User::LeaveIfError( session.AppForDocument( aFile, uid, type ) );
    mimeType = type.Des8().AllocL();
      
    CleanupStack::Pop(); // session
    session.Close();   
    
    CleanupStack::PushL( mimeType );

    // Resolve Display name
    //    
    HBufC* displayName = imageConverter->GetDisplayNameL( *mimeType );
    CleanupStack::PushL( displayName );
    
    RFileExtensionMIMETypeArray fileExtensions;
    
    //Resolve what format are suported
    TRAP( retVal, CImageDecoder::GetFileTypesL( fileExtensions ) );	
	if( retVal != KErrNone )
		{
		fileExtensions.ResetAndDestroy();
        User::Leave( retVal );
		}
		
	found=EFalse;
	TBuf<KBIPImageTypesLength> types = KBIPImageTypes();
	
	for( TInt index = 0; index < fileExtensions.Count(); index++ )
		{
		retVal=displayName->Compare((*fileExtensions[index]).DisplayName());		
		if(retVal==0 &&  types.Find((*fileExtensions[index]).DisplayName())!=KErrNotFound )
			{
			found=ETrue;		
			}
		}		
	fileExtensions.ResetAndDestroy();	
	if(found==EFalse)
		{
		User::Leave(KErrNotSupported);
		}
	
		
    // Resolve file size
    //
    TInt fileSize = 0;
    User::LeaveIfError( aFile.Size( fileSize ) );
          	
	
    // Resolve width and height
    //    
    TSize decodeSize = imageConverter->GetImageSizeL( aFile );
    
    
    // Store parameter into list
    //
    TBTSUImageParam param;
    
    param.iFile.Duplicate(aFile);
    param.iMimeType = mimeType;
    param.iDisplayName = displayName;
    param.iFileSize = fileSize;
    param.iPixelSize = decodeSize;    
    param.iSend=EFalse;
    
    
    User::LeaveIfError( iImageList.Append( param ) );

    CleanupStack::Pop( 3 ); // displayName, mimeType, imageConverter
    delete imageConverter;    

    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddImageL() completed"));
	}

// -----------------------------------------------------------------------------
// CBTServiceParameterList::AddObjectL
// -----------------------------------------------------------------------------
//
EXPORT_C void CBTServiceParameterList::AddObjectL( const TDesC& aFilePath )
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddObjectL()"));
    
    if ( &aFilePath == NULL || 
         aFilePath.Length() == 0 ||
         aFilePath.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        }

    RFile file;
    User::LeaveIfError( file.Open( iFileSession, aFilePath, EFileShareReadersOnly) );
    AddObjectL(file);
    file.Close();
    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddObjectL() completed"));
    }
    
 // -----------------------------------------------------------------------------
// CBTServiceParameterList::AddObjectL
// -----------------------------------------------------------------------------
//   
 EXPORT_C void CBTServiceParameterList::AddObjectL( RFile aFile )
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddObjectL()"));
    
        //chekc that file handle is correct
    if(!aFile.SubSessionHandle())
        {
        User::Leave( KErrArgument );
        }
    RFile file;
    
    file.Duplicate(aFile);

    iObjectList.AppendL( file );

    FLOG(_L("[BTSU]\t CBTServiceParameterList::AddObjectL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::XhtmlCount
// -----------------------------------------------------------------------------
//
TInt CBTServiceParameterList::XhtmlCount() const
    {
    return iXhtmlList.Count();
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ImageCount
// -----------------------------------------------------------------------------
//
TInt CBTServiceParameterList::ImageCount() const
    {
    return iImageList.Count();
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ObjectCount
// -----------------------------------------------------------------------------
//
TInt CBTServiceParameterList::ObjectCount() const
    {   
    return iObjectList.Count();
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::XhtmlAt
// -----------------------------------------------------------------------------
//
TBTSUXhtmlParam CBTServiceParameterList::XhtmlAt( TInt aIndex ) const
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::XhtmlAt()"));
    __ASSERT_DEBUG( aIndex >= 0 && aIndex < XhtmlCount(), BTSUPanic( EBTSUPanicOutOfRange ) );

    TBTSUXhtmlParam param;
    param.iFileName = iXhtmlList[ aIndex].iFileName;
    param.iRefObjectList = iXhtmlList[ aIndex].iRefObjectList;

    FLOG(_L("[BTSU]\t CBTServiceParameterList::XhtmlAt() completed"));

    return param;
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ImageAt
// -----------------------------------------------------------------------------
//
TBTSUImageParam CBTServiceParameterList::ImageAtL( TInt aIndex ) const
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::ImageAt()"));    
    
    TBTSUImageParam param; 
    
    if(aIndex<0 || aIndex> ImageCount() )
 		{
 		User::Leave(KErrGeneral);
 		}
    
    param.iFile        = iImageList[aIndex].iFile;
    param.iMimeType    = iImageList[aIndex].iMimeType;
    param.iDisplayName = iImageList[aIndex].iDisplayName;
    param.iPixelSize   = iImageList[aIndex].iPixelSize;
    param.iFileSize    = iImageList[aIndex].iFileSize;        
    param.iSend        = iImageList[aIndex].iSend;
	
	
    FLOG(_L("[BTSU]\t CBTServiceParameterList::ImageAt() completed"));

    return param;
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ObjectAt
// -----------------------------------------------------------------------------
//
RFile& CBTServiceParameterList::ObjectAtL( TInt aIndex ) 
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::ObjectAt()"));
    
    if(aIndex<0 || aIndex> iObjectList.Count() )
 		{
 		User::Leave(KErrGeneral);
 		}
    return iObjectList[aIndex];
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::HasAnyReferencedObjects
// -----------------------------------------------------------------------------
//
TBool CBTServiceParameterList::HasAnyReferencedObjects() const
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::HasAnyReferencedObjects()"));

    TBool result = EFalse;

    for ( TInt index = 0; index < iXhtmlList.Count(); index++ )
        {
        if ( iXhtmlList[index].iRefObjectList->Count() > 0 )
            {
            result = ETrue;
            break;
            }
        }

    FTRACE(FPrint(_L("[BTSU]\t CBTServiceParameterList::HasAnyReferencedObjects() completed with %d"), result ));

    return result;
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ObjectListSizeL
// -----------------------------------------------------------------------------
//
TInt CBTServiceParameterList::ObjectListSizeL() const
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::ObjectListSizeL()"));
 
    TInt totalFileSize = 0; 	
    
    for ( TInt index = 0; index < iObjectList.Count(); index++ )
        {                
        TInt fileSize = 0;
        iObjectList[index].Size( fileSize );
        totalFileSize += fileSize;        
        }
    return totalFileSize;
    }

// -----------------------------------------------------------------------------
// CBTServiceParameterList::ImageListSize
// -----------------------------------------------------------------------------
//
TInt CBTServiceParameterList::ImageListSize() const
    {
    FLOG(_L("[BTSU]\t CBTServiceParameterList::ImageListSize()"));

    TInt totalFileSize = 0;
    for ( TInt index = 0; index < iImageList.Count(); index++ )
        {
        totalFileSize += iImageList[index].iFileSize;
        }

    FTRACE(FPrint(_L("[BTSU]\t CBTServiceParameterList::ImageListSize %d"), totalFileSize ));
    return totalFileSize;
    }
// -----------------------------------------------------------------------------
// CBTServiceParameterList::RemoveImageL
// -----------------------------------------------------------------------------
//
 
 void CBTServiceParameterList::RemoveImageL(TInt aIndex)
 	{
 	if(aIndex<0 || aIndex> iImageList.Count() )
 		{
 		User::Leave(KErrGeneral);
 		}
 	
    delete iImageList[aIndex].iMimeType;
    delete iImageList[aIndex].iDisplayName;	
    iImageList.Remove(aIndex);
    iRemovedImageCount++;
 	}
 	
// -----------------------------------------------------------------------------
// CBTServiceParameterList::RemovedImageCount
// -----------------------------------------------------------------------------
// 
 TInt CBTServiceParameterList::RemovedImageCount()
 	{
 	return iRemovedImageCount;
 	}
 	
// -----------------------------------------------------------------------------
// CBTServiceParameterList::ResetImageHandleL
// -----------------------------------------------------------------------------
//  	
void CBTServiceParameterList::ResetImageHandleL(TInt aIndex)
 	{
 	if(aIndex<0 || aIndex> iImageList.Count() )
 		{
 		User::Leave(KErrGeneral);
 		} 	    
    iImageList[aIndex].iFile=RFile();
 	}
// -----------------------------------------------------------------------------
// CBTServiceParameterList::ResetObjectHandleL
// -----------------------------------------------------------------------------
// 	
void CBTServiceParameterList::ResetObjectHandleL(TInt aIndex)
 	{
    if(aIndex<0 || aIndex> iObjectList.Count() )
 		{
 		User::Leave(KErrGeneral);
 		}
 	iObjectList[aIndex]=RFile();
 	}


// -----------------------------------------------------------------------------
// CBTServiceParameterList::ResetObjectHandleL
// -----------------------------------------------------------------------------
// 	
void CBTServiceParameterList::MarkAsSendL(TInt aIndex)
 	{
    if(aIndex<0 || aIndex> iImageList.Count() )
 		{
 		User::Leave(KErrGeneral);
 		}
 	iImageList[aIndex].iSend=ETrue;
 	}


//  End of File  
