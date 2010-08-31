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
* Description:
*
*/


// INCLUDE FILES
#include "vMessageHandler.h"
#include "obexutilsdebug.h"
#include "etelmm.h"
#include <BTSapDomainPSKeys.h>
#include <app/smut.h> // KUidMsgTypeSMS
#ifdef NO101APPDEPFIXES_NEW
#include <app/smuthdr.h>
#endif //NO101APPDEPFIXES_NEW

#include <gsmupdu.h>
#include <txtrich.h>
#include <msvuids.h>

#include <app/csmsaccount.h>

#ifdef NO101APPDEPFIXES_NEW
// SMUT Unbranch
#include <app/csmsgetdetdescinterface.h>
#endif //NO101APPDEPFIXES_NEW

// todo @ QT migration: take official definition from Messaging at app layer (btmsgtypeuid.h)
const TUid KUidMsgTypeBt = {0x10009ED5};

// ================= MEMBER FUNCTIONS =======================
TBool CSapVMessageParser::HandleMessageL(CObexBufObject*  aReceivedObject, const TUid aMtmID,
                                        RFile& aFile, CMsvSession* aMsvSession, TTime aTime)
{
    FLOG( _L( " CSapVMessageParser: HandleMessageL\t" ) );

    CSapVMessageParser* parser = CSapVMessageParser::NewLC();
    TBool isVmsg=parser->ParseMessageL(aReceivedObject, aMtmID, aFile, aTime);
    if(isVmsg)
    {
        parser->SaveSapMessageL(aMsvSession);
    }
    CleanupStack::PopAndDestroy(parser);

    FLOG( _L( " CSapVMessageParser: HandleMessageL: Done\t" ) );
    
    return isVmsg;
}

CSapVMessageParser* CSapVMessageParser::NewLC()
{
    CSapVMessageParser* self = new (ELeave) CSapVMessageParser();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
}

void CSapVMessageParser::ConstructL()
{
}

// ---------------------------------------------------------
// Constructor
// ---------------------------------------------------------
//
CSapVMessageParser::CSapVMessageParser()
{
}

// ---------------------------------------------------------
// ParseMessageL
// Recognises and parses SAP VMessage.
// ---------------------------------------------------------
//
TBool CSapVMessageParser::ParseMessageL(CObexBufObject*  aReceivedObject, const TUid aMtmID,
                                        RFile& aFile, TTime aTime)
{
    FLOG( _L( " CSapVMessageParser: ParseMessageL\t" ) );

    static const TInt KExpandSize = 16;

    iType=ESapVMessageUnknown;

    if( aMtmID != KUidMsgTypeBt ||
        !CheckMime(aReceivedObject->Type()) ||
        !CheckName(aReceivedObject->Name()) ||
        !IsSapConnectionActive())
    {
        FLOG( _L( " CSapVMessageParser: ParseMessageL: Unknown\t" ) );
        return EFalse;
    }

    CBufFlat* buffer = CBufFlat::NewL( KExpandSize );
    CleanupStack::PushL(buffer);            // 1st push

    TInt fileLength;
    User::LeaveIfError( aFile.Size( fileLength ) );

    // Read the file into buffer
    buffer->ResizeL( fileLength );
    TPtr8 temp = buffer->Ptr(0);
    TInt pos = 0;
    aFile.Seek(ESeekStart, pos);
    User::LeaveIfError( aFile.Read( temp ) );
    
    pos=0;
    aFile.Seek(ESeekStart, pos); // rewind file

    SimpleParseL(temp);

    CleanupStack::PopAndDestroy(buffer); // -1 pop
    
    iTimeReceived=aTime;
    if(aReceivedObject->Time()==TTime(0))
    {
        iTimeOriginal = aTime;
    }
    else
    {
        iTimeOriginal = aReceivedObject->Time();
    }

    return (iType!=ESapVMessageUnknown);
}

// ---------------------------------------------------------
// IsSapConnectionActive
// Recognises active SAP connection.
// ---------------------------------------------------------
//
TBool CSapVMessageParser::IsSapConnectionActive()
{
    FLOG( _L( " CSapVMessageParser: IsSapConnectionActive\t" ) );

    TInt state=EBTSapNotConnected;
    RProperty::Get(KPSUidBluetoothSapConnectionState, KBTSapConnectionState, state);
    return state==EBTSapConnected;
}


// ---------------------------------------------------------
// CheckMime
// Check MIME type of vMessage.
// ---------------------------------------------------------
//
TBool CSapVMessageParser::CheckMime(const TDesC8& aType)
{
    FLOG( _L( " CSapVMessageParser: CheckMime\t" ) );

    _LIT8(KMimeVmsg,"text/x-vmsg\x00");

    return (aType.Compare(KMimeVmsg)==0);
}


// ---------------------------------------------------------
// CheckName
// Check object name of SAP vMessage.
// ---------------------------------------------------------
//
TBool CSapVMessageParser::CheckName(const TDesC& aName)
{
    FLOG( _L( " CSapVMessageParser: CheckName\t" ) );

    _LIT(KNameVmsg, "sap_sms.vmg");

    return (aName.Compare(KNameVmsg)==0);
}

// ---------------------------------------------------------
// Address
// Returns address field of parsed message.
// ---------------------------------------------------------
//
const TDesC& CSapVMessageParser::Address() const
{
    if(iType==ESapVMessageTextSMS && iAddress)
    {
        return *iAddress;
    }
    else
    {
        return KNullDesC;
    }
}

// ---------------------------------------------------------
// Message
// Returns body text of parsed message.
// ---------------------------------------------------------
//
const TDesC& CSapVMessageParser::Message() const
{
    if(iType==ESapVMessageTextSMS && iMessage)
    {
        return *iMessage;
    }
    else
    {
        return KNullDesC;
    }
}

// ---------------------------------------------------------
// ~CSapVMessageParser
// Destructor.
// ---------------------------------------------------------
//
CSapVMessageParser::~CSapVMessageParser()
{
    FLOG( _L( " CSapVMessageParser: ~CSapVMessageParser\t" ) );

    delete iAddress;
    delete iMessage;
}

static TBool Compare(const TDesC8& aData, TInt& aReadBytes, const TDesC8& aToken)
{
    if( (aData.Length() >= aReadBytes + aToken.Length()) &&
        (aData.Mid(aReadBytes, aToken.Length()).Compare(aToken)==0) )
    {
        aReadBytes+=aToken.Length();
        return ETrue;
    }
    else
    {
        return EFalse;
    }
}

// ---------------------------------------------------------
// SimpleParseL
// Parses SAP VMessage.
// ---------------------------------------------------------
//
void CSapVMessageParser::SimpleParseL(const TDesC8& aData)
{
    FLOG( _L( " CSapVMessageParser: SimpleParseL\t" ) );

    // SAP VMessage format definitions
    _LIT8(KVmsgStatus,  "BEGIN:VMSG\x0d\x0a"
                        "VERSION:1.1\x0d\x0a"
                        "X-IRMC-STATUS:");
    _LIT8(KVmsgBox,     "\x0d\x0a"
                        "X-IRMC-TYPE:SMS\x0d\x0a"
                        "X-IRMC-BOX:");
    _LIT8(KVmsgVCard,   "BEGIN:VCARD\x0d\x0a"
                        "VERSION:2.1\x0d\x0a");
    _LIT8(KVmsgName1,   "N:");
    _LIT8(KVmsgName2,   "N;ENCODING=8BIT;CHARSET=UTF-8:");
    _LIT8(KVmsgTel,     "\x0d\x0a"
                        "TEL:");
    _LIT8(KVmsgVCardEnd,"\x0d\x0a"
                        "END:VCARD\x0d\x0a");
    _LIT8(KVmsgVEnv,    "BEGIN:VENV\x0d\x0a");
    _LIT8(KVmsgVBody,   "BEGIN:VBODY\x0d\x0a"
                        "X-SMS;TYPE=TEXT;ENCODING=8BIT;CHARSET=UTF-8:");
    _LIT8(KVmsgEnd,     "\x0d\x0a"
                        "END:VBODY\x0d\x0a"
                        "END:VENV\x0d\x0a"
                        "END:VMSG\x0d\x0a");
    _LIT8(KVmsgUnread,  "UNREAD");
    _LIT8(KVmsgRead,    "READ");
    _LIT8(KVmsgInbox,   "INBOX\x0d\x0a");
    _LIT8(KVmsgSentbox, "SENTBOX\x0d\x0a");
    _LIT8(KCrLf,        "\x0d\x0a");

    // Simple SAP VMessage parsing
    TInt readBytes=0;
    if( Compare(aData, readBytes, KVmsgStatus) &&
        aData.Right(KVmsgEnd().Length()).Compare(KVmsgEnd)==0 )
    {   // The begin and end of the message are correct
        if(Compare(aData, readBytes, KVmsgUnread))
        {   // Status: Unread
            iStatus=ESapVMessageStatusUnread;
        }
        else if(Compare(aData, readBytes, KVmsgRead))
        {   // Status: Read or Sent
            iStatus=ESapVMessageStatusRead;
        }
        else
        {   // Unknown status
            return;
        }
        if( Compare(aData, readBytes, KVmsgBox) )
        {
            if(iStatus==ESapVMessageStatusRead && Compare(aData, readBytes, KVmsgSentbox))
            {   // Status: Sent
                iStatus=ESapVMessageStatusSent;
                if(!Compare(aData, readBytes, KVmsgVEnv))
                {
                    return;
                }
            }
            else if(! Compare(aData, readBytes, KVmsgInbox) )
            {
                return;
            }
            if( Compare(aData, readBytes, KVmsgVCard) &&
                ( Compare(aData, readBytes, KVmsgName1) ||
                    Compare(aData, readBytes, KVmsgName2) ) )
            {   // The begin of the message is correct
                TInt beginOfName=readBytes;
                TInt endOfName=aData.Find(KVmsgTel);
                TInt endOfFrom=aData.Find(KVmsgVCardEnd);
                readBytes=endOfFrom+KVmsgVCardEnd().Length();
                if(iStatus!=ESapVMessageStatusSent)
                {
                    if(!Compare(aData, readBytes, KVmsgVEnv))
                    {
                        return;
                    }
                }
                if(endOfFrom!=KErrNotFound && endOfName!=KErrNotFound &&
                    endOfName<endOfFrom && Compare(aData, readBytes, KVmsgVBody))
                {   // The middle part of the message is correct
                    TInt beginOfFrom=endOfName+KVmsgTel().Length();
                    TInt fromLength=endOfFrom-beginOfFrom;
                    if(fromLength <= RMobilePhone::KMaxMobileTelNumberSize)
                    {   // The sender field of the message is not too long
                        TInt beginOfBody=readBytes;
                        TInt bodyLength=(aData.Length()-KVmsgEnd().Length())-beginOfBody;

                        TPtrC8 name = aData.Mid(beginOfName, endOfName-beginOfName);
                        TPtrC8 from = aData.Mid(beginOfFrom, fromLength);
                        if((name.Find(KCrLf)==KErrNotFound) && (from.Find(KCrLf)==KErrNotFound))
                        {   // Message is correct
                            if(from.Length()==0 && name.Length()>0 &&
                                name.Length() <= RMobilePhone::KMaxMobileTelNumberSize)
                            {
                                iAddress = CnvUtfConverter::ConvertToUnicodeFromUtf8L(name);
                            }
                            else
                            {
                                iAddress = HBufC::NewL(from.Length());
                                iAddress->Des().Copy(from);
                            }

                            TPtrC8 body = aData.Mid(beginOfBody, bodyLength);
                            iMessage = CnvUtfConverter::ConvertToUnicodeFromUtf8L(body);

                            FLOG( _L( " CSapVMessageParser: SimpleParseL: SMS\t" ) );
                            iType=ESapVMessageTextSMS;
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------
// SaveSapMessageL
// Saves parsed VMessage.
// ---------------------------------------------------------
//
void CSapVMessageParser::SaveSapMessageL(CMsvSession* aMsvSession)
{
    FLOG( _L( " CSapVMessageParser: SaveSapMessageL\t" ) );

    switch(iType)
    {
        case ESapVMessageTextSMS:
            SaveSapSmsL(aMsvSession);
            break;

        case ESapVMessageMMSNotificationInd:
            SaveSapMmsL(aMsvSession);
            break;

        default: // Discard message
            break;
    }
}

// ---------------------------------------------------------
// SaveSapSmsL
// Saves parsed VMessage as SMS.
// ---------------------------------------------------------
//
void CSapVMessageParser::SaveSapSmsL(CMsvSession* aMsvSession)
{
    FLOG( _L( " CSapVMessageParser: SaveSapSmsL\t" ) );

    CParaFormatLayer* richParaFormatLayer = CParaFormatLayer::NewL();
    CleanupStack::PushL(richParaFormatLayer); // 1st push
    CCharFormatLayer* richCharFormatLayer = CCharFormatLayer::NewL(); 
    CleanupStack::PushL(richCharFormatLayer); // 2nd push
    CRichText* 
        richText = CRichText::NewL(richParaFormatLayer,richCharFormatLayer);
    CleanupStack::PushL(richText); // 3rd push

    richText->InsertL(0, Message());

    if(iStatus==ESapVMessageStatusSent)
    {
        SaveSmsToSentL(aMsvSession, richText);
    }
    else
    {
        SaveSmsToInboxL(aMsvSession, richText);
    }

    CleanupStack::PopAndDestroy(3, richParaFormatLayer);

    FLOG( _L( " CSapVMessageParser: SaveSapSmsL: Done\t" ) );
}

// ---------------------------------------------------------
// SaveSapMmsL
// Saves parsed VMessage as MMS notification.
// ---------------------------------------------------------
//
void CSapVMessageParser::SaveSapMmsL(CMsvSession* /*aMsvSession*/) const
{
    // This is not supported
}

// ---------------------------------------------------------
// SaveSmsToInboxL
// Saves parsed VMessage to Inbox as SMS.
// ---------------------------------------------------------
//
void CSapVMessageParser::SaveSmsToInboxL(CMsvSession* aMsvSession, CRichText* aMessage)
{
    #ifndef NO101APPDEPFIXES_NEW
    (void) aMsvSession; 
    (void) aMessage; 
    #endif //NO101APPDEPFIXES_NEW
    #ifdef NO101APPDEPFIXES_NEW
    FLOG( _L( " CSapVMessageParser: SaveSmsToInboxL\t" ) ); 
    CSmsHeader* header=CSmsHeader::NewL(CSmsPDU::ESmsDeliver, *aMessage );
    CleanupStack::PushL(header); // 1st push
    header->SetFromAddressL(Address());
    header->SetReplyPathProvided(EFalse);

    TMsvEntry newTEntry;

    newTEntry.iType = KUidMsvMessageEntry;
    newTEntry.iMtm = KUidMsgTypeSMS;  
    newTEntry.SetComplete(EFalse);
    newTEntry.SetFailed(EFalse);
    newTEntry.SetOperation(EFalse);
    newTEntry.SetMultipleRecipients(EFalse);
    newTEntry.SetVisible(EFalse);  // Make invisible
    // and in preparation to make sure gets cleaned up on errors.
    newTEntry.SetInPreparation(ETrue);
    newTEntry.SetSendingState(KMsvSendStateNotApplicable);
    newTEntry.iServiceId = KMsvLocalServiceIndexEntryId;
    newTEntry.iSize = 0;        
    newTEntry.iDate = iTimeReceived;
    header->Message().SetTime(iTimeOriginal);
    ((CSmsDeliver*)&header->Message().SmsPDU())->SetServiceCenterTimeStamp(iTimeOriginal);

// SMUT Unbranch
    CSmsGetDetDescInterface* smsPlugin = CSmsGetDetDescInterface::NewL();
    CleanupStack::PushL( smsPlugin );

    TBuf<KSmsDescriptionLength> description;
    smsPlugin->GetDescription( header->Message(), description );
    newTEntry.iDescription.Set(description);
    TBuf<KSmsDetailsLength> details;
    smsPlugin->GetDetails( aMsvSession->FileSession(), header->Message(), details );
    newTEntry.iDetails.Set(details);

    CleanupStack::PopAndDestroy( smsPlugin );

    // Create new entry to inbox
    CMsvEntry* inbox = aMsvSession->GetEntryL( KMsvGlobalInBoxIndexEntryId );
    CleanupStack::PushL( inbox );
    inbox->CreateL(newTEntry);
    CleanupStack::PopAndDestroy(inbox);

    TMsvId newEntryId=newTEntry.Id();
    aMsvSession->CleanupEntryPushL(newEntryId); //2nd push

    // Get the created entry
    CMsvEntry* newEntry = aMsvSession->GetEntryL(newEntryId);
    CleanupStack::PushL(newEntry);  // 3rd push

    // Store SMS message to the entry
    CMsvStore* newMessageStore = newEntry->EditStoreL();
    CleanupStack::PushL(newMessageStore);   // 4th push
    header->StoreL(*newMessageStore);
    newMessageStore->StoreBodyTextL(*aMessage);
    newMessageStore->CommitL();

    // Save the size & make visible
    newTEntry = newEntry->Entry();
    newTEntry.iSize = newMessageStore->SizeL();
                    
    // Saved OK. Make the entry visible and flag it as complete.
    newTEntry.SetVisible(ETrue);
    newTEntry.SetInPreparation(EFalse);   
    newTEntry.SetUnread(iStatus==ESapVMessageStatusUnread);
    newTEntry.SetNew(iStatus==ESapVMessageStatusRead);
    newTEntry.SetComplete(ETrue);
    newTEntry.SetReadOnly(ETrue);
    newEntry->ChangeL(newTEntry);

    CleanupStack::PopAndDestroy(2, newEntry);
    aMsvSession->CleanupEntryPop();             
    CleanupStack::PopAndDestroy(header);

    FLOG( _L( " CSapVMessageParser: SaveSmsToInboxL: Done\t" ) );
    #endif  //NO101APPDEPFIXES_NEW
}

// ---------------------------------------------------------
// SaveSmsToSentL
// Saves parsed VMessage to Sent folder as SMS.
// ---------------------------------------------------------
//
void CSapVMessageParser::SaveSmsToSentL(CMsvSession* aMsvSession, CRichText* aMessage)
{
    #ifndef NO101APPDEPFIXES_NEW
    (void) aMsvSession; 
    (void) aMessage; 
    #endif //NO101APPDEPFIXES_NEW
    
    #ifdef NO101APPDEPFIXES_NEW
    FLOG( _L( " CSapVMessageParser: SaveSmsToSentL\t" ) );

    CSmsHeader* header = CSmsHeader::NewL( CSmsPDU::ESmsSubmit, *aMessage );
    CleanupStack::PushL( header );
    header->SetFromAddressL(Address());

    TMsvEntry newTEntry;
    newTEntry.iType = KUidMsvMessageEntry;
    newTEntry.iMtm = KUidMsgTypeSMS;  
    newTEntry.SetComplete(EFalse);
    newTEntry.SetFailed(EFalse);
    newTEntry.SetOperation(EFalse);
    newTEntry.SetMultipleRecipients(EFalse);
    newTEntry.SetVisible(EFalse);  // Make invisible
    // and in preparation to make sure gets cleaned up on errors.
    newTEntry.SetInPreparation(ETrue);
    newTEntry.SetSendingState(KMsvSendStateSent);
    newTEntry.iServiceId = KMsvUnknownServiceIndexEntryId;
    newTEntry.iSize = 0;        
    newTEntry.iDate=iTimeReceived;
    header->Message().SetTime(iTimeOriginal);

// SMUT Unbranch
    CSmsGetDetDescInterface* smsPlugin = CSmsGetDetDescInterface::NewL();
    CleanupStack::PushL( smsPlugin );
    
    TBuf<KSmsDescriptionLength> description;
    smsPlugin->GetDescription( header->Message(), description );
    newTEntry.iDescription.Set(description);
    TBuf<KSmsDetailsLength> details;
    smsPlugin->GetDetails( aMsvSession->FileSession(), header->Message(), details );
    newTEntry.iDetails.Set(details);

    CleanupStack::PopAndDestroy( smsPlugin );

    CSmsSettings* settings = CSmsSettings::NewLC();
    CSmsAccount* account = CSmsAccount::NewLC();
    account->LoadSettingsL(*settings);
    CleanupStack::PopAndDestroy(account);
    header->SetSmsSettingsL( *settings );
    TInt scindex = settings->DefaultServiceCenter();
    if ( scindex != KErrNotFound )
    {
       header->SetServiceCenterAddressL( ( settings->GetServiceCenter( scindex ) ).Address() );
    }
    CleanupStack::PopAndDestroy(settings);

    CSmsNumber* rcpt = CSmsNumber::NewL();
    CleanupStack::PushL( rcpt );
    rcpt->SetAddressL(Address());
    if(Address()!=details)
    {
        rcpt->SetNameL(details);
    }
    header->Recipients().ResetAndDestroy();
    header->Recipients().AppendL( rcpt );
    CleanupStack::Pop( rcpt );

    // Create new entry to Sent folder
    CMsvEntry* sentFldr = aMsvSession->GetEntryL( KMsvSentEntryId );
    CleanupStack::PushL( sentFldr );
    sentFldr->CreateL(newTEntry);
    CleanupStack::PopAndDestroy(sentFldr);

    TMsvId newEntryId=newTEntry.Id();
    aMsvSession->CleanupEntryPushL(newEntryId); //2nd push

    // Get the created entry
    CMsvEntry* newEntry = aMsvSession->GetEntryL(newEntryId);
    CleanupStack::PushL(newEntry);  // 3rd push

    // Store SMS message to the entry
    CMsvStore* newMessageStore = newEntry->EditStoreL();
    CleanupStack::PushL(newMessageStore);   // 4th push
    header->StoreL(*newMessageStore);
    newMessageStore->StoreBodyTextL(*aMessage);
    newMessageStore->CommitL();

    // Save the size & make visible
    newTEntry = newEntry->Entry();
    newTEntry.iSize = newMessageStore->SizeL();
                    
    // Saved OK. Make the entry visible and flag it as complete.
    newTEntry.SetVisible(ETrue);
    newTEntry.SetInPreparation(EFalse);   
    newTEntry.SetComplete(ETrue);
    newEntry->ChangeL(newTEntry);

    CleanupStack::PopAndDestroy(2, newEntry);
    aMsvSession->CleanupEntryPop();             
    CleanupStack::PopAndDestroy(header);

    FLOG( _L( " CSapVMessageParser: SaveSmsToSentL: Done\t" ) );
    #endif  //NO101APPDEPFIXES_NEW
}

//  End of File
