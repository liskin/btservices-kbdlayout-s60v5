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
 * CCFUNCommandHandler class declaration for AT+CFUN command
 * 
 */

#ifndef CFUNCOMMANDHANDLER_H
#define CFUNCOMMANDHANDLER_H

#include <e32property.h> 

#include "atcmdasyncbase.h"

const TInt KGeneralProfileId = 0;
const TInt KOfflineProfileId = 5;

class MProfileEngine;

/**
 *  AT+CFUN command handler implementation class
 */
NONSHARABLE_CLASS( CCFUNCommandHandler ) : public CATCmdAsyncBase
    {
public:
    static CCFUNCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCFUNCommandHandler();
    
private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave
    
private: // method from CATCmdAsyncBase::MATCmdBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CCFUNCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    TInt ActivateProfile(TInt aFunc);
    TInt SetActiveProfile(TInt aProfileId);
    TInt RestartDevice();
    
private:
    RBuf8 iReply;
    
    MProfileEngine* iProfileEngine;
    RProperty iProperty;
    TInt iExpectedState;
    TInt iReset; // default 0 - do not reset the MT before setting it to <fun> power level
    };

#endif /* CFUNCOMMANDHANDLER_H */
