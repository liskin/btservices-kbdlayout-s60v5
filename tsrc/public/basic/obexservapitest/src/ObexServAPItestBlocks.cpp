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
#include <btmsgtypeuid.h>
#include <UiklafInternalCRKeys.h>
#include <UikonInternalPSKeys.h>
#include <obexutils.rsg>
#include <obex.h>
#include "testlogger.h"
#include "ObexServAPItest.h"



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
TInt CObexServAPItest::GetCenRepKeyIntValueL( CStifItemParser&  /*aItem*/ )
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
    TRAPD(error, iMessageServerIndex = TObexUtilsMessageHandler::CreateOutboxEntryL( KUidMsgTypeBt, R_BT_SEND_OUTBOX_SENDING ));  
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


// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  End of File
