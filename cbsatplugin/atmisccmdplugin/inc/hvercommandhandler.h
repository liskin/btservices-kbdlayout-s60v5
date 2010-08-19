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
 * CHVERCommandHandler class declaration for AT^HVER command
 * 
 */

#ifndef CHVERCOMMANDHANDLER_H
#define CHVERCOMMANDHANDLER_H

#include "atcmdsyncbase.h"
#include "debug.h"

#include <etel3rdparty.h>

/**
 *  AT^HVER command handler implementation class
 */
NONSHARABLE_CLASS( CHVERCommandHandler ) : public CATCmdSyncBase
    {
public:
    static CHVERCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CHVERCommandHandler();
    
public:
    void SetHWVersion(const TDesC8& aHWVersion);
    void SetTelephonyError(TInt aTelError); 

private: // methods from CATCmdSyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CHVERCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    void ParseSWVersion(const TDesC8& aSWVersion);
    
private:
    TInt    iTelError;
    TBuf8<CTelephony::KPhoneModelIdSize> iModel;

    RBuf8 iSWVersion;
    RBuf8 iReply;
    };

#endif /* CHVERCOMMANDHANDLER_H */
