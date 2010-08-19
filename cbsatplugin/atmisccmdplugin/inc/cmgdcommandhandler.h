/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 * 
 * Description :
 * 
 * CCMGDCommandHandler class declaration for AT+CMGD command
 * 
 */

#ifndef CMGDCOMMANDHANDLER_H
#define CMGDCOMMANDHANDLER_H

#include "atcmdasyncbase.h"

#include <etelmm.h>

class CMobilePhoneGsmSmsList;
class CRetrieveMobilePhoneSmsList;
/**
 *  AT+CMGD command handler implementation class
 */
NONSHARABLE_CLASS( CCMGDCommandHandler ) : public CATCmdAsyncBase
    {
private:
/**
 * States for CMGD command handler
 */
enum TCMGDState
    {
    ECMGDStateIdle,
    ECMGDStateRetrieveAllIndices,
    ECMGDStateDeleteOneEntry,
    ECMGDStateDeleteFilteredEntries,
    ECMGDStateDeleteAllEntries,
    ECMGDStateSimStoreNotSupported
    };

enum TCMGDFilter
    {
    ECMGDFilterReadMessagesOnly,
    ECMGDFilterReadAndSentMessages,
    ECMGDFilterReadSentAndUnsentMessages
    };

public:
    static CCMGDCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCMGDCommandHandler();
    
private: // methods from CActive
    
    virtual void RunL();
    virtual void DoCancel();
    virtual TInt RunError(TInt aError);

private: // methods from CATCmdAsyncBase    
    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    
    CCMGDCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    TInt ParseParameters();
    TInt DoHandleCommand();
    
    void GetMessageIndexListL();
    void FilterMessageListL();
    
private:
    
    RMobileSmsMessaging     iMobileSmsMessaging;
    RMobileSmsStore         iMobileSmsStore;
    
    CRetrieveMobilePhoneSmsList* iRetrieveSmsList;
    CMobilePhoneGsmSmsList*      iGsmSmsList;
    
    TCMGDState  iHandlerState;
    TCMGDFilter    iFilterType;
    TInt iIndex;
    TInt iDelFlag;
    RArray<TInt> iDeleteList;
    
    RBuf8 iReply;
    };

#endif /* CMGDCommandHandler_H */
