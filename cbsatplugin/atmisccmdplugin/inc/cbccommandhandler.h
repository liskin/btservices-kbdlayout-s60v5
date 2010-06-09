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
 * CCBCCommandHandler class declaration for AT+CBC command
 * 
 */

#ifndef CBCCOMMANDHANDLER_H
#define CBCCOMMANDHANDLER_H

#include "atcmdasyncbase.h"

/**
 *  AT+CBC command handler implementation class
 */
NONSHARABLE_CLASS( CCBCCommandHandler ) : public CATCmdAsyncBase
    {
public:
    static CCBCCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCBCCommandHandler();
    
private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave

private: // method from CATCmdAsyncBase::MATCmdBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCBCCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
private:
    RBuf8 iReply;
    
    RMobilePhone::TMobilePhoneBatteryInfoV1 iBatteryInfo;
    };

#endif /* CBCCOMMANDHANDLER_H */
