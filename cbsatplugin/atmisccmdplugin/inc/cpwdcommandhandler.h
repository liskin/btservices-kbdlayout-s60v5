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
 * Description : CPwdCommandHandler class declaration for AT+CPWD command 
 *
 */


#ifndef CPWDCOMMANDHANDLER_H
#define CPWDCOMMANDHANDLER_H
#include <e32base.h>
#include <etelmm.h>

#include "atcmdasyncbase.h"

/**
 *  This class implements an AO for AT+CPWD command using ATExt plugin framework.
 */
NONSHARABLE_CLASS( CCPWDCommandHandler ) : public CATCmdAsyncBase
{
public:
    /**
     * @param aCallback is used to notfiy ATExt server to complete the AT command request.
     */
    static CCPWDCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCPWDCommandHandler();

private:
    // methods inherited from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError is not required because RunL doesn't leave.
    
    // method from CATCmdAsyncBase::MATCmdBase
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
    CCPWDCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    /**
     *  Change password according to AT command arguments
     */
    void ChangePassword();
    
    /**
     * request Etel server to change password and active the AO
     * @param aType type of password to be changed
     * @param aOldPassword old password
     * @param aNewPassword new password
     */
    void ChangeSecurityCode(RMobilePhone::TMobilePhoneSecurityCode aType, TDesC8& aOldPassword, TDesC8& aNewPassword);
    
private:
    TInt iPendingEvent;
};

#endif /* CPWDCOMMANDHANDLER_H */
