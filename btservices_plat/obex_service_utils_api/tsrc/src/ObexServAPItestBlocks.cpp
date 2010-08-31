/*
* Copyright (c) 2002 - 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ?Description
*
*/



// INCLUDE FILES
#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>
#include <UiklafInternalCRKeys.h>
#include <UikonInternalPSKeys.h>
#include <obex.h>
#include "testlogger.h"
#include "ObexServAPItest.h"

#include    <btengdomaincrkeys.h> 

// CONSTANTS
const TInt    KBufferSize = 0x4000;  // 16kB

// Defined to coincide with the definition in btmsgtypeuid.h 
const TUid KUidMsgTypeBt                 = {0x10009ED5};


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CObexServAPItest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CObexServAPItest::Delete() 
    {
    delete iObexObject;
    iObexObject = NULL;
    delete iBuf;
    iBuf = NULL;
    iRFs.Close(); 
    }

// -----------------------------------------------------------------------------
// CObexServAPItest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "GetFileSystemStatus", CObexServAPItest::GetFileSystemStatusL ),
        ENTRY( "GetMmcFileSystemStatus", CObexServAPItest::GetMmcFileSystemStatusL ),
        ENTRY( "GetMessageCentreDriveL", CObexServAPItest::GetMessageCentreDriveL ),
        ENTRY( "CreateDefaultMtmServiceL", CObexServAPItest::CreateDefaultMtmServiceL ),
        ENTRY( "GetCenRepKeyIntValueL", CObexServAPItest::GetCenRepKeyIntValueL ),
        ENTRY( "GetPubSubKeyIntValue", CObexServAPItest::GetPubSubKeyIntValueL ),
        ENTRY( "CreateOutBoxEntry", CObexServAPItest::CreateOutBoxEntryL ),
        ENTRY( "RemoveOutBoxEntry", CObexServAPItest::RemoveOutBoxEntryL ),
        ENTRY( "SaveObjToInbox", CObexServAPItest::SaveObjToInboxL ),
        ENTRY( "CreateEntryToInbox", CObexServAPItest::CreateEntryToInboxL ),
        ENTRY( "SaveRFileObjectToInbox", CObexServAPItest::SaveRFileObjectToInboxL ),
        ENTRY( "RemoveObjectFromInbox", CObexServAPItest::RemoveObjectL ),
        ENTRY( "GetCenRepKeyStringValueL", CObexServAPItest::GetCenRepKeyStringValueL), // JHä 
        ENTRY( "CreateReceiveBufferAndRFileL", CObexServAPItest::CreateReceiveBufferAndRFileL), // JHä 
        ENTRY( "RemoveTemporaryRFileL", CObexServAPItest::RemoveTemporaryRFileL), // JHä 
        ENTRY( "SaveFileToFileSystemL", CObexServAPItest::SaveFileToFileSystemL), // JHä 
        ENTRY( "AddEntryToInboxL", CObexServAPItest::AddEntryToInboxL), // JHä 
        ENTRY( "UpdateEntryAttachmentL", CObexServAPItest::UpdateEntryAttachmentL), // JHä 
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }
    
 // -----------------------------------------------------------------------------
// CBtApiTest::TestCompleted
// -----------------------------------------------------------------------------
//  
void CObexServAPItest::TestCompleted( TInt aErr, const TUint8* aFunc, const TDesC& aArg )
    {
    iTestLogger->LogResult( (TPtrC8( aFunc )), aArg, aErr );
    Signal( aErr );
    }


// -----------------------------------------------------------------------------
// CObexServAPItest::GetFileSystemStatusL
// Test code for getting file system status.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::GetFileSystemStatusL( CStifItemParser& /*aItem*/ )
    {
    // Print to log file
    TInt retVal=TObexUtilsMessageHandler::GetFileSystemStatus();
    if (retVal == 0 || retVal == KErrDiskFull)
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("GetFileSystemStatusL"));
        }
    else
        {
        TestCompleted( retVal, TLFUNCLOG, _L("Error getting files ystem status"));    
        }    
    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::GetMmcFileSystemStatusL
// Test code for getting mmc file system status.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::GetMmcFileSystemStatusL( CStifItemParser& /*aItem*/ )
    {
    TInt retVal=TObexUtilsMessageHandler::GetMmcFileSystemStatus();
    if (retVal == 0 || retVal == KErrDiskFull)
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("GetFileSystemStatusL"));
        }
    else
        {
        TestCompleted( retVal, TLFUNCLOG, _L("Error getting files ystem status"));    
        }    

    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::GetMessageCentreDriveL
// Test code for getting message center drive. 
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::GetMessageCentreDriveL( CStifItemParser& /*aItem*/ )
    {
    
    TInt retVal = TObexUtilsMessageHandler::GetMessageCentreDriveL();
    if (retVal >= 0 )
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("GetMessageCentreDrive"));
        }
    else
        {
        TestCompleted( retVal, TLFUNCLOG, _L("GetMessageCentreDrive failed"));    
        }    

    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::CreateDefaultMtmServiceL
// Test code for creating Default MTM services.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::CreateDefaultMtmServiceL( CStifItemParser& /*aItem*/ )
    {

    TRAPD(error, TObexUtilsMessageHandler::CreateDefaultMtmServiceL(KUidMsgTypeBt) );
    if (error == KErrNone)
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("CreateDefaultMtmServiceL"));        
        }
    else
        {
        TestCompleted( error, TLFUNCLOG, _L("CreateDefaultMtmServiceL"));                       
        }
    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::GetCenRepKeyIntValueL
// Test code for Getting CenRep key.    
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::GetCenRepKeyIntValueL( CStifItemParser&  /*aItem*/ )  // JHä note: Is return code handling done right? 
    {
    TInt limit=0;    
    TInt error=0;
    TRAPD(leaveVal, error = TObexUtilsMessageHandler::GetCenRepKeyIntValueL(KCRUidUiklaf,
                                                                KUikOODDiskCriticalThreshold,
                                                                limit));
    if( leaveVal != KErrNone ) 
        {
        TestCompleted( leaveVal, TLFUNCLOG, _L("GetCenRepKeyIntValueL leaves"));                       
        }    
    else if (error == KErrNone)
        {
        TestCompleted( leaveVal, TLFUNCLOG, _L("GetCenRepKeyIntValue return error"));                       
        }   
    else
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("GetCenRepKeyIntValueL"));                           
        }    
    
    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::GetPubSubKeyIntValueL
// Test code for Getting PubSub key.    
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::GetPubSubKeyIntValueL( CStifItemParser& /*aItem*/ )
    {
    TInt status = KErrNone;
    TInt retVal = KErrNone;
    
    TRAPD(leaveVal, retVal = TObexUtilsMessageHandler::GetPubSubKeyIntValue(KPSUidUikon, KUikFFSFreeLevel, status));
    
    if( leaveVal != KErrNone ) 
        {
        TestCompleted( leaveVal, TLFUNCLOG, _L("GetPubSubKeyIntValueL leaves"));                       
        }    
    else if (retVal == KErrNone)
        {
        TestCompleted( retVal, TLFUNCLOG, _L("GetPubSubKeyIntValueL return error"));                       
        }   
    else
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("GetPubSubKeyIntValue"));                           
        }  

    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::OutBoxEntryL
// Test code for Creating outbox entry.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::CreateOutBoxEntryL( CStifItemParser& /*aItem*/ )
    {
    //todo resource is unavailable at the moment.
    // need to change the string constant according to the coming new localisation file
    TRAPD(error, iMessageServerIndex = TObexUtilsMessageHandler::CreateOutboxEntryL( KUidMsgTypeBt, 1 ));  
    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("CreateOutBoxEntryL leaves")); 
        }
    else     
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("CreateOutBoxEntryL"));     
        }

    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::OutBoxEntryL
// Test code for removing outbox entry.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::RemoveOutBoxEntryL( CStifItemParser& /*aItem*/ )
    {   
    
    TRAPD(error, TObexUtilsMessageHandler::DeleteOutboxEntryL( iMessageServerIndex ));
    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("RemoveOutBoxEntryL leaves")); 
        }
    else     
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("RemoveOutBoxEntryL"));     
        }
    return KErrNone;

    }
    
// -----------------------------------------------------------------------------
// CObexServAPItest::SaveObjToInboxL
// Test code for save received file to inbox. 
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::SaveObjToInboxL( CStifItemParser& /*aItem*/ )
    {   
    
    CObexBufObject* obexobject = CObexBufObject::NewL( NULL );
    CleanupStack::PushL(obexobject);    
    
    TRAP_IGNORE(TObexUtilsMessageHandler::SaveObjToInboxL(obexobject ,
                                              KNullDesC, 
                                              KUidMsgTypeBt ));
    TestCompleted( KErrNone, TLFUNCLOG, _L("SaveObjToInboxL leaves")); 
    
    CleanupStack::PopAndDestroy(obexobject);
    return KErrNone;

    }

// -----------------------------------------------------------------------------
// CObexServAPItest::CreateAndSaveObjectL
// Test code for creating entry to inbox.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::CreateEntryToInboxL( CStifItemParser& /*aItem*/ )
    {
    iFile = RFile();
    
    if ( iObexObject)
        {
        delete iObexObject;
        iObexObject = NULL;
        }
        
    iObexObject = CObexBufObject::NewL( NULL );
    
    TRAPD( error, TObexUtilsMessageHandler::CreateInboxAttachmentL( iObexObject,
                                                      KUidMsgTypeBt,
                                                      iMessageServerIndex,
                                                      iFile));
                                                      
                                                      
    iBuf = CBufFlat::NewL(1024);
    iBuf->ResizeL(1024);
    
    TObexRFileBackedBuffer bufferdetails(*iBuf,iFile,CObexBufObject::EDoubleBuffering);    
    
    TRAP_IGNORE( iObexObject->SetDataBufL( bufferdetails) );
    
                                                      
                                                      
    iFile.Write(_L8("test"));    
    iObexObject->SetNameL(_L("test.txt"));        
    
    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("CreateEntryToInboxL leaves")); 
        }    
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("CreateEntryToInboxL"));     
        }         
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CObexServAPItest::CreateAndSaveObjectL
// Test code for creating entry to inbox.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::SaveRFileObjectToInboxL( CStifItemParser& /*aItem*/ )
    {
    TRAPD( error, TObexUtilsMessageHandler::SaveObjToInboxL(
            iObexObject,
            iFile,
            iMessageServerIndex));
    
    delete iBuf;
    iBuf = NULL;
    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("SaveObjToInboxL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("SaveObjToInboxL"));     
        }                                          
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CObexServAPItest::RemoveObjectL
// Test code for removing entry to inbox.
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
TInt CObexServAPItest::RemoveObjectL( CStifItemParser& /*aItem*/ )
    {
    TRAPD(error,TObexUtilsMessageHandler::RemoveInboxEntriesL(iObexObject, iMessageServerIndex));    
    delete iBuf;
    iBuf = NULL;
    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("SaveObjToInboxL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("SaveObjToInboxL"));     
        }                                              
    return KErrNone;
    }

// Find out the path of the receive folder. 
TInt CObexServAPItest::GetCenRepKeyStringValueL( CStifItemParser& /*aItem*/ ) // JHä  - Done 
    {
    TInt error = KErrNone;
    TRAPD(leaveVal, error = TObexUtilsMessageHandler::GetCenRepKeyStringValueL(KCRUidBluetoothEngine,
                                                                KLCReceiveFolder,
                                                                iCenRepFolder));
    if( leaveVal != KErrNone ) 
        {
        TestCompleted( leaveVal, TLFUNCLOG, _L("GetCenRepKeyStringValueL leaves"));                       
        }    
    else if (error != KErrNone)
        {
        TestCompleted( leaveVal, TLFUNCLOG, _L("GetCenRepKeyStringValueL return error"));                       
        }   
    else
        {
        TestCompleted( KErrNone, TLFUNCLOG, _L("GetCenRepKeyStringValueL"));                           
        }    

    return KErrNone;    
   	}

// Create the receive buffer and temporary receive file. 
TInt CObexServAPItest::CreateReceiveBufferAndRFileL( CStifItemParser& /*aItem*/ ) // JHä - Done 
    {
    TFileName defaultFolder;
    defaultFolder.Zero();

    User::LeaveIfError(iRFs.Connect());
    
    iFile = RFile(); // Creating an empty file. 

    // Ensure that the object and buffer don't exist. 
    // This also deletes the previous iFile object, if any existed, 
    //  because its handle is held by iObexObject.  
    if(iObexObject)
        {
        delete iObexObject;
        iObexObject = NULL;
        }
    if( iBuf )
        {
        delete iBuf;    
        iBuf=NULL;
        }

    // Then create a new obex object for holding the buffer and the temp file. 
    iObexObject = CObexBufObject::NewL( NULL );

    defaultFolder.Append(_L("C:\\data\\"));
    defaultFolder.Append(iCenRepFolder);   // Got this from central repository in GetCenRepKeyStringValueL

    TRAPD(error,TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL( iFile,  // Out: But iFile is returned as closed. 
                                                                        defaultFolder, // Where to create the temp file.  
                                                                        iTempFullPathFilename, // Out: Can be used to open the file later. 
                                                                        iBuf,   // Out: This contains the message buffer. 
                                                                        KBufferSize));

    // Open the file again. 
    User::LeaveIfError(iFile.Open(iRFs, iTempFullPathFilename, EFileWrite));   

    TObexRFileBackedBuffer bufferdetails(*iBuf,iFile,CObexBufObject::EDoubleBuffering);    
    
    // Do we really need to trap leaves here? Maybe because of Stif?  
    TRAP(error, iObexObject->SetDataBufL( bufferdetails) );
    if (error != KErrNone)
        {
        error = KErrGeneral;  
        }

    iFile.Write(_L8("test")); // Put some test data into the file, although it is not mandatory.      
    iFile.Flush();

    iObexObject->SetNameL(_L("test.txt"));        

    // At this point we have an object in file system (iFile refers to it)    
    // and an entry in Inbox which we can later refer to with iMessageServerIndex.  

    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("CreateReceiveBufferAndRFileL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("CreateReceiveBufferAndRFileL"));     
        }                                              

    // iFile is left open  
    return KErrNone;
    }

// Copy the temporary receive file to the correct place in the file system after receiving is complete. 
TInt CObexServAPItest::SaveFileToFileSystemL( CStifItemParser& /*aItem*/ ) // JHä - Done
    {
    // Create bogus device name. 
    TBTDeviceName remoteDeviceName;
	remoteDeviceName.Zero(); 
	remoteDeviceName.Append(_L("APITestDevice")); 

// The iFile must be still open after calling CreateReceiveBufferAndRFileL method 
	
    TRAPD(error,TObexUtilsMessageHandler::SaveFileToFileSystemL(   iObexObject,
                                                                   KUidMsgTypeBt,
                                                                   iMessageServerIndex,	// This is gotten  
                                                                   iTempFullPathFilename, // Contains the final path of the file
                                                                   iFile,   // Handle to the temp file. 
                                                                   remoteDeviceName)); 

    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("SaveFileToFileSystemL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("SaveFileToFileSystemL"));     
        }                                              
    return KErrNone;
    }

// Create an entry in the Inbox and attach the file there. 
TInt CObexServAPItest::AddEntryToInboxL( CStifItemParser& /*aItem*/ ) // JHä - Done 
    {
    RArray<TMsvId>  tmpMsvIdArray; // Not really needed, but must be passed to the method. 
    
    TRAPD(error,TObexUtilsMessageHandler::AddEntryToInboxL( iMessageServerIndex, 
                                                            iTempFullPathFilename, 
                                                            &tmpMsvIdArray));

    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("AddEntryToInboxL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("AddEntryToInboxL"));     
        }                                              
    return KErrNone;
    }

// Replacing a broken link in Inbox with a proper link to a file. 
TInt CObexServAPItest::UpdateEntryAttachmentL( CStifItemParser& /*aItem*/ ) // JHä - Done
    {
    // Create a new file to be the replacement: 
    TFileName tempFilename; 
    TFileName defaultFolder;
    RFs rfs; 
    RFile newFile;  

    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs);  

    defaultFolder.Zero();
    defaultFolder.Append(_L("C:\\data\\"));
    defaultFolder.Append(iCenRepFolder);   // Got this from central repository in GetCenRepKeyStringValueL
    User::LeaveIfError( newFile.Temp( rfs, defaultFolder, tempFilename, EFileWrite) );
    newFile.Write(_L8("testing replacing"));    
    newFile.Flush();
    newFile.Close(); 

    CleanupStack::PopAndDestroy(); // rfs  

    // Then replace the attachment: 
    CDummySessionObserver* sessionObs = new( ELeave )CDummySessionObserver;;
    CleanupStack::PushL( sessionObs );  
    CMsvSession* msvSession = CMsvSession::OpenSyncL( *sessionObs );
    CleanupStack::PushL(msvSession);
    CMsvEntry* entry = msvSession->GetEntryL(iMessageServerIndex);
    CleanupStack::PushL(entry);
    TRAPD(error,TObexUtilsMessageHandler::UpdateEntryAttachmentL(tempFilename, entry));    

    // Cleanup: 
    CleanupStack::PopAndDestroy(3); // entry, msvSession, sessionObs, 
    
    // Save the path to the new file so it can be referred to later.  
    iTempFullPathFilename = tempFilename; 

    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("UpdateEntryAttachmentL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("UpdateEntryAttachmentL"));     
        }                                              
    return KErrNone;
    }

// Delete the temporary file. Called normally when the link is broken and the user searches for the file. 
// In test code this should be called after creating the temporary RFile and before 
// creating the Inbox entry. 
TInt CObexServAPItest::RemoveTemporaryRFileL( CStifItemParser& /*aItem*/ ) // JHä - Done 
    {
    TRAPD(error,TObexUtilsMessageHandler::RemoveTemporaryRFileL(iTempFullPathFilename));    
    if( error != KErrNone)
        {
        TestCompleted( error, TLFUNCLOG, _L("RemoveTemporaryRFileL leaves")); 
        }
    else     
        {
        TestCompleted( error, TLFUNCLOG, _L("RemoveTemporaryRFileL"));     
        }                                              
    return KErrNone;
    }


// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  End of File
