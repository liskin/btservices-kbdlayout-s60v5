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
 * CGMMCommandHandler class declaration for AT+CGMM command
 * 
 */

#ifndef CGMMCOMMANDHANDLER_H
#define CGMMCOMMANDHANDLER_H

#include "atcmdsyncbase.h"

#include <etel3rdparty.h>

/**
 *  AT+CGMM command handler implementation class
 */
NONSHARABLE_CLASS( CCGMMCommandHandler ) : public CATCmdSyncBase
    {
public:
    static CCGMMCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCGMMCommandHandler();
    
public:
    void SetManufacturer(const TDesC8& aManufacturer);
    void SetModelID(const TDesC8& aModelID);
    void SetTelephonyError(TInt aTelError); 

private: // methods from CATCmdSyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCGMMCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
private:
    TInt    iTelError;
    TBuf8<CTelephony::KPhoneManufacturerIdSize> iManufacturer;
    TBuf8<CTelephony::KPhoneModelIdSize> iModel;
    
    RBuf8 iReply;
    };

#endif /* CGMMCOMMANDHANDLER_H */
