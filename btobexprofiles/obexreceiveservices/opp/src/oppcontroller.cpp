/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CBtListenActive
*
*/


// INCLUDE FILES
#include    "oppcontroller.h"
#include    "btengdevman.h"
#include    <obexutilsmessagehandler.h>
#include    "debug.h"
#include    <bautils.h>
#include    <btengdomaincrkeys.h>
#include    <sysutil.h>
#include    <btengdomaincrkeys.h> 
#include    <msvids.h>
#include    <driveinfo.h>
#include    <es_sock.h>
#include    <bt_sock.h>
#include    <bluetoothdevicedialogs.h>
#include <hbtextresolversymbian.h>

// CONSTANTS

const TInt    KBufferSize = 0x10000;  // 64 kB
const TInt KFileManagerUID3 = 0x101F84EB; /// File Manager application UID3
_LIT( KBTDevDialogId, "com.nokia.hb.btdevicedialog/1.0" );
_LIT(KLocFileName, "btdialogs_");
_LIT(KPath, "z:/resource/qt/translations/");  

// ================= MEMBER FUNCTIONS =======================


COPPController* COPPController::NewL()
    {
    COPPController* self = new ( ELeave ) COPPController();
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
    }
    
COPPController::COPPController()
	{
    TRACE_FUNC
	}

void COPPController::ConstructL()	
	{
    TRACE_FUNC
    iObexTransferState = ETransferIdle;
    iLowMemoryActiveCDrive = CObexUtilsPropertyNotifier::NewL(this, ECheckPhoneMemory);
    iLowMemoryActiveMMC = CObexUtilsPropertyNotifier::NewL(this, ECheckMMCMemory);        
    iDevMan=CBTEngDevMan::NewL(this);  
    iResultArray = new (ELeave) CBTDeviceArray(1);
    // Get default folder from CenRep 
    TObexUtilsMessageHandler::GetCenRepKeyStringValueL(KCRUidBluetoothEngine, 
                                                       KLCReceiveFolder,
                                                       iCenRepFolder);
    iDialog = CObexUtilsDialog::NewL(this);
    TBool ok = HbTextResolverSymbian::Init(KLocFileName, KPath);
    if (!ok) 
        {
        User::Leave( KErrNotFound );
        }
	} 

COPPController::~COPPController()
    {
    TRACE_FUNC
    delete iObexObject;
    delete iBuf;
    delete iLowMemoryActiveCDrive;
    delete iLowMemoryActiveMMC;
    delete iDevMan;
    if (iResultArray)
        {
        iResultArray->ResetAndDestroy();
        delete iResultArray;
        }
    iFs.Close();
    delete iDialog;
    delete iProgressDialog;
    delete iRecvDoneDialog;
    }

// ---------------------------------------------------------
// ErrorIndication()
// ---------------------------------------------------------
//
void COPPController::ErrorIndication( TInt TRACE_ONLY(aError) )
    {
    TRACE_FUNC
    TRACE_ERROR((_L( "[oppreceiveservice] COPPController: ErrorIndication error:\t %d" ),aError));
    HandleError(EFalse); // false because not explicit abort
    }

// ---------------------------------------------------------
// AbortIndication() 
// ---------------------------------------------------------
//
void COPPController::AbortIndication()
	{
	TRACE_FUNC
	HandleError(ETrue); // true because explicit abort
	}

void COPPController::HandleError(TBool aAbort)
    {
    TRACE_ERROR((_L( "[oppreceiveservice] COPPController:HandleError" )));
    iReceivingFailed = ETrue;
    iShowRecvCompleteDialog = EFalse;
    if( iObexTransferState == ETransferPut || iObexTransferState == ETransferPutDiskError )
        {        
        if(iObexObject)
            {
            iObexObject->Reset();
            }
        CancelTransfer();
        if(!aAbort)
        	{
			if(iMediaType == ESrcsMediaBT)
				{
                TRAP_IGNORE(
                        HBufC* note = HbTextResolverSymbian::LoadLC(_L("txt_bt_dpophead_receiving_failed"));
                        iDialog->ShowErrorNoteL(note->Des());
                        CleanupStack::PopAndDestroy(note);
                        );
				}
			TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported)
        	}
        }
    delete iBuf;
    iBuf = NULL;
    iObexTransferState = ETransferIdle;
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveInboxEntriesL(iObexObject, iMsvIdParent));
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveTemporaryRFileL (iFullPathFilename));
    }

// ---------------------------------------------------------
// TransportUpIndication()
// ---------------------------------------------------------
//
void COPPController::TransportUpIndication()
	{
	TRACE_FUNC    
	iReceivingFailed = EFalse;
    iObexTransferState = ETransferIdle;	
 
	if ( !iFs.Handle() )
	    {
	    TRACE_INFO( (_L( "[oppreceiveservice] TransportUpIndication iFs.Connect()" )) ); 
	    if ( iFs.Connect() )   // error value not preserved, iFs.Handle() checked one more time before first useage
	        {
	        TRACE_INFO( (_L( "[oppreceiveservice] TransportUpIndication iFs.Connect() failed" )) ); 
	        }
	    }

    iFile = RFile();
    iFullPathFilename.Zero();
	}

// ---------------------------------------------------------
// ObexConnectIndication()
// ---------------------------------------------------------
//
TInt COPPController::ObexConnectIndication( const TObexConnectInfo& aRemoteInfo,
                                            const TDesC8& aInfo)
    {
    TRACE_FUNC
    (void) aRemoteInfo;
    (void) aInfo;
    
    if ( iMediaType == ESrcsMediaBT )
        {
        TRACE_INFO( _L( "[oppreceiveservice] ObexConnectIndication: BT media \t" ) );
        
        // Get remote device socket address and bluetooth name
        // Remote bluetooth name will be displayed in the new message in inbox.
        //
        TSockAddr addr;
        iObexServer->RemoteAddr(addr);
        TBTDevAddr tBTDevAddr = static_cast<TBTSockAddr>(addr).BTAddr();
        
        TBTRegistrySearch nameSearch;
        nameSearch.FindAddress(tBTDevAddr);
        
        iResultArray->Reset();
        // ignore any errors here, if we don't get the name, we don't get the name
        static_cast<void>(iDevMan->GetDevices(nameSearch, iResultArray));
        }
    
    return KErrNone;
    }

// ---------------------------------------------------------
// ObexDisconnectIndication(
// ---------------------------------------------------------
//
void COPPController::ObexDisconnectIndication(const TDesC8& aInfo)
    {
    TRACE_FUNC
    (void) aInfo;
    }

// ---------------------------------------------------------
// TransportDownIndication()
// ---------------------------------------------------------
//
void COPPController::TransportDownIndication()
    {
    TRACE_FUNC
    if(!iReceivingFailed && iShowRecvCompleteDialog)
        {
        //Launch recevice completed dialog.
        iRecvDoneDialog = CHbDeviceDialogSymbian::NewL();
        iRecvDoneDialog->SetObserver(this);
    
        CHbSymbianVariantMap* variantMap = CHbSymbianVariantMap::NewL();
        CleanupStack::PushL(variantMap);
        
        TInt dialogIdx = TBluetoothDialogParams::EReceiveDone;
        CHbSymbianVariant* dialogType = CHbSymbianVariant::NewL( (TAny*) &(dialogIdx), 
                                                            CHbSymbianVariant::EInt );
        CleanupStack::PushL(dialogType);
        TBuf16<6> dialogTypeKey;
        dialogTypeKey.Num(TBluetoothDialogParams::EDialogType);
        User::LeaveIfError(variantMap->Add(dialogTypeKey, dialogType));
        CleanupStack::Pop(dialogType);
        
        CHbSymbianVariant* deviceName = CHbSymbianVariant::NewL( (TAny*) (&iRemoteDeviceName), 
                                                            CHbSymbianVariant::EDes );
        CleanupStack::PushL(deviceName);
        TBuf16<6> deviceNameKey;
        deviceNameKey.Num(TBluetoothDeviceDialog::EDeviceName);
        User::LeaveIfError(variantMap->Add(deviceNameKey, deviceName));
        CleanupStack::Pop(deviceName);
        
        CHbSymbianVariant* fileName = CHbSymbianVariant::NewL( (TAny*) (&iReceivingFileName), 
                                                            CHbSymbianVariant::EDes );
        CleanupStack::PushL(fileName);
        TBuf16<6> fileNameKey;
        fileNameKey.Num(TBluetoothDeviceDialog::EReceivingFileName);
        User::LeaveIfError(variantMap->Add(fileNameKey, fileName));
        CleanupStack::Pop(fileName);
        
        CHbSymbianVariant* fileSz = CHbSymbianVariant::NewL( (TAny*) &iTotalSizeByte, 
                                                            CHbSymbianVariant::EInt );
        CleanupStack::PushL(fileSz);
        TBuf16<6> fileSzKey;
        fileSzKey.Num(TBluetoothDeviceDialog::EReceivingFileSize);
        User::LeaveIfError(variantMap->Add(fileSzKey, fileSz));
        CleanupStack::Pop(fileSz);
        
        CHbSymbianVariant* fileCnt = CHbSymbianVariant::NewL( (TAny*) &iFileCount, 
                                                            CHbSymbianVariant::EInt );
        CleanupStack::PushL(fileCnt);
        TBuf16<6> fileCntKey;
        fileCntKey.Num(TBluetoothDeviceDialog::EReceivedFileCount);
        User::LeaveIfError(variantMap->Add(fileCntKey, fileCnt));
        CleanupStack::Pop(fileCnt);
        
        iRecvDoneDialog->Show( KBTDevDialogId(), *variantMap, this );
        CleanupStack::PopAndDestroy(variantMap);
        iShowRecvCompleteDialog = EFalse;
        }
    
    // Remove receiving buffer and files used during file receiving.
    //
    delete iObexObject;
    iObexObject = NULL;
    TRAP_IGNORE(TObexUtilsMessageHandler::RemoveTemporaryRFileL (iFullPathFilename)); 
    iFs.Close();
    iFileCount = 0;
    }

// ---------------------------------------------------------
// PutRequestIndication()
// ---------------------------------------------------------
//
CObexBufObject* COPPController::PutRequestIndication()
    {
    TRACE_FUNC
    iLengthHeaderReceived = EFalse; // New put request so clear header based state
    iObexTransferState = ETransferPut;
    
    // Checking if backup is running now - if backup process is active, then we
    // need to cancel transfer - otherwise phone will freeze during receiving
    // data
    if ( IsBackupRunning() )
        {
        TRACE_INFO ( _L ("Backup in progress! Canceling incoming transfer."));
        iObexTransferState = ETransferPutInitError;
        return NULL;
        }
    
    TRAPD(err, HandlePutRequestL());
    if(err == KErrNone)
        {
        return iObexObject;
        }
    TRACE_INFO( _L( "[oppreceiveservice] COPPController: PutRequestIndication end\t" ) );
    if (iObexTransferState != ETransferPutInitError)
        {
        iObexTransferState = ETransferPutDiskError;
        }
    return NULL;
    }

// ---------------------------------------------------------
// PutPacketIndication()    
// ---------------------------------------------------------
//
TInt COPPController::PutPacketIndication()
    {
    TRACE_FUNC
    if(iObexTransferState == ETransferPutCancel)
        {
        // User cancelled the put request, so error the next packet to terminate the put request.
        // BIP considers the Unauthorized error response suitable for this...
        HandleError(ETrue); // reset state and clear up
        return KErrIrObexRespUnauthorized;
        }
    
    iTotalSizeByte = iObexObject->Length();     // get size of receiving file
    iReceivingFileName = iObexObject->Name();   // get name of receiving file
    
    // Check that capacity is suitable as soon as possible
    if(!iLengthHeaderReceived && iTotalSizeByte > 0)
        {
        iLengthHeaderReceived = ETrue; // total size value is from length header
        TBool capacity = ETrue;
        TRAPD(retTrap, capacity = CheckCapacityL());
        if(retTrap != KErrNone)
            {
            return KErrGeneral;
            }
        if(!capacity)
            {
            //TRAP_IGNORE(TObexUtilsUiLayer::ShowGlobalConfirmationQueryL(R_OUT_OF_MEMORY));
            //todo: Need to use Localized string.
            _LIT(KText, "Not enough memory to execute operation. Delete some documents and try again.");
            TRAP_IGNORE(iDialog->ShowErrorNoteL(KText));
    
            return KErrDiskFull;
            }
        }
    if(iObexObject->Name().Length() > KMaxFileName)
        {
        return KErrAccessDenied;
        }
    if(iObexTransferState == ETransferPutDiskError)
        {
        return KErrDiskFull;
        }
    // successfully received put packet if we reached here
    iObexTransferState = ETransferPut;
    
    // Now we need to either create (in the first instance) or update the dialog on the UI.
    if(ReceivingIndicatorActive())
        {
        TRAPD(err, UpdateReceivingIndicatorL());
        if(err < KErrNone)
            {
            return err;
            }
        }
    else if(!iNoteDisplayed)
        {
        // No note launched yet, so try to launch
        TRAPD(err, LaunchReceivingIndicatorL());
        iNoteDisplayed = (err == KErrNone);
        }
    
    return KErrNone;
    }

// ---------------------------------------------------------
// PutCompleteIndication()
// ---------------------------------------------------------
//
TInt COPPController::PutCompleteIndication()
    {
    TRACE_FUNC
    TInt retVal = KErrNone;
    if(iObexTransferState == ETransferPutCancel)
        {
        retVal = KErrIrObexRespUnauthorized;
        HandleError(ETrue);
        }
    else
        {
        retVal = HandlePutCompleteIndication();
        iObexTransferState = ETransferIdle;
        CloseReceivingIndicator();
        iFileCount++;
        iShowRecvCompleteDialog = ETrue;
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------
// GetRequestIndication()
// ---------------------------------------------------------
//
CObexBufObject* COPPController::GetRequestIndication( CObexBaseObject* aRequiredObject)
    {
    TRACE_FUNC
    (void) aRequiredObject;
    return NULL;
    }

// ---------------------------------------------------------
// GetPacketIndication()
// ---------------------------------------------------------
//
TInt COPPController::GetPacketIndication()
    {
    TRACE_FUNC
    return KErrNone;
    }

// ---------------------------------------------------------
// GetCompleteIndication()
// ---------------------------------------------------------
//
TInt COPPController::GetCompleteIndication()
    {
    TRACE_FUNC
    return KErrNone;
    }

// ---------------------------------------------------------
// SetPathIndication()
// ---------------------------------------------------------
//
TInt COPPController::SetPathIndication( const CObex::TSetPathInfo& aPathInfo, 
                                        const TDesC8& aInfo)
    {
    TRACE_FUNC
    
    (void) aPathInfo;
    (void) aInfo;
    // SetPath is not implemented in OPP - so following IrOBEX guidance, return
    // the Forbidden response code.
    return KErrIrObexRespForbidden;
    }



// ---------------------------------------------------------
// HandleNotifyL()
// ---------------------------------------------------------
//
void COPPController::HandleNotifyL( TMemoryPropertyCheckType aCheckType )
    {    
    TRACE_FUNC        
    
    // Only interested on this notification if we are receiving something
    if ( iObexTransferState == ETransferPut )
        {
        // Check the keys, what has been changed.
        TRACE_INFO( _L( "[oppreceiveservice] COPPController::HandleNotifyL\t" ) );
        if ( aCheckType == ECheckPhoneMemory )
            {            
            if ( SysUtil::FFSSpaceBelowCriticalLevelL( NULL, 0 ) )
                {
                TRACE_INFO( _L( "[oppreceiveservice] COPPController: Obex Server error diskfull\t" ) );
                iObexTransferState = ETransferPutDiskError;                    
                }
            }
        else if ( aCheckType == ECheckMMCMemory )
            {                                
            if ( SysUtil::MMCSpaceBelowCriticalLevelL( NULL, 0 ) )
                {                        
                TRACE_INFO( _L( "[oppreceiveservice] COPPController: Obex Server error diskfull\t" ) );
                iObexTransferState = ETransferPutDiskError;
                }
            }            
        }        
    }
// ---------------------------------------------------------
// HandlePutRequestL()
// ---------------------------------------------------------
//
void COPPController::HandlePutRequestL()
    {
    TRACE_FUNC
    
    delete iObexObject;
    iObexObject = NULL;
    
    iFile = RFile();
    
    if ( !iFs.Handle() )
        {
        User::Leave(KErrGeneral);
        }

    // Assign an initial value to iDrive
    iDrive = GetDriveWithMaximumFreeSpaceL();    
     
    // If iDrive is at critical space level, we immediately show out_of_memory.
    //
    if (SysUtil::DiskSpaceBelowCriticalLevelL( &iFs, 0, iDrive ))
        {
        //TRAP_IGNORE(TObexUtilsUiLayer::ShowGlobalConfirmationQueryL(R_OUT_OF_MEMORY));
        //todo: Need to use Localized string.
        _LIT(KText, "Not enough memory to execute operation. Delete some documents and try again.");
        TRAP_IGNORE(iDialog->ShowErrorNoteL(KText));
    
        User::Leave(KErrGeneral);
        }
        
    TRACE_INFO( (_L( "[oppreceiveservice] HandlePutRequestL %d\t" ),iDrive ) ); 
    
    iObexObject = CObexBufObject::NewL( NULL );    
    
    delete iBuf;
    iBuf = NULL;
    
    TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported);
    if ( iMediaType == ESrcsMediaBT )
        {
        TChar driveLetter;
        iDefaultFolder.Zero();
        iFs.DriveToChar(iDrive, driveLetter);
        iDefaultFolder.Append(driveLetter);
        if ( iDrive == EDriveC )
            {
            iDefaultFolder.Append(_L(":\\data\\"));
            }
        else
            {
            iDefaultFolder.Append(_L(":\\"));
            }
        iDefaultFolder.Append(iCenRepFolder);
        
        iFile = RFile();
        iFullPathFilename.Zero();
        TRAPD(err, TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL(iFile,
                                                                              iDefaultFolder,
                                                                              iFullPathFilename,
                                                                              iBuf,
                                                                              KBufferSize));
        if(err != KErrNone)
            {
            iObexTransferState = ETransferPutInitError;
            User::Leave(KErrGeneral);
            }
        }
    else
        {
        iObexTransferState = ETransferPutInitError;
        User::Leave(KErrGeneral);
        }
    
    User::LeaveIfError(iFile.Open(iFs,iFullPathFilename,EFileWrite));   
    TObexRFileBackedBuffer bufferdetails(*iBuf,iFile,CObexBufObject::EDoubleBuffering);    
    
    TRAPD(err, iObexObject->SetDataBufL( bufferdetails) );
    if (err != KErrNone)
        {
        iObexTransferState = ETransferPutInitError;
        User::Leave(KErrGeneral);  // set to != KErrNone
        }
    
    TRACE_INFO( _L( "[oppreceiveservice] COPPController: HandlePutRequestL completed\t" ) );
    }


// ---------------------------------------------------------
// HandlePutCompleteIndication()
// ---------------------------------------------------------
//
TInt COPPController::HandlePutCompleteIndication()
	{
	TRACE_FUNC        
    TInt retVal = KErrNone;

	TChar driveLetter;
	iDefaultFolder.Zero();
	iFs.DriveToChar(iDrive, driveLetter);
	iDefaultFolder.Append(driveLetter);
	if ( iDrive == EDriveC )
	    {
	    iDefaultFolder.Append(_L(":\\data\\"));
	    }
	else
	    {
	    iDefaultFolder.Append(_L(":\\"));
	    }
	iDefaultFolder.Append(iCenRepFolder);
	iFullPathFilename.Zero();
	iFullPathFilename.Append(iDefaultFolder);
       
        
	TRACE_INFO( (_L( "[oppreceiveservice] HandlePutCompleteIndication %d\t" ),iDrive ) ); 
	
	if (iMediaType==ESrcsMediaBT)
	    {
	    TRAP ( retVal, TObexUtilsMessageHandler::SaveFileToFileSystemL(iObexObject,
	                                                                   KUidMsgTypeBt,
	                                                                   iMsvIdParent,
	                                                                   iFullPathFilename,
	                                                                   iFile,
	                                                                   iRemoteDeviceName));
	    }
	TRACE_ASSERT( iMediaType!=ESrcsMediaIrDA, KErrNotSupported);
	if ( retVal == KErrNone)
	    {
	    TRAP (retVal, TObexUtilsMessageHandler::AddEntryToInboxL(iMsvIdParent, iFullPathFilename));		    
                
    if( retVal != KErrNone )
        {
        TRACE_INFO( (_L( "[oppreceiveservice] HandlePutCompleteIndication AddEntryToInboxL() failed  %d \t" ),retVal ) );                 	
        TRAP_IGNORE(TObexUtilsMessageHandler::RemoveInboxEntriesL(iObexObject, iMsvIdParent));        
        }
        }
	else
	    {
	    TRACE_INFO( (_L( "[oppreceiveservice] HandlePutCompleteIndication failed  %d \t" ),retVal ) ); 
	    }
	

    delete iObexObject;
    iObexObject = NULL;

    delete iBuf;
    iBuf = NULL;
    
    iPreviousDefaultFolder = iDefaultFolder;  // save the last file path where file is successfully saved to file system.
    iMsvIdParent = KMsvNullIndexEntryId; 
    TRACE_INFO( _L( "[oppreceiveservice] HandlePutCompleteIndication Done\t" ) );    
    return retVal;
	}

  
// ---------------------------------------------------------
// CheckCapacity()
// ---------------------------------------------------------
//	    
TBool COPPController::CheckCapacityL()
    {
    TRACE_FUNC_ENTRY   
    
    iDrive = EDriveZ; // Intialize iDrive to Z
    TInt filesize = iObexObject->Length();
    
    RFs rfs ;
    User::LeaveIfError(rfs.Connect());
         
    TInt mmcDrive = KDefaultDrive;   // External memroy card  
    TInt imsDrive = KDefaultDrive;   // Internal mass storage   

    User::LeaveIfError(DriveInfo::GetDefaultDrive(DriveInfo::EDefaultMassStorage, imsDrive));
    User::LeaveIfError(DriveInfo::GetDefaultDrive(DriveInfo::EDefaultRemovableMassStorage, mmcDrive));      
    
    TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL imsDrive=%d; mmcDrive=%d\t" ),imsDrive, mmcDrive ) );
    
    TVolumeInfo volumeInfo;
    TInt err = rfs.Volume(volumeInfo, imsDrive);
    
    // If err != KErrNone, Drive is not available.
    //
    if ( !err )
        {
        // Check capacity on Internal mass storage            
        TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL Internal mass storage\t" )) );
        if ( !SysUtil::DiskSpaceBelowCriticalLevelL( &rfs, filesize, imsDrive ) )
            {
            iDrive = imsDrive;            
            }
        }
    
    if ( iDrive == EDriveZ)
        {
        err = rfs.Volume(volumeInfo, mmcDrive);
        if ( !err )
            {
            // Check capacity on Internal mass storage    
            TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL Checking memory card\t" )) );
            if ( !SysUtil::DiskSpaceBelowCriticalLevelL( &rfs, filesize, mmcDrive ) )
                {                    
                iDrive = mmcDrive;
                }   
            }
        }           
    if ( iDrive == EDriveZ )
        {
        TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL Checking phone memory\t" )) );
        // Phone memory
        if( !SysUtil::DiskSpaceBelowCriticalLevelL( &rfs, filesize, EDriveC ))
            {
            iDrive = EDriveC;
            }
        }
    rfs.Close();
    TRACE_INFO( (_L( "[oppreceiveservice] CheckCapacityL iDrive = %d\t" ),iDrive ) );
    TRACE_FUNC_EXIT
    if (iDrive == EDriveZ)
        {
        // If there is no free space for receiving file, we need to set iPreviousDefaultFolder back to iDefaultFolder.
        // In order to show the file receveing dialog correctly.
        iDefaultFolder = iPreviousDefaultFolder;
        return EFalse;
        }
    return ETrue;
    }    

// ---------------------------------------------------------
// IsOBEXActive()
// ---------------------------------------------------------
//	    
TBool COPPController::IsOBEXActive()
    {
    TRACE_FUNC
    return ETrue;
    }

// ---------------------------------------------------------
// SetMediaType()
// ---------------------------------------------------------
//
void COPPController::SetMediaType( TSrcsMediaType aMediaType ) 
    {
    TRACE_FUNC    
    iMediaType=aMediaType;    
    }
    
// ---------------------------------------------------------
// SetObexServer()
// ---------------------------------------------------------
//
TInt COPPController::SetObexServer( CObexServer* aServer)
	{	
	TInt retVal=KErrNone;    
	
	if (aServer)
	    {  
	    iObexServer = aServer;
	    retVal=aServer->Start(this);    
	    }
	return retVal;
	}
   
// ---------------------------------------------------------
// CancelTransfer()
// ---------------------------------------------------------
//
void COPPController::CancelTransfer()
    {
    TRACE_FUNC
    CloseReceivingIndicator();
    if(iObexTransferState == ETransferPut)
        {
        iObexTransferState = ETransferPutCancel;
        }
    else // go to idle for all other states
        {
        iObexTransferState = ETransferIdle;
        }
    }

void COPPController::LaunchReceivingIndicatorL()
    {
    if(ReceivingIndicatorActive())
        {
        return;
        }
    
    if(iTotalSizeByte > 0)
        {
        if(iReceivingFileName.Length() > 0)
            {
            iProgressDialog = CHbDeviceDialogSymbian::NewL();
            iProgressDialog->SetObserver(this);

            CHbSymbianVariantMap* variantMap = CHbSymbianVariantMap::NewL();
            CleanupStack::PushL(variantMap);
            
            TInt dialogIdx = TBluetoothDialogParams::EReceiveProgress;
            CHbSymbianVariant* dialogType = CHbSymbianVariant::NewL( (TAny*) &(dialogIdx), 
                                                                CHbSymbianVariant::EInt );
            CleanupStack::PushL(dialogType);
            TBuf16<6> dialogTypeKey;
            dialogTypeKey.Num(TBluetoothDialogParams::EDialogType);
            User::LeaveIfError(variantMap->Add(dialogTypeKey, dialogType));
            CleanupStack::Pop(dialogType);
            
            CHbSymbianVariant* deviceName = CHbSymbianVariant::NewL( (TAny*) (&iRemoteDeviceName), 
                                                                CHbSymbianVariant::EDes );
            CleanupStack::PushL(deviceName);
            TBuf16<6> deviceNameKey;
            deviceNameKey.Num(TBluetoothDeviceDialog::EDeviceName);
            User::LeaveIfError(variantMap->Add(deviceNameKey, deviceName));
            CleanupStack::Pop(deviceName);
            
            CHbSymbianVariant* fileName = CHbSymbianVariant::NewL( (TAny*) (&iReceivingFileName), 
                                                                CHbSymbianVariant::EDes );
            CleanupStack::PushL(fileName);
            TBuf16<6> fileNameKey;
            fileNameKey.Num(TBluetoothDeviceDialog::EReceivingFileName);
            User::LeaveIfError(variantMap->Add(fileNameKey, fileName));
            CleanupStack::Pop(fileName);
            
            CHbSymbianVariant* fileSz = CHbSymbianVariant::NewL( (TAny*) &iTotalSizeByte, 
                                                                CHbSymbianVariant::EInt );
            CleanupStack::PushL(fileSz);
            TBuf16<6> fileSzKey;
            fileSzKey.Num(TBluetoothDeviceDialog::EReceivingFileSize);
            User::LeaveIfError(variantMap->Add(fileSzKey, fileSz));
            CleanupStack::Pop(fileSz);
            
            CHbSymbianVariant* fileCnt = CHbSymbianVariant::NewL( (TAny*) &iFileCount, 
                                                                CHbSymbianVariant::EInt );
            CleanupStack::PushL(fileCnt);
            TBuf16<6> fileCntKey;
            fileCntKey.Num(TBluetoothDeviceDialog::EReceivedFileCount);
            User::LeaveIfError(variantMap->Add(fileCntKey, fileCnt));
            CleanupStack::Pop(fileCnt);
            
            iDialogActive = ETrue;
            iProgressDialog->Show( KBTDevDialogId(), *variantMap, this );
            CleanupStack::PopAndDestroy(variantMap);
            }
        else
            {
            if(iMediaType == ESrcsMediaBT)
                {
                //TODO - Remove the usage of the resources.
                //iProgressDialog->ShowProgressDialogL(R_BT_RECEIVING_DATA);
                }
            TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported);
            }
        }
    else
        {
    
        /*
         * TODO - The functionality provided by CGlobalDialog will be removed
         * TODO - and this will be provided by CGlobalProgressDialog.
         */
        if(iMediaType == ESrcsMediaBT)
            {
            // TODO 
            }
        TRACE_ASSERT(iMediaType != ESrcsMediaIrDA, KErrNotSupported);
        }
    }

void COPPController::UpdateReceivingIndicatorL()
    {
    if(iProgressDialog)
        {
        CHbSymbianVariantMap* variantMap = CHbSymbianVariantMap::NewL();
        CleanupStack::PushL(variantMap);
        
        TInt bytesReceived = iObexObject->BytesReceived();
        CHbSymbianVariant* progress = CHbSymbianVariant::NewL( (TAny*) &bytesReceived, CHbSymbianVariant::EInt );
        CleanupStack::PushL(progress);
        User::LeaveIfError(variantMap->Add(_L("progress"), progress));
        CleanupStack::Pop(progress);
        
        iProgressDialog->Update(*variantMap);
        CleanupStack::PopAndDestroy(variantMap);
        }
    }

void COPPController::CloseReceivingIndicator(TBool aResetDisplayedState)
    {
    TRACE_FUNC
    if(aResetDisplayedState)
        {
        iNoteDisplayed = EFalse;
        }
    
    if(iProgressDialog)
        {
        iProgressDialog->Cancel();
        iDialogActive = EFalse;
        delete iProgressDialog;
        iProgressDialog = NULL;
        }
    }

 // ---------------------------------------------------------
  // GetDriveWithMaximumFreeSpace()
  // ---------------------------------------------------------
  // 
  TInt COPPController::GetDriveWithMaximumFreeSpaceL()
      {
      // Get drive with maximum freespace among phone memory, MMC, internal mass storage.
      //
      TRACE_FUNC   
      TVolumeInfo volumeInfoC;
      TVolumeInfo volumeInfoE;
      TVolumeInfo volumeInfoF;
      TInt64 max = 0;
      TInt drive = 0;
      
      TInt err = iFs.Volume(volumeInfoC, EDriveC);
     
      if ( !err )
          {
          // set initial values to max and drive.
          max = volumeInfoC.iFree;
          drive = EDriveC;
          }
           
      err = iFs.Volume(volumeInfoE, EDriveE);     
      if ( !err )
          {
          if (volumeInfoE.iFree >= max)
              {
              max = volumeInfoE.iFree;
              drive = EDriveE;             
              }
          
          }
           
      err = iFs.Volume(volumeInfoF, EDriveF);
      if ( !err )
          {
          if (volumeInfoF.iFree >= max)
              {
              max = volumeInfoF.iFree;
              drive = EDriveF;             
              }
          }
      
      max = 0;
      return drive;
      }
 
 
 // ----------------------------------------------------------
 // COPPController::HandleGetDevicesComplete
 // Callback from devman
 // ----------------------------------------------------------
 //
 void COPPController::HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* aDeviceArray)
    {
    TRACE_INFO( _L( "[oppreceiveservice] HandleGetDevicesComplete: enter \t" ) );
    
    (void) aDeviceArray;
    
    if ( aErr == KErrNone )
        {
        if ( iResultArray->Count())
            {             
            iRemoteDeviceName.Zero();
            if ( iResultArray->At(0)->FriendlyName().Length() > 0 )
                {
                TRACE_INFO( _L( "[oppreceiveservice] HandleGetDevicesComplete: got friendly name \t" ) );
                iRemoteDeviceName.Copy(iResultArray->At(0)->FriendlyName());
                }
            else
                {
                TRACE_INFO( _L( "[oppreceiveservice] HandleGetDevicesComplete: got devciename name \t" ));
                TRAP_IGNORE(iRemoteDeviceName.Copy( BTDeviceNameConverter::ToUnicodeL(iResultArray->At(0)->DeviceName())));
                }
            }
        }
    }
 TBool COPPController::IsBackupRunning()
    {
    const TUint32 KFileManagerBkupStatus = 0x00000001;
    
    TInt status = EFileManagerBkupStatusUnset;
    TBool retValue = EFalse;
    TInt err = RProperty::Get( TUid::Uid(KFileManagerUID3), KFileManagerBkupStatus,
                              status );
    if ( err == KErrNone )
        {
        if ( status == EFileManagerBkupStatusBackup || 
             status == EFileManagerBkupStatusRestore )
            {
            TSecureId fileManagerSecureId( KFileManagerUID3 );
            //only returning ETrue if backup process is still active
            retValue = ProcessExists( fileManagerSecureId );
            }
        }
   
    return retValue;
    }
 
 TBool COPPController::ProcessExists( const TSecureId& aSecureId )
     {
     _LIT( KFindPattern, "*" );
     TFindProcess finder(KFindPattern);
     TFullName processName;
     while( finder.Next( processName ) == KErrNone )
         {
         RProcess process;
         if ( process.Open( processName ) == KErrNone )
             {
             TSecureId processId( process.SecureId() );
             process.Close();
             if( processId == aSecureId )
                 {
                 return ETrue;
                 }
             }
         }
     return EFalse;
     }
 
 void COPPController::DialogDismissed(TInt aButtonId)
     {
     (void) aButtonId;
     }

 void COPPController::DataReceived(CHbSymbianVariantMap& aData)
     {
     if(aData.Keys().MdcaPoint(0).Compare(_L("actionResult")) == 0)
         {
         TInt val = *(static_cast<TInt*>(aData.Get(_L("actionResult"))->Data()));
         switch(val)
             {
             case TBluetoothDialogParams::ECancelReceive:
                 {
                 //User choose to cancel receiving.
                 CancelTransfer();
                 }break;
                 
             case TBluetoothDialogParams::EHide:
                 {
                 //Use choose to hide the progress dialog.
                 CloseReceivingIndicator(EFalse);
                 }break;
                 
             case TBluetoothDialogParams::EShow:
             case TBluetoothDialogParams::ECancelShow:
                 {
                 //In case of Show, the device dialog will handle the opening of conversation view.
                 iRecvDoneDialog->Cancel();
                 delete iRecvDoneDialog;
                 iRecvDoneDialog = NULL;
                 }break;
             }
         }
     }
 
 void COPPController::DeviceDialogClosed(TInt aCompletionCode)
     {
     (void) aCompletionCode;
     }

//////////////////////////// Global part ////////////////////////////

//  End of File  
