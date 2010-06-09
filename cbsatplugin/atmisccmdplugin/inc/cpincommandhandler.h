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
 * Description : CCPIMCommandHandler class declaration for AT+CPIN command 
 *
 */

#ifndef CPINCOMMANDHANDLER_H
#define CPINCOMMANDHANDLER_H

#include "atcmdasyncbase.h"

/**
 *  This class implements an AO for AT+CPIN command using ATExt plugin framework.
 */
NONSHARABLE_CLASS( CCPINCommandHandler ) : public CATCmdAsyncBase
{
public:
    /**
     * @param aCallback is used to notfiy ATExt server to complete the AT command request.
     */
    static CCPINCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCPINCommandHandler();

private:
    // Methods inherited from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError is not required because RunL doesn't leave.

    // method from CATCmdAsyncBase::MATCmdBase
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );

    CCPINCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();

    /**
     * Handle status change event for RMobilePhone::GetLockInfo method
     */
    void HandleGetLockInfo();
    /**
     * Handle status change event for RMobilePhone::HandleVerifySecurityCode method
     */
    void HandleVerifySecurityCode();
    /**
     * Change SIM PIN with PUK code 
     */
    void ChangePassword();
    
private:

    RMobilePhone::TMobilePhoneLockInfoV1 iLockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg iLockInfoPckg;
    
    TInt iPendingEvent;
};


#endif /* CPINCOMMANDHANDLER_H */
