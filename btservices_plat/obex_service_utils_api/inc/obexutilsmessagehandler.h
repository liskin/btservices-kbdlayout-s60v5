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
* Description: 
*
*/


#ifndef OBEXUTILSMESSAGEHANDLER_H
#define OBEXUTILSMESSAGEHANDLER_H

//  INCLUDES
#include <e32base.h>
#include <e32cmn.h>
#include <obex.h>
#include <msvapi.h>

#include <cmsvattachment.h>
#include <biodb.h>

// CLASS DECLARATION

/**
*  Utility methods for message handling.
*/
class TObexUtilsMessageHandler
    {
    public:
    
        /**
        * Get status of file system. (in C drive)
        * @return Symbian OS errorcode
        */
        IMPORT_C static TInt GetFileSystemStatus();

        /**
        * Get status of file system in MMC.
        * @return Symbian OS errorcode
        */
        IMPORT_C static TInt GetMmcFileSystemStatus();

        /**
        * Get current message centre drive
        * @return Drive unit of the message centre drive
        */
        IMPORT_C static TInt GetMessageCentreDriveL();

        /**
        * Creates default Mtm service
        * @param aMtm Mtm
        */
        IMPORT_C static void CreateDefaultMtmServiceL(TUid aMtm);

        /**
        * Saves received object to Inbox.
        * @param aReceivedObject The object to be saved.
        * @param aDrive The current messaging drive.
        * @param aMtmID The message type module id.
        * @return None.
        */
        IMPORT_C static void SaveObjToInboxL( CObexBufObject* aReceivedObject,
                                              const TDesC16& aPath, 
                                              const TUid aMtmID );

        /**
        * Creates an Outbox entry
        * @param aMTm Message type id
        * @param aResourceID Resource id for the message entry text
        * @return Message server entry.
        */
        IMPORT_C static TMsvId CreateOutboxEntryL( const TUid& aMtm, const TInt& aResourceID );

        /**
        * Deletes an Outbox entry
        * @param aMessageServerIndex Message server entry
        * @return None.
        */
        IMPORT_C static void DeleteOutboxEntryL( const TMsvId& aMessageServerIndex );
        
        /**
        * Create inbox attachment and entries.
        * @param aBufObject The object to be saved
        * @param aMtmId MTM Id
        * @param aMsvIdParent Parent message entry (returned)
        * @param aFile File to the attachment (returned)
        * @return None.
        */
        IMPORT_C static void CreateInboxAttachmentL(
            CObexBufObject* &aBufObject,
            const TUid aMtmId,
            TMsvId &aMsvIdParent,
            RFile &aFile);
        
        /**
         * Create inbox attachment and entries.
         * @param aBufObject The object to be saved
         * @param aMtmId MTM Id
         * @param aMsvIdParent Parent message entry (returned)
         * @param aFile File to the attachment (returned)
         * @param aRemoteBluetoothName sender's bluetooth name.
         * @return None.
         */
        IMPORT_C static void CreateInboxAttachmentL(
            CObexBufObject* &aBufObject,
            const TUid aMtmId,
            TMsvId &aMsvIdParent,
            RFile &aFile,
            const TDesC& aRemoteBluetoothName);

        /**
        * Saves received object to Inbox.
        * @param aReceivedObject The object to be saved (set to null after remove).
        * @param aFile File to the attachment
        * @param aMsvIdParent Parent message entry
        * @return None.
        */
        IMPORT_C static void SaveObjToInboxL(
            CObexBufObject* &aReceivedObject,
            RFile &aFile,
            TMsvId aMsvIdParent);
        
        /**
        * Remove unfinished work from inbox
        * @param aBufObject The object to be saved (set to null after remove)
        * @param aMsvIdParent Parent message entry
        * @return None.
        */
        IMPORT_C static void RemoveInboxEntriesL(CObexBufObject* &aBufObject, TMsvId aMsvIdParent);
        
        /**
        * Gets central repository key's integer value
        * @param aRepositoryUid Central repository's UID value
        * @param aId Central repository's ID value
        * @param aVal Returned integer value
        * @return Symbian OS errorcode
        */
        IMPORT_C static TInt GetCenRepKeyIntValueL(TUid aRepositoryUid, TUint32 aId, TInt& aVal);
        
        /**
        * Gets central repository key's string value
        * @param aRepositoryUid Central repository's UID value
        * @param aId Central repository's ID value
        * @param aVal Returned string value
        * @return Symbian OS errorcode
        */

        IMPORT_C static TInt GetCenRepKeyStringValueL(TUid aRepositoryUid, TUint32 aId, TDes& aVal);

        /**
        * Gets Publish & Subscribe key's integer value
        * @param aCategory Category of the key
        * @param aKey PUBSUB Key
        * @param value Returned value
        * @return Symbian OS errorcode
        */
        IMPORT_C static TInt GetPubSubKeyIntValue(TUid aCategory, TUint aKey, TInt& value);
        
        /**
        * Create receive buffer and RFile object.
        * @Since S60 v5.0
        * @param aFile File to temp file
        * @param aPath Path The directory in which the file is created
        * @param aFullName Contains the full path and file name of the file (returned) 
        * @param aReceiveObject Obex object for receive buffer ( returned )
        * @param aBufSize Size of receive buffer
        * @return Symbian error code.
        */
        IMPORT_C static TInt CreateReceiveBufferAndRFileL ( RFile& aFile,
                                                            const TDesC& aPath,
                                                            TFileName& aFullName, 
                                                            CBufFlat* &aReceiveObject, 
                                                            const TInt aBufSize );   
        
        /**
        * Remove temporary RFile object.
        * @Since S60 v5.0
        * @param aFullName Contains the full path and file name of the file  
        * @return Symbian error code.
        */
        IMPORT_C static void RemoveTemporaryRFileL ( const TFileName& aFullName );
        
        /**
         * Create inbox entries and saves received object to file system
         * @Since S60 v5.0
         * @param aReceiveObject Obex object for receive buffer 
         * @param aMtmId MTM Id
         * @param aMsvIdParent Parent message entry (returned)
         * @param aFullName The directory and file name in which the file is saved  (returned)
         * @param aFile File handler identical with the one in CreateReceiveBufferAndRFileL()
         *              it must be opened before use. 
         * @return None
         */ 
        IMPORT_C static void SaveFileToFileSystemL(CObexBufObject* &aReceiveObject,
                                                   const TUid aMtmId,
                                                   TMsvId& aMsvIdParent,
                                                   TFileName& aFullName,
                                                   RFile& aFile,
                                                   const TDesC& aSenderBluetoothName);
        
        /**
         * Add an entry to Inbox
         * @Since S60 v5.0
         * @param aMsvIdParent Parent message entry
         * @param aFullName Contains the full path and file name of the file
         * @return None
         */
        IMPORT_C static void AddEntryToInboxL( TMsvId& aMsvIdParent,
                                               TFileName& aFullName);
                                               
        /**
         * Add an entry to Inbox
         * @Since S60 v5.0
         * @param aMsvIdParent Parent message entry
         * @param aFullName Contains the full path and file name of the file
         * @param aMsvIdArray an array contains a list of TMsvId ( returned )
         * @return None
         */
        IMPORT_C static void AddEntryToInboxL( TMsvId& aMsvIdParent,
                                              TFileName& aFullName,
                                              RArray<TMsvId>* aMsvIdArray);
        
        /**
         * Update entry attachmeent in Inbox
         * @Since S60 v5.0
         * @param aFullName Contains the full path and file name of the file
         * @param aMsvIdParent Parent message entry
         * @return None
         */  
        IMPORT_C static void UpdateEntryAttachmentL ( TFileName& aFullName,CMsvEntry* aParentEntry );
        
        
        
    private:

        class CDummySessionObserver;
        class TReceivedData;

        /**
        * Create inbox attachment and entries.
        * @param aMtmId Mtm ID of the message
        * @param aMsvIdParent Parent message entry (returned)
        * @param aFile File to the attachment (returned)
        * @param aRemoteBluetoothName sender's bluetooth name.
        */
        static void DoCreateInboxAttachmentL(
            const TUid aMtmId,
            TMsvId &aMsvIdParent,
            RFile &aFile,
            const TDesC& aRemoteBluetoothName = KNullDesC);

        /**
        * Deletes Obex Buf Object buffer
        * @param aBufObject The buffer object
        * @return None.
        */
        static void RemoveObexBuffer(CObexBufObject* &aBufObject);

        /*
        * Creates Msv session
        * @param aSessionObs Session observer
        * @param aMsvSession Session
        */        
        static void CreateMsvSessionLC(
            CDummySessionObserver* &aSessionObs,
            CMsvSession* &aMsvSession);

        /*
        * Creates Msv session to selected destination
        * @param aSessionObs Session observer
        * @param aMsvSession Session
        * @param aDestination Destination
        * @param aMsvId Destination type
        */        
        static void CreateMsvSessionLC(
            CDummySessionObserver* &aSessionObs,
            CMsvSession* &aMsvSession,
            CMsvEntry* &aDestination,
            TMsvId aMsvId);

        /**
        * Gets current time
        * @return None.
        */
        static TTime GetCurrentTime();

        /**
        * Stores message as rich text
        * @param aParentEntry Parent entry
        * @param aFile File to store to the message entry's store
        * @return None.
        */        
        static void StoreAsRichTextL(CMsvEntry* aParentEntry, RFile& aFile);

        /**
        * Updates attachment's information
        * @param aRecData Received obex data
        * @param aAttachEntry Attachment entry
        * @param aAttachInfo Attachment information
        * @param aStore Message store (attachment)
        * @return None.
        */
        static void UpdateAttachmentInformationL(
            TReceivedData& aRecData,
            CMsvEntry* aAttachEntry,
            CMsvAttachment* aAttachInfo,
            CMsvStore* aStore);

        /**
        * Finalises the parent entry to contain correct data
        * @param aParentEntry Parent entry
        * @param aReceivedData Received obex data
        * @param aStore Attachment's store
        * @param aIsBioMsg Is BIO message?
        * @return None.
        */
        static void FinaliseMessageL(
            CMsvEntry* aParentEntry,
            TReceivedData& aRecData,
            CMsvStore* aStore,
            TBool aIsBioMsg);
        
        /**
         * Finalises the parent entry to contain correct data
         * @Since v5.0
         * @param aParentEntry Parent entry
         * @param aStore Attachment's store
         * @param aFileEntry a file entry
         * @param aFileNameParser a file name parser
         * @return None.
         */
        static void FinaliseMessageL(
            CMsvEntry* aParentEntry,
            CMsvStore* aStore,
            TEntry& aFileEntry,
            TParse& aFileNameParser);

        /**
        * Stores BIO message to parent entry's body and updates information
        * @param aBioDB BIO database for a BIO message
        * @param parentEntry Parent entry
        * @param aFile File to save to parent entry's body
        * @return None.
        */
        static void StoreAndUpdateBioMessageL(
            CBIODatabase* aBioDB,
            TUid aBioMsgId,
            CMsvEntry* parentEntry,
            RFile& aFile);
        
        /**
        * Recognises MIME type of a file
        * @param aFile File to recognise
        * @param aAttachInfo Attachment information
        * @return None.
        */
        static void RecogniseObjectsL(
            TFileName& aFileName,
            CMsvAttachment* aAttachInfo);

        /**
        * Saves MIME type and gets BIO for that MIME type
        * @param aFile File to be recognised
        * @param aFsSess File session to use for BIO database
        * @param aAttachInfo Attachment information
        * @param aBioMsgId Returned BIO message's ID
        * @return BIO database for a MIME type (null if not BIO)
        */
        static CBIODatabase* SaveMimeAndGetBioLC(
            TFileName& aFileName,
            RFs& aFsSess,
            CMsvAttachment* aAttachInfo,
            TUid &aBioMsgId);

        /**
        * Constructs rich text
        * @return Text in rich text.
        */
        static CRichText* ConstructRichTextLC();

        /**
        * Makes a valid OS filesystem filename by removing illegal characters
        * @param aFileSession File server filesession
        * @param aFileName File name to check and modify
        * @return None.
        */
        static void MakeValidFileName(RFs& aFileSession, TDes& aFileName);

        /**
        * Converts text from UTF-8 to two-byte Unicode
        * @param aText destination where to store the converted text
        * @param aType source of conversion in UTF-8
        * @return None.
        */
        static void SetBioDataTextL(TBioMsgIdText& aText, const TDesC8& aType);

        /**
        * Converts text from UTF-8 to two-byte Unicode
        * @param aDetails destination where to store the converted text
        * @param aType source of conversion in UTF-8
        * @return None.
        */
        static void SetDetailsLC(
            TPtrC& aDetails,
            const TDesC8& aType);
        
        
        /**
         * Rename file and move file if needed.
         * @Since S60 v5.0
         * @param anOldFileName  a file name with absolute path ( returned )
         * @param aNewFileName   new path where file will be moved to 
         * @param aFileSession   a file session
         * @return None
         */
        static TInt RenameFileL(TFileName& aFileName,
                                TFileName& aNewFileName,
                                const TDesC& aNewPath, 
                                RFs& aFileSession);
        
        /**
         * Rename filename with a segement number.
         * @Since S60 v5.0
         * @param aFilename name of the file ( returned )
         * @param aSegmentNum number used to modifed the file name
         * @param aSegmentString string will be added to filename to solve the duplicate filenames
         * @return None.
         */
        static TInt RenameFileWithSegmentNumL(
            TDes& aFileName, 
            TInt aSegmentNum, 
            TDes& aSegementString);
        
        /**
         * FixExtremeLongFileName
         * @param aFileName extreme long file name (returned)
         * @param aDriveAndPath file drive and path 
         * @return                          None
         */
        static void FixExtremeLongFileName(TFileName& aExtremeLongFileName, const TDesC& aDriveAndPath);

        
        /**
        * A dummy class for opening CMsvSession.
        */
        class CDummySessionObserver : public CBase , public MMsvSessionObserver
            {
            public:
                void HandleSessionEventL( TMsvSessionEvent/*aEvent*/,
                    TAny* /*aArg1*/,
                    TAny* /*aArg2*/,
                    TAny* /*aArg3*/ ) {};
            };

        /**
        * Class for obex received data
        */
        class TReceivedData
            {
            public:
                TInt bytesReceived;
                TTime recTime;
                TFileName msgName;
            };

    };

#endif      // OBEXUTILSMESSAGEHANDLER_H

// End of File
