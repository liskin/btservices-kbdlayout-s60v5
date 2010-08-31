/*
* Copyright (c) 2002,2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


// INCLUDE FILES
#include "obexutilsmessagehandler.h"
#include "obexutilsdebug.h"
#include "obexutilsentryhandler.h"

#include <msvids.h>
#include <msvuids.h>
#include <apgcli.h>
#include <txtrich.h>

#include <biouids.h>
#include <bautils.h>

#include <e32property.h>
#include <UikonInternalPSKeys.h>

#include <mmsvattachmentmanagersync.h>
#include <mmsvattachmentmanager.h>

#include <centralrepository.h>
#include <btengdomaincrkeys.h>
#include "updatemusiccollection.h"

#include <pathinfo.h>          // provides interface for quering system paths 

#ifdef __BT_SAP
 #include "vMessageHandler.h"
#endif // __BT_SAP

// CONSTANT
const TInt KFFSBelowCritical = -2;
const TInt KRenameOffSet = 4 ;

// todo @ QT migration: take official definition from Messaging at app layer (btmsgtypeuid.h)
const TUid KUidMsgTypeBt = {0x10009ED5};

const TInt KObexUtilsMaxCharToFromField = 256;

const TInt32 KUidMsgTypeBtTInt32 = 0x10009ED5;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RemoveObexBuffer  Removes Obex buffer object.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::RemoveObexBuffer(CObexBufObject* &aBufObject)
    {
    if (aBufObject)
        {
        delete aBufObject;
        aBufObject = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CreateMsvSessionLC  Creates MSV session
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::CreateMsvSessionLC(
    CDummySessionObserver* &aSessionObs,
    CMsvSession* &aMsvSession)
    {
    aSessionObs = new( ELeave )CDummySessionObserver;
    CleanupStack::PushL( aSessionObs );  //1st push
    aMsvSession = CMsvSession::OpenSyncL( *aSessionObs );
    CleanupStack::PushL( aMsvSession );  //2nd push
    }

// -----------------------------------------------------------------------------
// CreateMsvSessionLC  Creates MSV session to selected destination
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::CreateMsvSessionLC(
    CDummySessionObserver* &aSessionObs,
    CMsvSession* &aMsvSession,
    CMsvEntry* &aDestination,
    TMsvId aMsvId)
    {
    CreateMsvSessionLC(aSessionObs, aMsvSession);
    aDestination = aMsvSession->GetEntryL( aMsvId );
    CleanupStack::PushL( aDestination );  //3rd push
    }

// -----------------------------------------------------------------------------
// GetCenRepKeyIntValueL Gets integer repository value
// Returns: Error
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsMessageHandler::GetCenRepKeyIntValueL(
    TUid aRepositoryUid,  // aRepositoryUid Central repository's UID value
    TUint32 aId,          // Central repository's ID value
    TInt& aVal)           // Returned integer value
    {
    CRepository* storage = CRepository::NewLC( aRepositoryUid );
    TInt retval = storage->Get( aId, aVal );
    if (retval != KErrNone)
        {
        FLOG(_L("[OBEXUTILS]\t GetCenRepKeyIntValueL() Get failed"));
        }
    CleanupStack::PopAndDestroy( storage );
    return retval;
    }

// -----------------------------------------------------------------------------
// GetCurrentTime Gets current time
// Returns: Current time
// -----------------------------------------------------------------------------
//
TTime TObexUtilsMessageHandler::GetCurrentTime()
    {
    TTime time;
    time.UniversalTime();
    return time;
    }

// -----------------------------------------------------------------------------
// StoreAsRichTextL Change message content to Richtext
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::StoreAsRichTextL( 
    CMsvEntry* aParentEntry,  // A pointer to new message server entry.
    RFile& aFile)             // New file to store
    {
    FLOG(_L("[OBEXUTILS]\t StoreAsRichTextL()"));

    TInt fileLength = 0;
    User::LeaveIfError( aFile.Size( fileLength ) );

    // Calculate the number of blocks to read
    const TInt blockSize = 128 * 1024;
    TInt fullBlockCnt = fileLength / blockSize;
    if (fileLength % blockSize > 0)
        {
        fullBlockCnt++;
        }

    CRichText* richText = ConstructRichTextLC(); // 1st, 2nd, 3rd push

    RBuf8 buffer8;
    CleanupClosePushL(buffer8);  // 4th push
    buffer8.CreateL( blockSize );
    RBuf buffer16;
    CleanupClosePushL(buffer16); // 5th push
    buffer16.CreateL(blockSize);
    
    // Read the file into buffer in small chunks  
    TInt readBytes = 0;
    for (TInt i = 0; i < fullBlockCnt; ++i)
        {
        User::LeaveIfError( aFile.Read( buffer8 ) );
    
        // In order for Bio to understand 8-bit binary data, 
        // it needs to be stored in wide rich text object
        // in the low byte of each word with the upper byte as 0.
        // Do not use UTF8, just convert 8-bit to 16-bit.  
        buffer16.Copy( buffer8 );
        
        richText->InsertL(readBytes, buffer16);
        readBytes += buffer8.Length();
        }
    CleanupStack::PopAndDestroy(2, &buffer8);
    // -2 pop: free unused memory before StoreBodyTextL()

    CMsvStore* parentStore = aParentEntry->EditStoreL();
    CleanupStack::PushL(parentStore);   // 4th push
    parentStore->StoreBodyTextL(*richText);
    parentStore->CommitL();

    CleanupStack::PopAndDestroy(4);  // parentStore, richText, richParaFormatLayer,
                                     // richCharFormatLayer

    FLOG(_L("[OBEXUTILS]\t StoreAsRichTextL() completed"));
    }

// -----------------------------------------------------------------------------
// UpdateAttachmentInformationL Add the obex object as a file attachment.
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::UpdateAttachmentInformationL(
    TReceivedData& aRecData,      // The obex received data
    CMsvEntry* aAttachEntry,      // Attachment entry
    CMsvAttachment* aAttachInfo,  // Attachment information
    CMsvStore* aStore)            // Message store (attachment)
    {
    FLOG(_L("[OBEXUTILS]\t UpdateAttachmentInformationL()"));

    TMsvEntry attachTEntry = aAttachEntry->Entry();

    SetDetailsLC(attachTEntry.iDescription, aAttachInfo->MimeType());
    attachTEntry.iSize = aRecData.bytesReceived;
    attachTEntry.iDate = aRecData.recTime;
    aAttachEntry->ChangeL(attachTEntry);  // Save properties.
    CleanupStack::PopAndDestroy(); // unicode in SetDetailsLC

    aAttachInfo->SetSize(aRecData.bytesReceived);
    aAttachInfo->SetAttachmentNameL(aRecData.msgName);
    aAttachInfo->SetComplete(ETrue);

    aStore->AttachmentManagerExtensionsL().ModifyAttachmentInfoL(aAttachInfo);
    
    FLOG(_L("[OBEXUTILS]\t UpdateAttachmentInformation() completed"));
    }

// -----------------------------------------------------------------------------
// FinaliseMessageL Save the size of all the attachments and make visible.
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::FinaliseMessageL(
    CMsvEntry* aParentEntry,  // Parent entry
    TReceivedData& aRecData,  // The obex received data
    CMsvStore* aStore,        // Attachment's store
    TBool aIsBioMsg)          // Is BIO message?
    {
    FLOG(_L("[OBEXUTILS]\t FinaliseMessageL()"));
    // Save the size of all the attachment & make visible.   
    TMsvEntry parentTEntry = aParentEntry->Entry();
    parentTEntry.iDate = GetCurrentTime();
    parentTEntry.iSize = aRecData.bytesReceived;
    parentTEntry.SetUnread(ETrue);
    
    // Saved OK. Make the entry visible and flag it as complete.
    parentTEntry.SetVisible(ETrue);                   // visible
    parentTEntry.SetInPreparation(EFalse);            // complete
    if (!aIsBioMsg)
        {
        parentTEntry.iDescription.Set(aRecData.msgName);  // "Subject"
        }
    aParentEntry->ChangeL(parentTEntry);              // commit changes

    aStore->CommitL();
    
    FLOG(_L("[OBEXUTILS]\t FinaliseMessageL() completed"));
    }

// -----------------------------------------------------------------------------
// FinaliseMessageL Save the size of all the attachments and make visible.
// for Saving file received via BT/IR directly to file system.
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::FinaliseMessageL(
    CMsvEntry* aParentEntry,
    CMsvStore* aStore,
    TEntry& aFileEntry,
    TParse& aFileNameParser)
    {
    FLOG(_L("[OBEXUTILS]\t FinaliseMessageL() new "));
    
    // Save the size of all the attachment & make visible.
    TMsvEntry parentTEntry = aParentEntry->Entry();
    parentTEntry.iDate = GetCurrentTime();
    parentTEntry.iSize = aFileEntry.iSize;  
    // Saved OK. Make the entry visible and flag it as complete.
    parentTEntry.SetVisible(ETrue);                   // visible
    parentTEntry.SetUnread(ETrue);
    parentTEntry.SetInPreparation(EFalse);            // complete
    parentTEntry.iDescription.Set(aFileNameParser.FullName());        // "Subject" 
    aParentEntry->ChangeL(parentTEntry);              // commit changes
    aStore->CommitL();
    
    FLOG(_L("[OBEXUTILS]\t FinaliseMessageL() new completed "));
    }

// -----------------------------------------------------------------------------
// StoreAndUpdateBioMessageL Stores and updates a BIO message
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::StoreAndUpdateBioMessageL(
    CBIODatabase* aBioDB,     // Bio database
    TUid aBioMsgId,           // Bio message ID
    CMsvEntry* parentEntry,   // Parent message entry
    RFile& aFile)             // File to attachment
    {
    FLOG(_L("[OBEXUTILS]\t StoreAndUpdateBioMessageL()"));
    
    TInt index;
    aBioDB->GetBioIndexWithMsgIDL(aBioMsgId, index);
    
    StoreAsRichTextL( parentEntry, aFile );
    
    TMsvEntry parentTEntry = parentEntry->Entry();
    parentTEntry.iMtm = KUidBIOMessageTypeMtm;
    parentTEntry.iServiceId = KMsvLocalServiceIndexEntryId;  
    parentTEntry.iBioType = aBioMsgId.iUid;
    parentTEntry.iMtmData1 = KUidMsgTypeBtTInt32;
    parentTEntry.iDescription.Set(aBioDB->BifReader(index).Description());
    parentEntry->ChangeL(parentTEntry);
    
    FLOG(_L("[OBEXUTILS]\t StoreAndUpdateBioMessageL() completed"));
    }

// -----------------------------------------------------------------------------
// RecogniseObjectsL Recognised objects if no type field has been given.
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::RecogniseObjectsL(
    TFileName& aFileName,
    CMsvAttachment* aAttachInfo)  // Attachment information
    {
    FLOG(_L("[OBEXUTILS]\t RecogniseObjectsL()"));

    TPtrC8 mimeType;
    RApaLsSession lsSess;
    User::LeaveIfError( lsSess.Connect() );
    CleanupClosePushL(lsSess);
    TDataType appDataType;
    TUid appUid;
    if (lsSess.AppForDocument(aFileName, appUid, appDataType) == KErrNone)
        {
        mimeType.Set(appDataType.Des8());
        }
    else
        {
        // Don't trust on sender's MIME type here. Just set to empty.
        mimeType.Set(KNullDesC8);
        FLOG(_L("[OBEXUTILS]\t RecogniseObjectsL() MIME check failed"));
        }
    CleanupStack::PopAndDestroy();  // lsSess
    HBufC* buf16 = HBufC::NewLC(mimeType.Length());
    buf16->Des().Copy(mimeType);
    TPtrC mimeType16(buf16->Des());
    CleanupStack::PopAndDestroy();   //   buf16
    
    CUpdateMusicCollection* updateMusicCollection = CUpdateMusicCollection::NewL();
    CleanupStack::PushL(updateMusicCollection);    
    if (updateMusicCollection->isSupported(mimeType16))
        {
        updateMusicCollection->addToCollectionL(aFileName);
        }
    CleanupStack::PopAndDestroy();  // updateMusicCollection
    
    aAttachInfo->SetMimeTypeL( mimeType );
    
    FLOG(_L("[OBEXUTILS]\t RecogniseObjectsL() completed"));
    }

// -----------------------------------------------------------------------------
// SaveMimeAndGetBioLC Saves MIME type and gets BIO database for that MIME
// Returns: BIO database if BIO message
// -----------------------------------------------------------------------------
//
CBIODatabase* TObexUtilsMessageHandler::SaveMimeAndGetBioLC(
    TFileName& aFileName,
    RFs& aFsSess,                 // File session
    CMsvAttachment* aAttachInfo,  // Attachment information
    TUid &aBioMsgId)              // BIO message ID for data
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveMimeAndGetBioLC()"));
    
    RecogniseObjectsL( aFileName, aAttachInfo );

    // For attachments / BIO check.
    CBIODatabase* bioDB = CBIODatabase::NewLC( aFsSess );  // 1st push
    
    // Run through the data objects, seeing if they can be accepted by BIO.
    // Check if they are bio objects

    TBioMsgIdText bioMessageIdText;
    SetBioDataTextL( bioMessageIdText, aAttachInfo->MimeType() );

    // Remove any zero termination on mime type
    //
    TInt zeroTermination = bioMessageIdText.Locate( TChar(0) ); 
    
    if ( zeroTermination != KErrNotFound )
        {
        bioMessageIdText.SetLength( zeroTermination );
        }

    TInt bioMsgErr = bioDB->IsBioMessageL( EBioMsgIdIana, bioMessageIdText, NULL, aBioMsgId );
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveMimeAndGetBioLC() completed"));
    if (bioMsgErr != KErrNone)
        {
        return NULL;
        }
    return bioDB;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CreateInboxAttachmentL Creates inbox entry and attachment for it
// If this leaves, it means the entry and obex object have already been deleted.
// In this case aMsvIdParent has been set to zero.
// Returns: None.
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::CreateInboxAttachmentL(
    CObexBufObject* &aBufObject,  // Obex object
    const TUid aMtmId,            // Mtm ID of the message
    TMsvId &aMsvIdParent,         // Message ID of the parent message entry
    RFile &aFile)                 // File to the attachment
    {
    TMsvId msvId = 0;
    TRAPD(retTrap, DoCreateInboxAttachmentL(aMtmId, msvId, aFile,KNullDesC))
    if (retTrap != KErrNone)
        {
        aMsvIdParent = 0;
        if (msvId)
            {
            RemoveInboxEntriesL(aBufObject, msvId);
            }
        User::Leave(retTrap);
        }
    aMsvIdParent = msvId;
    }

// -----------------------------------------------------------------------------
// CreateInboxAttachmentL Creates inbox entry and attachment for it
// If this leaves, it means the entry and obex object have already been deleted.
// In this case aMsvIdParent has been set to zero.
// Returns: None.
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::CreateInboxAttachmentL(
    CObexBufObject* &aBufObject,  // Obex object
    const TUid aMtmId,            // Mtm ID of the message
    TMsvId &aMsvIdParent,         // Message ID of the parent message entry
    RFile &aFile,                 // File to the attachment
    const TDesC& aRemoteBluetoothName)  // bluetooth name of remote device
    {
    TMsvId msvId = 0;
    TRAPD(retTrap, DoCreateInboxAttachmentL(aMtmId, msvId, aFile,aRemoteBluetoothName))
    if (retTrap != KErrNone)
        {
        aMsvIdParent = 0;
        if (msvId)
            {
            RemoveInboxEntriesL(aBufObject, msvId);
            }
        User::Leave(retTrap);
        }
    aMsvIdParent = msvId;
    }

// -----------------------------------------------------------------------------
// DoCreateInboxAttachmentL Creates inbox entry and attachment for it (private)
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::DoCreateInboxAttachmentL(
    const TUid aMtmId,     // Mtm ID of the message
    TMsvId &aMsvIdParent,  // Message ID of the parent message entry
    RFile &aFile,          // File to the attachment
    const TDesC& aRemoteBluetoothName)  // bluetooth name of remote device
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateInboxAttachmentL()"));
    
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CMsvEntry* inbox;
    CreateMsvSessionLC(sessionObs, msvSession, inbox, KMsvGlobalInBoxIndexEntryId);
    // 1st, 2nd, 3rd push

    TBuf<KObexUtilsMaxCharToFromField> toFrom;

    // todo @ QT migration: load from string id for localization
    if (aMtmId == KUidMsgTypeBt)
        {
        _LIT(KBluetoothMsg, "Bluetooth message");
        toFrom.Copy(KBluetoothMsg);
        }
    else
        {
        _LIT(KInfraredMsg, "Infrared message");
        toFrom.Copy(KInfraredMsg);    
        }
    
    TMsvEntry parentTEntry;
    parentTEntry.iMtm = aMtmId;
    parentTEntry.SetVisible(EFalse); // Make invisible..
    parentTEntry.SetUnread(ETrue);
    
    // ...and in preparation to make sure it gets cleaned up on errors.
    //
    parentTEntry.SetInPreparation(ETrue); 
    parentTEntry.iType = KUidMsvMessageEntry;
    parentTEntry.iServiceId = KMsvUnknownServiceIndexEntryId;
    // iDate & iSize will be set later
    if ( aRemoteBluetoothName.Length() > 0 )
        {
        parentTEntry.iDetails.Set(aRemoteBluetoothName);
        }
    else
        {
        parentTEntry.iDetails.Set(toFrom);
        }
    inbox->CreateL(parentTEntry);
    
    aMsvIdParent = parentTEntry.Id();
    
    // Create attachment for the inbox entry
    
    TMsvEntry attachTEntry;
    attachTEntry.iType      = KUidMsvAttachmentEntry;
    attachTEntry.iServiceId = KMsvUnknownServiceIndexEntryId;   
    attachTEntry.iMtm       = aMtmId;
    
    CMsvEntry* parentEntry = msvSession->GetEntryL(aMsvIdParent);
    CleanupStack::PushL(parentEntry);  // 4th push
    parentEntry->CreateL(attachTEntry);  // create attachment as child of parent
    
    // create the v2 attachment, save RFile
    
    CMsvEntry* attachEntry = msvSession->GetEntryL(attachTEntry.Id());
    CleanupStack::PushL(attachEntry);  // 5th push
    
    CMsvStore* store = attachEntry->EditStoreL();
    CleanupStack::PushL(store);  // 6th push
    CMsvAttachment* attachment = CMsvAttachment::NewL(CMsvAttachment::EMsvFile);
    CleanupStack::PushL(attachment);  // 7th push
     
    store->AttachmentManagerExtensionsL().CreateAttachmentL(_L("Unnamed"), aFile, attachment);
    CleanupStack::Pop(attachment);
    store->CommitL();
    // CommitL() required here otherwise EditStoreL() will fail in SaveObjToInbox()
    
    CleanupStack::PopAndDestroy(6);
    // store, attachEntry, parentEntry, inbox, msvSession, sessionObs

    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateInboxAttachmentL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::RemoveInboxEntriesL
// -----------------------------------------------------------------------------
EXPORT_C void TObexUtilsMessageHandler::RemoveInboxEntriesL(
    CObexBufObject* &aBufObject, // Obex object
    TMsvId aMsvIdParent)         // Message ID of the parent message entry
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::RemoveInboxEntriesL()"));
    
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CMsvEntry* inbox;
    CreateMsvSessionLC(sessionObs, msvSession, inbox, KMsvGlobalInBoxIndexEntryId);
    // 1st, 2nd, 3rd push
    
    // Close file
    
    RemoveObexBuffer(aBufObject);
    
    // Delete parent and descendants
    if ( aMsvIdParent )
        {
        inbox->DeleteL(aMsvIdParent);
        }
    
    CleanupStack::PopAndDestroy(3);
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::RemoveInboxEntriesL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::SaveObjToInboxL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::SaveObjToInboxL( 
    CObexBufObject* /*aReceivedObject*/, const TDesC16& /*aPath*/, const TUid /*aMtmID*/ )
    {
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::SaveObjToInboxL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::SaveObjToInboxL(
    CObexBufObject* &aReceivedObject, // Obex object
    RFile  &aFile,         
    TMsvId aMsvIdParent)              // Message ID of the parent message entry
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL()"));

    // Open session. 
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CreateMsvSessionLC(sessionObs, msvSession);  // 1st, 2nd push

    CMsvEntry* parentEntry = msvSession->GetEntryL(aMsvIdParent);
    CleanupStack::PushL( parentEntry );  // 3rd push

#ifdef __BT_SAP
    if(CSapVMessageParser::HandleMessageL(aReceivedObject, parentEntry->Entry().iMtm,
        aFile, msvSession, GetCurrentTime()))
        {
        RemoveObexBuffer(aReceivedObject);
        CMsvEntry* inbox = msvSession->GetEntryL( KMsvGlobalInBoxIndexEntryId );
        CleanupStack::PushL( inbox );         //4th push
        inbox->DeleteL(aMsvIdParent);
        // Close session etc.
        CleanupStack::PopAndDestroy(4, sessionObs);  // inbox, parentEntry, session, sessionObs
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL() completed"));
        return;
        }
#endif // __BT_SAP
    CMsvEntry* attachEntry = msvSession->GetEntryL(((*parentEntry)[0]).Id());
    CleanupStack::PushL(attachEntry); // 4th push

    CMsvStore* store = attachEntry->EditStoreL();
    CleanupStack::PushL( store );  // 5th push

    // Collect obex received data before obex buffer deletion

    TReceivedData receivedData;
    receivedData.bytesReceived = aReceivedObject->BytesReceived();
    receivedData.recTime = aReceivedObject->Time();

    RFs& fsSess = msvSession->FileSession();
    receivedData.msgName = aReceivedObject->Name();
    MakeValidFileName(fsSess, receivedData.msgName);

    // Rename attachment for MIME type checking
    
    RemoveObexBuffer(aReceivedObject);
    store->AttachmentManagerExtensionsL().RenameAttachmentL(0, receivedData.msgName);
    RFile newFile = store->AttachmentManagerL().GetAttachmentFileL(0);
    CleanupClosePushL(newFile);  // 6th push
    
    // RenameAttachmentL() modified attachment info, so read it now
    
    CMsvAttachment* attachInfo = store->AttachmentManagerL().GetAttachmentInfoL(0);
    CleanupStack::PushL( attachInfo ); // 7th push

    TUid bioMsgId;  // SaveMimeAndGetBioLC modifies this
    TFileName fileName;
    newFile.FullName(fileName);
    CBIODatabase* bioDB = SaveMimeAndGetBioLC( fileName, fsSess, attachInfo, bioMsgId );
    // 8th push
    if (bioDB)  // bio message
        {
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL() BIO"));
        StoreAndUpdateBioMessageL( bioDB, bioMsgId, parentEntry, newFile );
        FinaliseMessageL(parentEntry, receivedData, store, ETrue);
        TMsvId attachId = attachEntry->EntryId();
        CleanupStack::PopAndDestroy(5);  // bioDB, attachInfo, newFile (close),
                                         // store, attachEntry
        // PopAndDestroying here because open store locks the entry,
        // it can be deleted only after destroying the store pointer
        parentEntry->DeleteL(attachId);  // invalidate unused attachment
        CleanupStack::PopAndDestroy(3);  // parentEntry, msvSession, sessionObs
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL() BIO completed"));
        }
    else  // attachment message
        {
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL() Attachment"));
        CleanupStack::PopAndDestroy();   // bioDB
        UpdateAttachmentInformationL(receivedData, attachEntry, attachInfo, store);
        CleanupStack::Pop(attachInfo);   // ownership taken by ModifyAttachmentInfoL()
        FinaliseMessageL(parentEntry, receivedData, store, EFalse);
        CleanupStack::PopAndDestroy(6);  // newFile (close), store, attachEntry,
                                         // parentEntry, msvSession, sessionObs
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL() Attachment completed"));
        }

    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveObjToInboxL() completed"));
    }


// -----------------------------------------------------------------------------
// GetCenRepKeyStringValueL Gets integer repository value
// Returns: Error
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsMessageHandler::GetCenRepKeyStringValueL(
    TUid aRepositoryUid,  // aRepositoryUid Central repository's UID value
    TUint32 aId,          // Central repository's ID value
    TDes& aVal)           // Returned String
    {
    CRepository* storage = CRepository::NewLC( aRepositoryUid );
    TInt retval = storage->Get( aId, aVal );
    if (retval != KErrNone)
        {
        FLOG(_L("[OBEXUTILS]\t GetCenRepKeyStringValueL() Get failed"));
        }
    CleanupStack::PopAndDestroy( storage );
    return retval;
    }

// -----------------------------------------------------------------------------
// CreateReceiveBufferL Creates an Obex receive buffer object and RFile object.
// Returns: Symbian error code
// -----------------------------------------------------------------------------
//

EXPORT_C TInt TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL(
    RFile& aFile,
    const TDesC& aPath,
    TFileName& aFullName, 
    CBufFlat* &aReceiveObject, 
    const TInt aBufSize )  
    {
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL()"));
    
    aReceiveObject = CBufFlat::NewL(aBufSize); 
    aReceiveObject->ResizeL(aBufSize);
    
    
    RFs rfs;
    User::LeaveIfError( rfs.Connect() );
    if ( !BaflUtils::FolderExists( rfs, aPath ) )
        {
        User::LeaveIfError(rfs.MkDirAll(aPath));
        }
    User::LeaveIfError( aFile.Temp( rfs, aPath, aFullName, EFileWrite) );
    // Set temp file  hidden file.
    //
    TUint setAttMask(0);
    TUint clearAttMask(0);
    aFile.SetAtt(setAttMask | KEntryAttHidden, clearAttMask);
    aFile.Flush();
    aFile.Close();
    
    RFile logFile;
    TFileName logFilePath;
    TFileName cenRepFolder;
    GetCenRepKeyStringValueL(KCRUidBluetoothEngine, KLCReceiveFolder,cenRepFolder);
    
    logFilePath.Zero();
    logFilePath.Append(PathInfo::PhoneMemoryRootPath());
    logFilePath.Append(cenRepFolder);
    
    if ( !BaflUtils::FolderExists(rfs , logFilePath) )
        {
        User::LeaveIfError(rfs.MkDirAll(logFilePath));
        }
    logFilePath.Append(_L("log.txt"));
    if ( !BaflUtils::FileExists( rfs, logFilePath ))
        {
        logFile.Create(rfs, logFilePath, EFileWrite);
        }
    else
        {
        User::LeaveIfError(logFile.Open(rfs, logFilePath, EFileWrite));
        }
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL()    log.txt is created"));
    TFileText ft;              
    ft.Set(logFile);
    ft.Write(aFullName);    // restore tempfilename in logfile

    // Set log file 
    logFile.SetAtt(setAttMask | KEntryAttHidden, clearAttMask);
    logFile.Flush();
    logFile.Close();
    rfs.Close();
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateReceiveBufferAndRFileL() completed"));
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::RemoveTemporaryRFileL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::RemoveTemporaryRFileL ( const TFileName& aFullName )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::RemoveTemporaryRFileL()"));
    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    
    TParse fileNameParser;
    User::LeaveIfError(fileNameParser.Set(aFullName, NULL, NULL));
    
    TFileName logFilePath;
    TFileName cenRepFolder;
    GetCenRepKeyStringValueL(KCRUidBluetoothEngine, KLCReceiveFolder,cenRepFolder);
    logFilePath.Zero();
    logFilePath.Append(PathInfo::PhoneMemoryRootPath());
    logFilePath.Append(cenRepFolder);
    logFilePath.Append(_L("log.txt"));
            
    if ( fileNameParser.Ext().Compare(_L(".$$$")) == 0  )                                                                           
        {
        // aFullName is a temp file name, we need to remove it.        
        rfs.Delete( aFullName );
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::RemoveTemporaryRFileL() temp file is removed (bt off)"));
        }
    else if ( (aFullName.Length() == 0) && BaflUtils::FileExists(rfs, logFilePath))
        {
        RFile logFile;
        User::LeaveIfError(logFile.Open(rfs, logFilePath, EFileRead));
        TFileText ft;
        ft.Set(logFile);
        TFileName tempFileName;
        ft.Read(tempFileName);  // read tempfile name stored in logfile.
        
        if ( tempFileName.Length() > 0 )
            {
            rfs.Delete(tempFileName);
            }
        logFile.Close();   
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::RemoveTemporaryRFileL() temp files are removed (reboot)"));
        }              
    rfs.Delete( logFilePath );
    rfs.Close();
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::RemoveTemporaryRFileL() completed"));
    }
// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::SaveFileToFileSystemL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::SaveFileToFileSystemL(
    CObexBufObject* &aReceivedObject,
    const TUid aMtmId,
    TMsvId& aMsvIdParent,
    TFileName& aFullName, 
    RFile& aFile,
    const TDesC& aSenderBluetoothName)
    {
   
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveFileToFileSystemL()"));
    // Create inbox entry ( this was done in CreateAttachment() before )
    //
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CMsvEntry* inbox;
    CreateMsvSessionLC(sessionObs, msvSession, inbox, KMsvGlobalInBoxIndexEntryId);
    // 1st, 2nd, 3rd push

    TBuf<KObexUtilsMaxCharToFromField> toFrom;

    // todo @ QT migration: load from string id for localization
    if (aMtmId == KUidMsgTypeBt)
        {
        _LIT(KBluetoothMsg, "Bluetooth message");
        toFrom.Copy(KBluetoothMsg);
        }
    else
        {
        _LIT(KInfraredMsg, "Infrared message");
        toFrom.Copy(KInfraredMsg);    
        }
    
    // NOTE: toFrom will be updated to Bluetooth sender's bt name in the future.

    TMsvEntry parentTEntry;   
    parentTEntry.iMtm = aMtmId;
    parentTEntry.SetVisible(EFalse); // Make invisible..
    parentTEntry.SetUnread(ETrue);  //  Set msg to Unread
        
    // ...and in preparation to make sure it gets cleaned up on errors.
    //
    parentTEntry.SetInPreparation(ETrue); 
    parentTEntry.iType = KUidMsvMessageEntry;
    parentTEntry.iServiceId = KMsvUnknownServiceIndexEntryId;
    // iDate & iSize will be set later
    if ( aSenderBluetoothName.Length() > 0 )
        {
        parentTEntry.iDetails.Set(aSenderBluetoothName);
        }
    else
        {
        parentTEntry.iDetails.Set(toFrom);
        }
    inbox->CreateL(parentTEntry);
        
    aMsvIdParent = parentTEntry.Id();
        
    // Create attachment for the inbox entry
    
    TMsvEntry attachTEntry;
    attachTEntry.iType      = KUidMsvAttachmentEntry;
    attachTEntry.iServiceId = KMsvUnknownServiceIndexEntryId;   
    attachTEntry.iMtm       = aMtmId;
     
    CMsvEntry* parentEntry = msvSession->GetEntryL(aMsvIdParent);
    CleanupStack::PushL(parentEntry);  // 4th push
    parentEntry->CreateL(attachTEntry);  // create attachment as child of parent
  
    // Check if message is sap message
    // If we delete parent entry in sap, we have to delete the RFile object ourself as well )
    //
    RFs& fsSess = msvSession->FileSession();
    TFileName tempFullName;
    aFile.FullName(tempFullName);
    
#ifdef __BT_SAP   
    if(CSapVMessageParser::HandleMessageL(aReceivedObject, parentEntry->Entry().iMtm,
            aFile, msvSession, GetCurrentTime()))
        {
        RemoveObexBuffer(aReceivedObject);
        CMsvEntry* inbox = msvSession->GetEntryL( KMsvGlobalInBoxIndexEntryId );
        CleanupStack::PushL( inbox );         //5th push
        inbox->DeleteL(aMsvIdParent);
        fsSess.Delete(tempFullName);  // we have to use fullname to delete the file object in file system 
        // Close session etc.
        CleanupStack::PopAndDestroy(5, sessionObs);  // inbox, parentEntry, session, sessionObs
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveFileToFileSystemL() completed"));
        return;
        }
#endif // __BT_SAP

    // Collect obex received data before obex buffer deletion
    TReceivedData receivedData;
    receivedData.bytesReceived = aReceivedObject->BytesReceived();
    receivedData.recTime = aReceivedObject->Time();
    receivedData.msgName = aReceivedObject->Name();
    
    TFileName filename;
    TParse parse;
    parse.Set(aFullName, NULL, NULL);
    filename.Copy(receivedData.msgName);
    FixExtremeLongFileName(filename, parse.DriveAndPath());
    MakeValidFileName( fsSess, filename);     
    RemoveObexBuffer(aReceivedObject);

    // We set file to visible before renaming 
    // so that photo/music app can scan the media files we recevie. 
    //
    // Set file back to visible
    //
    TUint setAttMask(0);
    TUint clearAttMask(0);
    User::LeaveIfError(aFile.Open(fsSess,tempFullName,EFileWrite));
    User::LeaveIfError(aFile.SetAtt(setAttMask , clearAttMask | KEntryAttHidden));
    aFile.Close();
    
    
    // Move file to the actual drive and directory
    // as well rename the file with the correct file name instead of temporary name 
    // tempFullName will be updated in RenameFileL()    
    TInt error = RenameFileL(tempFullName, filename, parse.DriveAndPath(), fsSess);
    
    if ( error != KErrNone )
        {
        fsSess.Delete(tempFullName);   // If rename fails, we remove the temp RFile object.
        }
    aFullName = tempFullName; // assign new full Name back to aFullName,which goes back to caller
    
    CleanupStack::PopAndDestroy(4);         // parentEntry, inbox, msvSession, sessionObs
     
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::SaveFileToFileSystemL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::AddEntryToInboxL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::AddEntryToInboxL( 
    TMsvId& aMsvIdParent,
    TFileName& aFullName)
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::AddEntryToInboxL() "));
        
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CreateMsvSessionLC(sessionObs, msvSession);
    // 1st, 2nd push
        
    CMsvEntry* parentEntry = msvSession->GetEntryL(aMsvIdParent);
    CleanupStack::PushL(parentEntry);  // 3th push
    
    CMsvEntry* attachEntry = msvSession->GetEntryL(((*parentEntry)[0]).Id());
    CleanupStack::PushL(attachEntry); // 4th push
            
    CMsvStore* store = attachEntry->EditStoreL();
    CleanupStack::PushL( store );  // 5th push
        
    CObexutilsEntryhandler* entryHandler = CObexutilsEntryhandler::NewL();
    CleanupStack::PushL(entryHandler);  // 6th push  
                
    CMsvAttachment* attachInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvLinkedFile);
    CleanupStack::PushL(attachInfo);  // 7th push
           
    // Create attachment Info
    //
    RFs& fsSess = msvSession->FileSession();
        
    TParse fileNameParser;
    User::LeaveIfError(fileNameParser.Set(aFullName, NULL, NULL));
    attachInfo->SetAttachmentNameL( fileNameParser.FullName());
                 
    TEntry fileEntry;
    User::LeaveIfError(fsSess.Entry( fileNameParser.FullName(), fileEntry) );
    attachInfo->SetSize(fileEntry.iSize);
            
    // check if it's the bio message
           
    TUid bioMsgId;  // SaveMimeAndGetBioLC modifies this
    CBIODatabase* bioDB = SaveMimeAndGetBioLC( aFullName, fsSess, attachInfo, bioMsgId );
    // 8th push
    if (bioDB)  // bio message
        {
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::AddEntryToInboxL() BIO"));
    
        RFile file;
        User::LeaveIfError(file.Open(fsSess,aFullName,EFileShareReadersOnly));
        TReceivedData receivedData;
        receivedData.bytesReceived = fileEntry.iSize;
        receivedData.recTime = fileEntry.iModified;
        receivedData.msgName = fileNameParser.Name();     
                    
        StoreAndUpdateBioMessageL( bioDB, bioMsgId, parentEntry, file );
        FinaliseMessageL(parentEntry, receivedData, store, ETrue);
        TMsvId attachId = attachEntry->EntryId();
        file.Close();
        fsSess.Delete(aFullName);
        CleanupStack::PopAndDestroy(5);  // bioDB, attachInfo, linkHandler,
                                             // store, attachEntry
            
        // PopAndDestroying here because open store locks the entry,
        // it can be deleted only after destroying the store pointer
        parentEntry->DeleteL(attachId);  // invalidate unused attachment
        CleanupStack::PopAndDestroy(3);  // parentEntry, msvSession, sessionObs
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::AddEntryToInboxL() BIO completed"));
        }
    else
        {

        CleanupStack::PopAndDestroy();  // bioDB
        // Add entry to Inbox
        entryHandler->AddEntryAttachment(aFullName, attachInfo, store);
        CleanupStack::Pop(attachInfo);   // attachInfo, Pass ownership to store
        CleanupStack::PopAndDestroy(entryHandler);  // entryHandler
        FinaliseMessageL(parentEntry, store,fileEntry, fileNameParser);
        CleanupStack::PopAndDestroy(5);         // linkHandler, attachInfo, store, attachEntry,
                                                    // parentEntry, msvSession, sessionObs                            
        FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::AddEntryToInboxL() completed "));
        }
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::AddEntryToInboxL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::AddEntryToInboxL( 
    TMsvId& aMsvIdParent,
    TFileName& aFullName,
    RArray<TMsvId>* aMsvIdArray)
    {
    AddEntryToInboxL(aMsvIdParent, aFullName);
    aMsvIdArray->Append(aMsvIdParent);
    }

// -----------------------------------------------------------------------------
// CObexUtilsMessageHandler::UpdateEntryAttachmentL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::UpdateEntryAttachmentL ( 
    TFileName& aFullName,
    CMsvEntry* aParentEntry)
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::UpdateEntryAttachmentL() "));
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CreateMsvSessionLC(sessionObs, msvSession);
    // 1st, 2nd push
      
    CMsvEntry* attachEntry = msvSession->GetEntryL(((*aParentEntry)[0]).Id());
    CleanupStack::PushL(attachEntry); // 3th push
            
    CMsvStore* store = attachEntry->EditStoreL();
    CleanupStack::PushL( store );  // 4th push
  
    CObexutilsEntryhandler* entryHandler = CObexutilsEntryhandler::NewL();
    CleanupStack::PushL(entryHandler);  // 5th push  
    
    // Note:
    // Because setFilePath() in CMsvAttachment is not implementated by Symbian yet, 
    // we have to delete the original attachment and add another new one to fix the broken link.
    //
    
    // remove the old attachment first.
    //
    store->AttachmentManagerExtensionsL().RemoveAttachmentL(0);
    // Create a new attachment.
    //
    CMsvAttachment* attachInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvLinkedFile);
    CleanupStack::PushL(attachInfo);  // 6th  push
    
    // Get mime type
    //
    RecogniseObjectsL(aFullName, attachInfo);
    RFs& fsSess = msvSession->FileSession();
    
    TParse fileNameParser;
    User::LeaveIfError(fileNameParser.Set(aFullName, NULL, NULL));
    attachInfo->SetAttachmentNameL(fileNameParser.NameAndExt());
    TEntry fileEntry;
    User::LeaveIfError(fsSess.Entry( fileNameParser.FullName(), fileEntry) );
    attachInfo->SetSize(fileEntry.iSize);

    entryHandler->AddEntryAttachment(aFullName,attachInfo, store);
    CleanupStack::Pop(attachInfo);   // attachInfo, Pass ownership to store
    CleanupStack::PopAndDestroy(entryHandler);  // entryHandler
    FinaliseMessageL(aParentEntry, store,fileEntry, fileNameParser);
    CleanupStack::PopAndDestroy(4);   // store, 
                                      // attachEntry, msvSession, sessionObs
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::UpdateEntryAttachmentL() completed "));
    }


// Common methods

// -----------------------------------------------------------------------------
// ConstructRichTextLC Handles rich text initialisation.
// Returns: CRichText* A pointer to the rich text.
// -----------------------------------------------------------------------------
//
CRichText* TObexUtilsMessageHandler::ConstructRichTextLC()
    {
    FLOG(_L("[OBEXUTILS]\t ConstructRichTextLC()"));

    CParaFormatLayer* richParaFormatLayer = CParaFormatLayer::NewL();
    CleanupStack::PushL( richParaFormatLayer );

    CCharFormatLayer* richCharFormatLayer = CCharFormatLayer::NewL(); 
    CleanupStack::PushL( richCharFormatLayer );
    CRichText* richText = CRichText::NewL( richParaFormatLayer, 
                                           richCharFormatLayer );
    CleanupStack::PushL(richText);

    FLOG(_L("[OBEXUTILS]\t ConstructRichTextLC() completed"));

    return richText;
    }

// -----------------------------------------------------------------------------
// MakeValidFileName Rename the given filename to a valid filename.
// Removes invalid characters from a filename; if no characters left, filename
// is set to R_NAME_FOR_INVALID_FILE
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::MakeValidFileName( 
    RFs& aFileSession, // A reference to file server session.
    TDes& aFileName )  // The filename to modify.
    {
    FLOG(_L("[OBEXUTILS]\t MakeValidFileName()"));

    TParse fileParse;
    fileParse.Set(aFileName, NULL, NULL);
    
    if (fileParse.NameOrExtPresent())
        {        
        aFileName = fileParse.NameAndExt();
        TText badChar;    
        TInt nameLength = aFileName.Length();
    
        while ( !aFileSession.IsValidName(aFileName, badChar) )
            {
            // Locate and delete all instances of the bad char
            FOREVER
                {
                TInt index = aFileName.Locate(TChar(badChar)); 
                nameLength--;
                if (index == KErrNotFound)
                    {
                    break;
                    }             
                aFileName.Delete(index,1);               
                }
                if ( nameLength <= 0 )
                    {
                    break;
                    }
            }    
        }    
    FLOG(_L("[OBEXUTILS]\t MakeValidFileName() completed"));
    }

// -----------------------------------------------------------------------------
// TObexUtilsMessageHandler::CreateOutboxEntryL
// -----------------------------------------------------------------------------
//
EXPORT_C TMsvId TObexUtilsMessageHandler::CreateOutboxEntryL(
    const TUid& aMtm,          // Message type id
    //todo need to check whether to use the textmap id or send as a string here
    const TInt& /*aResourceID*/ )  // Resource id for the message entry text
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateOutboxEntryL()"));

    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CMsvEntry* outBoxEntry;
    CreateMsvSessionLC(sessionObs, msvSession, outBoxEntry, KMsvGlobalOutBoxIndexEntryId);
    // 1st, 2nd, 3rd push

    TBuf<KObexUtilsMaxCharToFromField> toFrom;
 //   TObexUtilsUiLayer::ReadResourceL( toFrom, aResourceID );
    //todo need to change the string constant to orbit localisation file
    toFrom.Copy(_L("Bluetooth message"));

    // Message entry fields
    TMsvEntry newTEntry;
    newTEntry.SetVisible( ETrue );          // Make visible
    newTEntry.SetInPreparation( ETrue );    // and in preparation to make sure gets cleaned up on errors.
    newTEntry.iDetails.Set( toFrom );       // To/From field on Outbox
    newTEntry.SetSendingState( KMsvSendStateSending );
    newTEntry.iType = KUidMsvMessageEntry;      
    newTEntry.iMtm = aMtm;                  // message type id 
    newTEntry.iServiceId = KMsvGlobalOutBoxIndexEntryId;
    outBoxEntry->CreateL( newTEntry );
    
    CleanupStack::PopAndDestroy(3);         //outBoxEntry, msvSession, sessionObs
    return newTEntry.Id();
    }

// -----------------------------------------------------------------------------
// TObexUtilsMessageHandler::DeleteOutboxEntryL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::DeleteOutboxEntryL( const TMsvId& aMessageServerIndex )
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::DeleteOutboxEntryL()"));

    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CMsvEntry* outBoxEntry;
    CreateMsvSessionLC(sessionObs, msvSession, outBoxEntry, KMsvGlobalOutBoxIndexEntryId);
    // 1st, 2nd, 3rd push

    outBoxEntry->DeleteL( aMessageServerIndex );

    CleanupStack::PopAndDestroy(3);         //outBoxEntry, msvSession, sessionObs
    }

// -----------------------------------------------------------------------------
// TObexUtilsMessageHandler::GetFileSystemStatus
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsMessageHandler::GetFileSystemStatus()
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::GetFileSystemStatus()"));

    /**
    *  Free space level of FFS file system (drive C:)
    *  The integer value may be:
    *       0: Free space OK
    *      -1: Free space at or below warning level
    *      -2: Free space at or below critical level
    */
    
    TInt status = KErrNone;
    TInt retVal = KErrNone;
    
    retVal = GetPubSubKeyIntValue(KPSUidUikon, KUikFFSFreeLevel, status);
    if ( ( retVal == KErrNone ) && ( status == KFFSBelowCritical ) )
        {
        retVal = KErrDiskFull;
        }
     
    FTRACE(FPrint(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::GetFileSystemStatus completed with %d."), retVal));
    return retVal;
    }

// -----------------------------------------------------------------------------
// TObexUtilsMessageHandler::GetMmcFileSystemStatus
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsMessageHandler::GetMmcFileSystemStatus()
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::GetMmcFileSystemStatus()"));

    /**
    *  Free space level of FFS file system (MMC)
    *  The integer value may be:
    *       0: Free space OK
    *      -1: Free space at or below warning level
    *      -2: Free space at or below critical level
    */
   
    TInt retVal = KErrNone;
    TInt status = KErrNone;
    
    retVal = GetPubSubKeyIntValue(KPSUidUikon, KUikMmcFFSFreeLevel, status);
    if ( ( retVal == KErrNone ) && ( status == KFFSBelowCritical ) )
        {
        retVal = KErrDiskFull;
        }
    
    FTRACE(FPrint(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::GetMmcFileSystemStatus completed with %d."), retVal));
    return retVal;
    }

// -----------------------------------------------------------------------------
// TObexUtilsMessageHandler::GetMessageCentreDriveL
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsMessageHandler::GetMessageCentreDriveL()
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::GetMessageCentreDriveL()"));
    
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CreateMsvSessionLC(sessionObs, msvSession);  // 1st, 2nd push
    
    TDriveUnit driveUnit = msvSession->CurrentDriveL();
    CleanupStack::PopAndDestroy(2);  // msvSession, sessionObs
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::GetMessageCentreDriveL() Completed"));
    return driveUnit;
    }

// -----------------------------------------------------------------------------
// TObexUtilsMessageHandler::CreateDefaultMtmServiceL
// -----------------------------------------------------------------------------
//
EXPORT_C void TObexUtilsMessageHandler::CreateDefaultMtmServiceL(TUid aMtm)
    {
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateDefaultMtmServiceL()"));
    
    CDummySessionObserver* sessionObs;
    CMsvSession* msvSession;
    CMsvEntry* root;
    CreateMsvSessionLC(sessionObs, msvSession, root, KMsvRootIndexEntryId);
    // 1st, 2nd, 3rd push
    
    TMsvSelectionOrdering msvSelectionOrdering = root->SortType();
    msvSelectionOrdering.SetShowInvisibleEntries( ETrue );
    root->SetSortTypeL( msvSelectionOrdering );
    
    CMsvEntrySelection* sel = root->ChildrenWithMtmL( aMtm );
    CleanupStack::PushL( sel );  // 4th push
    if (sel->Count() == 0)
        {
        TMsvEntry serviceEntry;
        serviceEntry.iMtm = aMtm;
        serviceEntry.iType = KUidMsvServiceEntry;
        serviceEntry.SetVisible(EFalse);
        serviceEntry.SetReadOnly(ETrue);
        serviceEntry.SetInPreparation(EFalse);
        root->CreateL(serviceEntry);
        }
    CleanupStack::PopAndDestroy(4);  // sel, root, msvSession, sessionObs
    
    FLOG(_L("[OBEXUTILS]\t TObexUtilsMessageHandler::CreateDefaultMtmServiceL() Completed"));
    }

// -----------------------------------------------------------------------------
// GetCenRepKeyIntValue Gets integer value for a specified CenRep key
// Returns: CenRep key's value
// -----------------------------------------------------------------------------
//
EXPORT_C TInt TObexUtilsMessageHandler::GetPubSubKeyIntValue(
    TUid aCategory,  // Category of the key
    TUint aKey,      // PUBSUB key
    TInt& value)     // Returned value
    {
    TInt retVal;
    RProperty property;
    retVal = property.Attach(aCategory, aKey);
    if ( retVal == KErrNone )
        {
        retVal = property.Get(value);
        if (retVal != KErrNone)
            {
            FLOG(_L("[OBEXUTILS]\t GetPubSubKeyIntValue() Get failed for CenRep key"));
            }
        }
    else
        {
        FLOG(_L("[OBEXUTILS]\t GetPubSubKeyIntValue() Attach failed for CenRep key"));
        }
    property.Close();
    return retVal;
    }

// -----------------------------------------------------------------------------
// SetBioDataTextL Converts message type to bio message id.
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::SetBioDataTextL(
    TBioMsgIdText& aText, // A bio message id text.
    const TDesC8& aType ) // The type of the content. 
    {
    FLOG(_L("[OBEXUTILS]\t SetBioDataTextL()"));

    HBufC16* unicode = CnvUtfConverter::ConvertToUnicodeFromUtf8L( aType );
    CleanupStack::PushL( unicode );
    
    if (unicode->Length() > KMaxBioIdText )
        {
        User::Leave( KErrOverflow );
        }
    
    aText = unicode->Des();
    CleanupStack::PopAndDestroy( unicode );
    
    FLOG(_L("[OBEXUTILS]\t SetBioDataTextL() completed"));
    }

// -----------------------------------------------------------------------------
// SetDetailsLC Converts message type to unicode.
// Returns: None.
// -----------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::SetDetailsLC(
    TPtrC& aDetails,      // A message text.
    const TDesC8& aType ) // The type of the content.
    {
    FLOG(_L("[OBEXUTILS]\t SetDetailsLC()"));

    HBufC16* unicode = CnvUtfConverter::ConvertToUnicodeFromUtf8L( aType );
    CleanupStack::PushL( unicode );
    aDetails.Set(unicode->Des());

    FLOG(_L("[OBEXUTILS]\t SetDetailsLC() completed"));
    }

//-------------------------------------------------------------------------------
// Rename a file and move file if needed 
// Return: None.
//-------------------------------------------------------------------------------
//
TInt TObexUtilsMessageHandler::RenameFileL(
    TFileName& aFileName, 
    TFileName& aNewFileName,
    const TDesC& aNewPath, 
    RFs& aFileSession)
    {
    
    // We move the file to the final location
    //
    CFileMan* fileMan= CFileMan::NewL(aFileSession);
    CleanupStack::PushL(fileMan);       
    TPtrC tmpNewPath;
    TParse fileParse;
    fileParse.Set(aFileName, NULL, NULL);
    
    TInt error = fileMan->Move(aFileName,aNewPath,CFileMan::ERecurse );
    
    // if error while moving to new location, keep file in old location and change file name
    if (error != KErrNone)
        {
        tmpNewPath.Set (fileParse.DriveAndPath()); 
        }
    else
        {
        tmpNewPath.Set (aNewPath);
        }
    
    CleanupStack::PopAndDestroy(fileMan);

    TFileName tempFile;
    tempFile.Append(tmpNewPath);
    tempFile.Append(fileParse.NameAndExt());
    aFileName = tempFile;
    
    // check the duplicated file name. if so, we renamed in a different way. 
    TInt segmentNum = 0;
    TBuf<64> segmentString;
    
    TFileName newFullName;
    newFullName.Zero();
    newFullName.Append(tmpNewPath);
    newFullName.Append(aNewFileName);
    
    aFileSession.SetSessionPath(tmpNewPath);
    
    while ( BaflUtils::FileExists(aFileSession, newFullName) )    
        {
        segmentNum++;
        User::LeaveIfError( RenameFileWithSegmentNumL(aNewFileName, segmentNum, segmentString) );
        newFullName.Zero();
        newFullName.Append(tmpNewPath);
        newFullName.Append(aNewFileName);
        }
    // rename the file.
    //
    error = aFileSession.Rename(aFileName, newFullName);

    aFileName = newFullName;
    
    return error;
    }

//-------------------------------------------------------------------------------
// Rename a file by adding a additional segment string
// Return: None.
//-------------------------------------------------------------------------------
//
TInt TObexUtilsMessageHandler::RenameFileWithSegmentNumL(
    TDes &aFileName,        // name for the file 
    TInt aSegmentNum,       // segment number
    TDes &aSegmentString)   // segment String with parenthesis
    {
    
    FLOG(_L("[OBEXUTILS]\t RenameFileWithSegmentNumL()"));

    const char KDot = '.';
    const char KLeftParenthesis = '(';
    const char KRightParenthesis = ')';
    
    // Convert Integer to String
    // 
    TBuf<64> buf;
    buf.Zero();
    _LIT(KNum, "%d" );
    buf.AppendFormat(KNum, aSegmentNum);
    
    // Rules to modify the duplicated filenames.
    // case 1: No extention in filename,  we append aSegmentString directly.
    // case 2: Filename with extention, we insert aSegmentString right before dot (.)
    // case 3: Filename with aSegmentString is still duplicated, 
    //         we remove the old aSegmentString from aFileName and add a new one and add a new aSegmentString.
    
    TInt posOfDot = aFileName.LocateReverse(KDot);
    
    if ( aSegmentNum > 1 )   // filename with aSegementString is still duplicated
        {
        // In case the original file name contains the same string as our aSegmentString
        // eg 1 with ext. original filename is file(1).txt, after renaming, it is changed to file(1)(1).txt.
        // eg 2 without ext. original filename is file(1), after renaming, it is changed to file(1)(1).
        TInt posDeleteStarts = posOfDot == KErrNotFound ? 
                                aFileName.Length() - aSegmentString.Length() :
                                posOfDot - aSegmentString.Length();
                                
        aFileName.Delete(posDeleteStarts, aSegmentString.Length());
        }
    
    // Construct (New) aSegmentString
    aSegmentString.Zero();
    aSegmentString.Append(KLeftParenthesis);
    aSegmentString.Append(buf);
    aSegmentString.Append(KRightParenthesis);
    
    // 
    // If the length of new filename will be larger than KMaxFileName, we remove few chars from the end of aFilename 
    // in order to add aSegmentString without panic.
    // 
    if ( KMaxFileName < (aFileName.Length() + aSegmentString.Length()))
        {
        aFileName.Delete(KMaxFileName - aSegmentString.Length(), KMaxFileName - aFileName.Length());
        }
    
    posOfDot = aFileName.LocateReverse(KDot);
    if ( posOfDot == KErrNotFound )   
        {
        // aFileName has no extension
        // we append the modification to aFileName
        //
        aFileName.Append(aSegmentString);
        }
    else
        {
        // Dot (.) is found, we insert the modifications before dot.
        //
        aFileName.Insert(posOfDot,aSegmentString);
        
        }
    
    FLOG(_L("[OBEXUTILS]\t RenameFileWithSegmentNumL() completed"));
    return KErrNone;
    }

//-------------------------------------------------------------------------------
// Shorten an extreme long file name
// Return: None.
//-------------------------------------------------------------------------------
//
void TObexUtilsMessageHandler::FixExtremeLongFileName(TFileName& aExtremeLongFileName, const TDesC& aDriveAndPath)
     {
     FLOG( _L( "[OBEXUTILS]\t FixExtremeLongFileName() ") );
     const char KDot = '.';
     TInt combinedLength = aExtremeLongFileName.Length() + aDriveAndPath.Length();
     
     if ( combinedLength > aExtremeLongFileName.MaxLength() )
        {
        FLOG( _L( "[OBEXUTILS]\t FixExtremeLongFileName: combinedLenght > aExtremeLongFileName.MaxLength()\t") );
         //KDefaultFileExtensionSeparator
         TFileName tempFileName;
         TInt dotPos = aExtremeLongFileName.LocateReverse( KDot );    
         TInt overLap = combinedLength - aExtremeLongFileName.MaxLength() + KRenameOffSet;  
 
         if ( dotPos > 0 )
             {
             tempFileName.Copy( aExtremeLongFileName.Left( dotPos - overLap ) );
             tempFileName.Append( aExtremeLongFileName.Right( aExtremeLongFileName.Length() - dotPos ) );  
             }
         else
             {
             tempFileName.Copy( aExtremeLongFileName.Left(aExtremeLongFileName.Length() - overLap ) );
             }
         aExtremeLongFileName.Copy( tempFileName );
         }
     FLOG( _L( "[OBEXUTILS]\t FixExtremeLongFileName() completed") );
     }

//  End of File  

