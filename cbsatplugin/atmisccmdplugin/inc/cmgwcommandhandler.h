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
 * CMGWCommandHandler class declaration for AT+CMGW command
 * 
 */

#ifndef CMGWCOMMANDHANDLER_H
#define CMGWCOMMANDHANDLER_H

#include "atcmdasyncbase.h"

#include <etel3rdparty.h>
#include <etelmm.h>

class CRetrieveMobilePhoneSmspList;
class CMobilePhoneSmspList;

/**
 *  AT+CMGW command handler implementation class
 */
NONSHARABLE_CLASS( CCMGWCommandHandler ) : public CATCmdAsyncBase
    {
private:
    enum TCMGWState
        {
        ECMGWStateIdle,                     // Idle
        ECMGWStateEditMode,                 // Edit mode
        ECMGWStateRetrieveSCA,              // Retrieve SCA
        ECMGWStatePreparePDU,
        ECMGWStateWritePDU,                 // Write PDU
        ECMGWStateSimStoreNotSupported      // Sim store not supported
        };
    
public:
    static CCMGWCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCMGWCommandHandler();
    
    void SetMessageFormat(TInt aFormat);
    
private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    virtual TInt RunError (TInt aError);

private: // methods from CATCmdAsyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCMGWCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();

    void HandleEditModeCommand( const TDesC8& aCmd );
    TInt ParseParameters();
    TInt CreateSmsEntry();
    TInt ExtractSCA();
    
private:
    RMobileSmsMessaging     iMobileSmsMessaging;
    RMobileSmsStore         iMobileSmsStore;
    CRetrieveMobilePhoneSmspList* iRetrieveMobilePhoneSmspList;
    CMobilePhoneSmspList*   iMobilePhoneSmspList;
    RMobileSmsMessaging::TMobileSmspEntryV1 iMobileSmspEntry;
    
    RMobilePhone::TMobileAddress iServiceCentre;
    
    TCMGWState iState;
    RBuf8 iTPDU;
    
    TInt iTPDULength;
    TInt iTPDUStat;
    TUint8 iSCALength;
    RMobileSmsStore::TMobileSmsStoreStatus iMessageStatus;
      
    RMobileSmsStore::TMobileGsmSmsEntryV1 iEntry;
    RMobileSmsStore::TMobileGsmSmsEntryV1Pckg iEntryPckg;
    
    RBuf8 iReply;
    
    TInt iMsgFormat;
    };

#endif /* CMGWCOMMANDHANDLER_H */
