/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Image push implementation
*
*/



// INCLUDE FILES
#include "BTSBIPController.h"
#include "BTSUDebug.h"
#include "BTSUImageConverter.h"
#include "BTSUXmlParser.h"
#include <hbtextresolversymbian.h>



// CONSTANTS
// image push target header
_LIT8( KBIPImagePushID, "\xE3\x3D\x95\x45\x83\x74\x4A\xD7\x9E\xC5\xC1\x6B\xE3\x1E\xDE\x8E" );

// type headers
_LIT8( KBTSBIPCapabilities, "x-bt/img-capabilities\0");
_LIT8( KBTSBIPImageType,    "x-bt/img-img\0");
_LIT8( KBTSBIPThmType,      "x-bt/img-thm\0");

// imageBTS descriptor
_LIT8( KBTSBIPDescriptorStart,     "<image-descriptor version=\"1.0\">\r" );
_LIT8( KBTSBIPDescriptorEncoding,  "<image encoding=\"" );
_LIT8( KBTSBIPDescriptorPixel,     "\" pixel=\"" );
_LIT8( KBTSBIPDescriptorSize,      "\" size=\"" );
_LIT8( KBTSBIPDescriptorEnd,       "\"/>\r</image-descriptor>" );

// temp file path for capabilities object

//temp file path drive letter
_LIT(KBTSBIPTempPathDrive,"c:");
const TInt KBTSUMaxPathLenght=256;
const TInt KBTSUMaxPrivatePathLenght=20;

_LIT(KBTNotSendSomeText,"txt_bt_info_receiving_device_does_not_support_all");
_LIT(KBTNotSendOneText,"txt_bt_info_receiving_device_does_not_support_this");




// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBIPController::CBTSBIPController
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSBIPController::CBTSBIPController( MBTServiceObserver* aObserver, 
                                      CBTServiceParameterList* aList ) :
                                      iListPtr( aList ),
                                      iObserverPtr( aObserver )
									  
    {
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSBIPController::ConstructL( const TUint aRemotePort, 
                                    const TBTDevAddr& aRemoteDeviceAddr )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::ConstructL()"));
    
    // Add image push target header
    //
    CObexHeader* header = CObexHeader::NewL();
    CleanupStack::PushL( header );
    header->SetByteSeqL( KBTSUTargetHeader, KBIPImagePushID );

    RArray<CObexHeader*> headerList;
    CleanupClosePushL( headerList );
    headerList.Append( header );
    
    CreateClientL ( this, aRemoteDeviceAddr, aRemotePort, headerList );    

    CleanupStack::Pop( 2 ); //header, headerlist
    headerList.Close();

    FLOG(_L("[BTSU]\t CBTSBIPController::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSBIPController* CBTSBIPController::NewL( MBTServiceObserver* aObserver,
                                           const TUint aRemotePort,
                                           const TBTDevAddr& aRemoteDeviceAddr,
                                           CBTServiceParameterList* aList )
    {
    CBTSBIPController* self = 
        new( ELeave ) CBTSBIPController( aObserver, aList );
    CleanupStack::PushL( self );
    self->ConstructL( aRemotePort, aRemoteDeviceAddr );
    CleanupStack::Pop(self);
    return self;
    }

// Destructor
CBTSBIPController::~CBTSBIPController()
    {
    DeleteTempFile( iThumbnailFileName );
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::ConnectCompleted
// -----------------------------------------------------------------------------
//
void CBTSBIPController::ConnectCompleted( TInt aStatus )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::ConnectCompleted()"));

    if ( aStatus == KErrNone )
        {
        iFileIndex = 0;
        // get remote device capabilities
        //
        TRAPD( error, GetL() );
        if ( error != KErrNone )
            {
            iObserverPtr->ControllerComplete( EBTSGettingFailed );
            }
        }
    else
        {
        //Error on Obex level
        //
        iObserverPtr->ControllerComplete( EBTSConnectingFailed );
        }

    FLOG(_L("[BTSU]\t CBTSBIPController::ConnectCompleted() completed"));
    }
// -----------------------------------------------------------------------------
// CBTSBIPController::ClientConnectionClosed
// -----------------------------------------------------------------------------
//
void CBTSBIPController::ClientConnectionClosed()
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::ClientConnectionClosed()"));

    // Everything ready, stop service
    //    
    iObserverPtr->ControllerComplete( EBTSNoError );	
    FLOG(_L("[BTSU]\t CBTSBIPController::ClientConnectionClosed() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::PutCompleted
// -----------------------------------------------------------------------------
//
void CBTSBIPController::PutCompleted( TInt aStatus, 
                                      const CObexHeaderSet* aPutResponse )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::PutCompleted()"));

    // Remove temporary thumbnail file
    //
    DeleteTempFile( iThumbnailFileName );
    if ( aStatus == KErrNone )
        {              
        iFileIndex++;
        // Send was ok. Start sending next image
        //
        TRAPD( error, SendL() );
        if ( error )
            {
            FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::Send leaved with %d"), error ));
            iObserverPtr->ControllerComplete( EBTSPuttingFailed);
            }                
        }
    else if ( aStatus == KErrIrObexRespPartialContent )
        {
        // Remote device has requested a thumbnail image
        //
        TRAPD( error, SendThumbnailL(aPutResponse ) );
        if ( error )
            {
            FTRACE(FPrint(_L("[BTSU]\t CBTSBPPController::Send thumbnail leaved with %d"), error ));
            iObserverPtr->ControllerComplete( EBTSPuttingFailed );
            }
        }
    else
        {
        // Some error on Obex level
        //
        iObserverPtr->ControllerComplete( EBTSPuttingFailed);
        }

    FLOG(_L("[BTSU]\t CBTSBIPController::PutCompleted() done"));
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::GetCompleted
// -----------------------------------------------------------------------------
//
void CBTSBIPController::GetCompleted( TInt aStatus, 
                                      CObexBufObject* aGetResponse )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::GetCompleted()"));	

    if ( aStatus == KErrAbort )
        {
        // Connection is cancelled
        //
        iObserverPtr->ControllerComplete( EBTSGettingFailed );
        }
    
    else if ( aStatus == KErrNone )
        {
        TRAPD( error, HandleGetCompleteIndicationL( aGetResponse ) );
        if ( error != KErrNone )
            {
            DeleteTempFile( iTempFileName );
            // Error on capability handling
            //
            iObserverPtr->ControllerComplete( EBTSGettingFailed );
            }
        }
    else if( aStatus != KErrAbort && aGetResponse->BytesReceived()==0 )
        {
        TRAPD( error,iObserverPtr->LaunchProgressNoteL( iClient,iListPtr->ImageCount() ) );
        error=KErrNone;
        TRAP(error, SendL() );    	 
        if ( error != KErrNone )
            {            
            iObserverPtr->ControllerComplete( EBTSPuttingFailed );
            }    	      
        } 	
    else if ( aStatus != KErrNone && aGetResponse->BytesReceived()>0 )
        {
        // Error on Obex level
        //
        iObserverPtr->ControllerComplete( EBTSGettingFailed );
        }
    

    FLOG(_L("[BTSU]\t CBTSBIPController::GetCompleted() done"));
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::SendL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::SendL()
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::SendL()"));

    
    if ( iListPtr->ImageCount() > 0 && iFileIndex < iListPtr->ImageCount())
        {        
        
        RArray<CObexHeader*> headerList;
        CleanupClosePushL( headerList );

        // Add Type header
        //
        CObexHeader* typeHeader = CObexHeader::NewL();
        CleanupStack::PushL( typeHeader );
        typeHeader->SetByteSeqL( KBTSUTypeHeader, KBTSBIPImageType );
        headerList.Append( typeHeader );

        // Add image descriptor
        //
        HBufC8* imagedescriptor = CreateImageDescriptorL( );
        CleanupStack::PushL( imagedescriptor );
  
        CObexHeader* imageDescriptorHeader = CObexHeader::NewL();
        CleanupStack::PushL( imageDescriptorHeader );
        imageDescriptorHeader->SetByteSeqL( KBTSUImgDescriptorHeader, imagedescriptor->Des() );
        headerList.Append( imageDescriptorHeader );

        // Send image
        //
        
        TBTSUImageParam imageparam = iListPtr->ImageAtL( iFileIndex );        
        RBuf filename;
        filename.CreateL(KMaxFileName);
        CleanupClosePushL(filename);
        imageparam.iFile.Name(filename);
        
        iObserverPtr->UpdateProgressNoteL(imageparam.iFileSize,iFileIndex,filename);
        CleanupStack::PopAndDestroy(&filename);
        
        iListPtr->MarkAsSendL(iFileIndex);
        
        
        iClient->PutObjectL( headerList, imageparam.iFile );
        
          
        CleanupStack::Pop(4); // headerList, imageDescriptorHeader, typeHeader, imagedescriptor
        delete imagedescriptor;
        headerList.Close();
        }
    else
        {
        FLOG(_L("[BTSU]\t CBTSBIPController::SendL() all images sent, closing connection"));

        // All images sent, close client connection.
        //
        iClient->CloseClientConnection();
        }

    FLOG(_L("[BTSU]\t CBTSBIPController::SendL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::GetL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::GetL()
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::GetL()"));

    RArray<CObexHeader*> headerList;
    CleanupClosePushL(headerList);
    
    // Add capabilities type header
    //
    CObexHeader* typeHeader = CObexHeader::NewL();
    CleanupStack::PushL( typeHeader );
    typeHeader->SetByteSeqL( KBTSUTypeHeader, KBTSBIPCapabilities );
    headerList.Append( typeHeader );

    // Get capabilities object from remote device
    //
    iClient->GetObjectL( headerList );

    CleanupStack::Pop(2); // headerList, typeHeader
    headerList.Close();
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::SendThumbnailL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::SendThumbnailL( const CObexHeaderSet *aPutResponse )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::SendThumbnail()"));

    // Create thumbnail for sending
    // Delete the created thumbnail on PutComplete
    //

    // Fill header array
    //
	
    RArray<CObexHeader*> headerList;
    CleanupClosePushL(headerList);

    // Add ImageHandle header
    //
    CObexHeader* imageHandleHeader = CObexHeader::NewL();
    CleanupStack::PushL( imageHandleHeader );

    aPutResponse->First();
	User::LeaveIfError(aPutResponse->Find(KBTSUImageHandleHeader,
			 *imageHandleHeader ) );
	headerList.Append( imageHandleHeader );
	
    // Add Type header
    //
    CObexHeader* typeHeader = CObexHeader::NewL();
    CleanupStack::PushL( typeHeader );
    typeHeader->SetByteSeqL( KBTSUTypeHeader, KBTSBIPThmType );
    headerList.Append( typeHeader );
	
	
    CreateTempFileL( iThumbnailFileName );
    CBTSUImageConverter* imageConverter = CBTSUImageConverter::NewL();
    CleanupStack::PushL( imageConverter );
    
    TBTSUImageParam imgparam = iListPtr->ImageAtL( iFileIndex );
    imageConverter->CreateThumbnailL(imgparam.iFile, iThumbnailFileName );
    
    CleanupStack::PopAndDestroy(imageConverter);    

	// Add Name header
    //
    TParse parse;
    User::LeaveIfError( parse.Set( iThumbnailFileName, NULL, NULL ) );
    CObexHeader* nameHeader = CObexHeader::NewL();
    CleanupStack::PushL( nameHeader );
    nameHeader->SetUnicodeL( KBTSUNameHeader, parse.NameAndExt() );
    headerList.Append( nameHeader );

    // send thumbnail
    //
    iClient->PutObjectL( headerList, iThumbnailFileName );
    
    // Cleanup
    //
    CleanupStack::Pop(4); // headerList, imageHandleHeader, typeHeader, nameHeader
    headerList.Close();
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::CreateTempFileL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::CreateTempFileL( TFileName& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::CreateTempFileL()"));

    RFs fileSession;
    RFile file;    
    
    TBuf<KBTSUMaxPrivatePathLenght> privatepath;     
    TBuf<KBTSUMaxPathLenght> tempPath;     
    
    User::LeaveIfError( fileSession.Connect() );
    CleanupClosePushL( fileSession );    
    
    User::LeaveIfError(fileSession.CreatePrivatePath(EDriveC));
    User::LeaveIfError(fileSession.PrivatePath(privatepath));
    tempPath.Append(KBTSBIPTempPathDrive());
    tempPath.Append(privatepath);    
    User::LeaveIfError( file.Temp( fileSession, privatepath, 
                            aFileName, EFileWrite ) );
    
    file.Flush();
    file.Close();
    CleanupStack::Pop();    // Close fileSession
    fileSession.Close();
    }


// -----------------------------------------------------------------------------
// CBTSBIPController::GenerateTempFileNameL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::GenerateTempFileNameL( TFileName& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::GenerateTempFileNameL()"));

    RFs fileSession;
    RFile file;  
    
    TBuf<KBTSUMaxPrivatePathLenght> privatepath;     
    TBuf<KBTSUMaxPathLenght> tempPath;     
    
    User::LeaveIfError( fileSession.Connect() );
    CleanupClosePushL( fileSession );
    
    User::LeaveIfError(fileSession.CreatePrivatePath(EDriveC));
    User::LeaveIfError(fileSession.PrivatePath(privatepath ));
    tempPath.Append(KBTSBIPTempPathDrive());
    tempPath.Append(privatepath);
    User::LeaveIfError(file.Temp( fileSession, tempPath, 
                            aFileName, EFileWrite ) );                            
    
    file.Flush();
    file.Close();
    // Delete the file so that only a unique name is created
    fileSession.Delete( aFileName );
    CleanupStack::Pop();    // Close fileSession
    fileSession.Close();
    }    


// -----------------------------------------------------------------------------
// CBTSBIPController::DeleteTempFileL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::DeleteTempFile( TFileName& aFileName )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::DeleteTempFile()"));

    if ( &aFileName != NULL )
        {
        if ( aFileName.Length() > 0 )
            {
            RFs fileSession;
            TInt retVal = fileSession.Connect();
            if (retVal == KErrNone)
                {
                fileSession.Delete( aFileName );
                }
            fileSession.Close();
            }
        }

    FLOG(_L("[BTSU]\t CBTSBIPController::DeleteTempFile() complete"));
    }

// -----------------------------------------------------------------------------
// CBTSBIPController::CreateImageDescriptorL
// -----------------------------------------------------------------------------
//
HBufC8*  CBTSBIPController::CreateImageDescriptorL()
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::CreateImageDescriptorL()"));

    //   Example image descriptor of an small jpeg picture
    //   with size 160*120 pixels and a size of 5000 bytes.
    //
    //  <image-descriptor version=\"1.0\">
    //  <image encoding=\"JPEG\" pixel=\"160*120\" size=\"5000\"/>
    //  </image-descriptor>
    TBTSUImageParam param = iListPtr->ImageAtL( iFileIndex );
    
    // Add start of image description
    //
    TBuf8<KBTSUMaxStringLength> string( KBTSBIPDescriptorStart );

    // Add image encoding
    //
    string.Append( KBTSBIPDescriptorEncoding );
    string.Append( *param.iDisplayName );

    // Add image pixel size
    //
    string.Append( KBTSBIPDescriptorPixel );
    string.AppendNum( param.iPixelSize.iWidth );
    string.Append( '*' );
    string.AppendNum( param.iPixelSize.iHeight );

    // Add image size
    //
    string.Append( KBTSBIPDescriptorSize );
    string.AppendNum( param.iFileSize );

    // Add end of image description
    //
    string.Append( KBTSBIPDescriptorEnd );

    FLOG(_L("[BTSU]\t CBTSBIPController::CreateImageDescriptorL() completed"));
    
    return string.AllocL();
    }


// -----------------------------------------------------------------------------
// CBTSBIPController::HandleGetCompleteIndicationL
// -----------------------------------------------------------------------------
//
void CBTSBIPController::HandleGetCompleteIndicationL( CObexBufObject* aGetResponse )
    {
    FLOG(_L("[BTSU]\t CBTSBIPController::HandleGetCompleteIndicationL()"));

    TBool found;
    TBool allSupported;
    TInt picindex,capindex;
    CBTSUXmlParser* xmlParser = CBTSUXmlParser::NewL();
    CleanupStack::PushL( xmlParser );
    GenerateTempFileNameL( iTempFileName );
    aGetResponse->WriteToFile( iTempFileName );
    aGetResponse->Reset();
    
    // Parse capability object and create a list of supported image encodings
    //
    RArray<TBTSUImageCap>* remoteCapabilityList = 
        xmlParser->GetImgCapabilityListL( iTempFileName );
    
    // Delete the temp file since we dont need it anymore
    //
    DeleteTempFile( iTempFileName );

    // Go through all the images on our sending list and check 
    // if remote device is capable of receiving those.
    // 
    allSupported= ETrue;   
    for (picindex=0; picindex< iListPtr->ImageCount(); picindex++ )
    	{
    	found=EFalse;
    	for (capindex=0; capindex < remoteCapabilityList->Count(); capindex++)
    		{
    		//Find first is encoding suported			
    		if((iListPtr->ImageAtL( picindex ).iDisplayName->Compare(*(*remoteCapabilityList)[capindex].iEncoding))==0)		
    			{
    			found=ETrue;    			
    			//Check pixel size
    			if((*remoteCapabilityList)[capindex].iMinPixelSize.iHeight>=0)
    				{
    				if(((*remoteCapabilityList)[capindex].iMaxPixelSize.iWidth < iListPtr->ImageAtL( picindex ).iPixelSize.iWidth)  ||
    				   ((*remoteCapabilityList)[capindex].iMaxPixelSize.iHeight < iListPtr->ImageAtL( picindex ).iPixelSize.iHeight)|| 
    				   ((*remoteCapabilityList)[capindex].iMinPixelSize.iHeight > iListPtr->ImageAtL( picindex ).iPixelSize.iHeight)||
    				   ((*remoteCapabilityList)[capindex].iMinPixelSize.iWidth > iListPtr->ImageAtL( picindex ).iPixelSize.iWidth)
    				   )
    					{
    					found=EFalse;
    					}
    				}
    		
    			//Check byte size
    			if((*remoteCapabilityList)[capindex].iMaxByteSize>=0)
    				{    				
    				if((*remoteCapabilityList)[capindex].iMaxByteSize<iListPtr->ImageAtL( picindex ).iFileSize)
    					{
    					found=EFalse;
    					}
    				}    	
    			// If file is supported, stop the loop.
    			//
    			if ( found )
    			    break;
    			}
     		}
    	allSupported = found & allSupported;
    	}
    	
	for (TInt index=0; index < remoteCapabilityList->Count(); index++)
		{
		if((*remoteCapabilityList)[index].iEncoding)
			{
			delete ((*remoteCapabilityList)[index].iEncoding);
			}

		}
		
	remoteCapabilityList->Close();
	delete remoteCapabilityList;
    CleanupStack::PopAndDestroy( xmlParser ); 
    
    if(!allSupported  && iListPtr->ImageCount() > 1)
    	{      	
        HBufC* sendText = HbTextResolverSymbian::LoadLC(KBTNotSendSomeText);
    	iObserverPtr->LaunchConfirmationQuery(sendText->Des());
    	CleanupStack::PopAndDestroy( sendText );
    	}
    else if ( !allSupported  &&  iListPtr->ImageCount() == 1)
        {
        // We allow user to choose wheather to send the image file which is not supported on target device
        HBufC* sendText = HbTextResolverSymbian::LoadLC(KBTNotSendOneText);
        iObserverPtr->LaunchConfirmationQuery(sendText->Des());
        CleanupStack::PopAndDestroy( sendText );
        } 	
    else if( allSupported )  	
        {
    	iObserverPtr->LaunchProgressNoteL( iClient, iListPtr->ImageCount() + iListPtr->ObjectCount());
    
	    // Start sending images
    	//   	
   		SendL();   		
    	}
    	
    
    FLOG(_L("[BTSU]\t CBTSBIPController::HandleGetCompleteIndicationL() #3"));
    }
    

//-----------------------------------------------------------------------------
// void CBTSBIPController::ConnectTimedOut()
// -----------------------------------------------------------------------------
//        
void CBTSBIPController::ConnectTimedOut()    
    {
    iObserverPtr->ConnectTimedOut();    
    }



void CBTSBIPController::SendUnSupportedFiles()
    {
    // Everything went ok. Start sending images

    // group leaving functions in one trap for better performance:
    TRAPD(err, {
            iObserverPtr->LaunchProgressNoteL( iClient, iListPtr->ImageCount() );
            SendL(); } );
    
    if ( err != KErrNone )
        {
        DeleteTempFile( iTempFileName );
        // Error on capability handling
        //
        iObserverPtr->ControllerComplete( EBTSGettingFailed );
        }
    }

//  End of File  
