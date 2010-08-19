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
 * CGMICommandHandler class declaration for AT+CGMI command
 * 
 */

#ifndef CGMICOMMANDHANDLER_H
#define CGMICOMMANDHANDLER_H

#include "atcmdsyncbase.h"

#include <etel3rdparty.h>

/**
 *  AT+CGMI command handler implementation class
 */
NONSHARABLE_CLASS( CCGMICommandHandler ) : public CATCmdSyncBase
    {
public:
    static CCGMICommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCGMICommandHandler();
    
public:
    void SetManufacturer(const TDesC8& aManufacturer);
    void SetTelephonyError(TInt aTelError); 

private: // methods from CATCmdSyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCGMICommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
private:
    TInt    iTelError;
    TBuf8<CTelephony::KPhoneManufacturerIdSize> iManufacturer;
    
    RBuf8 iReply;
    };

#endif /* CGMICOMMANDHANDLER_H */
