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
 * CGSNCommandHandler class declaration for AT+CGSN command
 * 
 */

#ifndef CGSNCOMMANDHANDLER_H
#define CGSNCOMMANDHANDLER_H

#include "atcmdsyncbase.h"

#include <etel3rdparty.h>

/**
 *  AT+CGSN command handler implementation class
 */
NONSHARABLE_CLASS( CCGSNCommandHandler ) : public CATCmdSyncBase
    {
public:
    static CCGSNCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCGSNCommandHandler();
    
public:
    void SetSerialNum(const TDesC8& aSerial);
    void SetTelephonyError(TInt aTelError);  

private: // methods from CATCmdSyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCGSNCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
private:
    TInt    iTelError;
    TBuf8<CTelephony::KPhoneSerialNumberSize> iSN;
    
    RBuf8   iReply;
    };

#endif /* CGSNCOMMANDHANDLER_H */
