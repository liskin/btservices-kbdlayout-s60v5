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
 * CGMRCommandHandler class declaration for AT+CGMR command
 * 
 */

#ifndef CGMRCOMMANDHANDLER_H
#define CGMRCOMMANDHANDLER_H


#include "atcmdsyncbase.h"

/**
 *  AT+CGMR command handler implementation class
 */
NONSHARABLE_CLASS( CCGMRCommandHandler ) : public CATCmdSyncBase
    {
public:
    static CCGMRCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCGMRCommandHandler();
    
private: // methods from CATCmdSyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCGMRCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    TInt GetSoftwareVersion();
    
private:    
    RBuf8 iReply;
    };

#endif /* CGMRCOMMANDHANDLER_H */
