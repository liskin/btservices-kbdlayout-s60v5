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

#ifndef VMESSAGEHANDLER_H
#define VMESSAGEHANDLER_H

//  INCLUDES
#include <obex.h>
#include <msvapi.h>

// MACROS
// None.

// DATA TYPES
// None.

// FUNCTION PROTOTYPES
// None.

// FORWARD DECLARATIONS
// None.

// CLASS DECLARATION
/**
*  CSapVMessageParser
*  
*/
NONSHARABLE_CLASS(CSapVMessageParser) : public CBase
{
public:
    /**
    * Recognises and parses SAP VMessage object
    * @param aReceivedObject received OBEX object.
    * @param aMtmID message type.
    * @param aFile attachment file
    * @param aMsvSession pointer msvsession.
    * @param aTime current time.
    * @return TBool is message recognised and parsed.
    */   
    static TBool HandleMessageL(CObexBufObject*  aReceivedObject, const TUid aMtmID,
                                RFile& aFile, CMsvSession* aMsvSession, TTime aTime);
private:

typedef enum {
    ESapVMessageUnknown,
    ESapVMessageDiscard,
    ESapVMessageTextSMS,
    ESapVMessageMMSNotificationInd
} TSapVMessageType;

typedef enum {
    ESapVMessageStatusUnknown,
    ESapVMessageStatusUnread,
    ESapVMessageStatusRead,
    ESapVMessageStatusSent
} TSapVMessageStatus;

private:
    static CSapVMessageParser* NewLC();
    void ConstructL();
    /**
    * Constructor.
    * @param None
    */
    CSapVMessageParser();

    /**
    * Recognises and parses SAP VMessage object
    * @param aReceivedObject received OBEX object.
    * @param aMtmID message type.
    * @param aFile attachment file
    * @param aTime current time.
    * @return TBool is message recognised and parsed.
    */   
    TBool ParseMessageL(CObexBufObject*  aReceivedObject, const TUid aMtmID,
                        RFile& aFile, TTime aTime);

    /**
    * Saves parsed message.
    * @param aMsvSession pointer msvsession.
    * @return None.
    */   
    void SaveSapMessageL(CMsvSession* aMsvSession);

    /**
    * Saves parsed message as SMS message.
    * @param aMsvSession pointer msvsession.
    * @return None.
    */   
    void SaveSapSmsL(CMsvSession* aMsvSession);

    /**
    * Saves parsed message as MMS notification.
    * @param aMsvSession pointer msvsession.
    * @return None.
    */   
    void SaveSapMmsL(CMsvSession* aMsvSession) const;
    
    /**
    * Recognises active SAP connection
    * @param None.
    * @return TBool is SAP connection active.
    */   
    static TBool IsSapConnectionActive();

    /**
    * Checks MIME type
    * @param aType MIME type.
    * @return TBool is MIME type for VMessage.
    */   
    static TBool CheckMime(const TDesC8& aType);

    /**
    * Checks name
    * @param aName file name.
    * @return TBool is name for SAP VMessage.
    */   
    static TBool CheckName(const TDesC& aName);

    /**
    * Returns Address field of the parsed message
    * @param None.
    * @return TDesC address field of the parsed message.
    */   
    const TDesC& Address() const;

    /**
    * Returns Body text of the parsed message
    * @param None.
    * @return TDesC body text of the parsed message.
    */   
    const TDesC& Message() const;

    /**
    * Destructor.
    */
    ~CSapVMessageParser();

    /**
    * Parses SAP VMessage object
    * @param aData VMessage data.
    * @return None.
    */   
    void SimpleParseL(const TDesC8& aData);

    /**
    * Saves parsed message to Inbox as SMS message.
    * @param aMsvSession pointer msvsession.
    * @return None.
    */   
    void SaveSmsToInboxL(CMsvSession* aMsvSession, CRichText* aMessage);

    /**
    * Saves parsed message to Sent folder as SMS message.
    * @param aMsvSession pointer msvsession.
    * @return None.
    */   
    void SaveSmsToSentL(CMsvSession* aMsvSession, CRichText* aMessage);

private:
    TSapVMessageType iType;
    TSapVMessageStatus iStatus;
    TTime iTimeOriginal;
    TTime iTimeReceived;
    HBufC* iAddress;
    HBufC* iMessage;
};

#endif //VMESSAGEHANDLER_H

// End of File
