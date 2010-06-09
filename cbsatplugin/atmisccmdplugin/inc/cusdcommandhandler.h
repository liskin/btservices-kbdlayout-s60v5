/* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 * Description :
 *
 */

#ifndef CUSDCOMMANDHANDLER_H
#define CUSDCOMMANDHANDLER_H

#include <e32base.h>
#include <etelmm.h>
#include <mmretrieve.h>
#include <gsmuelem.h>

#include "atcmdsyncbase.h"

class CUSSDSendMessageImpl;
class CUSSDReadMessageImpl;
/**
 *  Class for accessing plugin information and common functionality
 */
NONSHARABLE_CLASS( MUSSDCallback )
    {
public:
    /**
     * Callback method for handling received USSD messages
     * @param aError - the result code from the read operation
     */
    virtual void HandleReadMessageComplete(TInt aError) = 0;

    /**
      * Callback method for handling sent USSD messages
      * @param aError - the result code from the send operation
      */
    virtual void HandleSendMessageComplete(TInt aError) = 0;
    };

/**
 *  Class for handling USSD requests
 */
NONSHARABLE_CLASS( CCUSDCommandHandler ) : public CATCmdSyncBase,
                                           public MUSSDCallback
{
public:
    static CCUSDCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCUSDCommandHandler();
     
private: // methods from MUSSDCallback
    virtual void HandleSendMessageComplete(TInt aError);
    virtual void HandleReadMessageComplete(TInt aError);
    
private: // methods from CATCmdAsyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    virtual void HandleCommandCancel();
    
private:
    CCUSDCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    TInt ParseCUSDCmd();
    void CCUSDCommandHandler::FindFirstCarriageReturnL( 
         const TDesC& aBuffer ,
         TUint& aSkipChars , 
         TUint& aStartBit );
    
    TInt Pack(const TDesC8& aIn, TDes8& aOut);
    TInt ElementSizeInBits(TSmsDataCodingScheme::TSmsAlphabet aAlphabet) const;
    TInt PackedOctetsRequired( TInt aNumUDUnits ) const;
    TBool IsDcsValid( const TUint8 aDcs ) const;
    void FormatUSSDReceivedData();

private:
    TBool iSendCommandSent;
    RBuf8 iReply;

    RMobileUssdMessaging iUssdMessaging;

    CUSSDSendMessageImpl* iUSSDSender;
    RMobileUssdMessaging::TMobileUssdAttributesV1 iSendAttributes;
    RBuf8 iUSSDCommand;

    CUSSDReadMessageImpl* iUSSDReader;
    RMobileUssdMessaging::TGsmUssdMessageData iReceivedData;
    TInt iDisplayRetMessage;
    RMobileUssdMessaging::TMobileUssdAttributesV1 iReceiveAttributes;
    };

#endif // CUSDCOMMANDHANDLER_H
