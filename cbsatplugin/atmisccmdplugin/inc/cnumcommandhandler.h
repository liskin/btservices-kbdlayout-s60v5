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
 * Description : CCNUMCommandHandler class declaration for AT+CNUM command 
 *
 */

#ifndef CNUMCOMMANDHANDLER_H
#define CNUMCOMMANDHANDLER_H

#include "atcmdasyncbase.h"

/**
 *  This class implements an AO for AT+CPIN command using ATExt plugin framework.
 */
NONSHARABLE_CLASS( CCNUMCommandHandler ) : public CATCmdAsyncBase
{
public:
    /**
     * @param aCallback is used to notfiy ATExt server to complete the AT command request.
     */
    static CCNUMCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser,
                                    RMobilePhone& aPhone, RTelServer& aTelServer);
    ~CCNUMCommandHandler();

private:
    // methods inherited from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError is not required because RunL doesn't leave.

    // method from CATCmdAsyncBase::MATCmdBase
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );

    CCNUMCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser,
                       RMobilePhone& aPhone, RTelServer& aTelServer);
    void ConstructL();

private:
    enum TATNumState
        {
        EATNumGetStoreInfo,
        EATNumReadNumber
        };
    
    RTelServer& iTelServer;
    TInt iPendingEvent;
    
    RMobileONStore iOwnNumberStore;
    RMobileONStore::TMobileONStoreInfoV1 iOwnStoreInfo;
    RMobileONStore::TMobileONStoreInfoV1Pckg iOwnStoreInfoPckg;
    RMobileONStore::TMobileONEntryV1 iOwnNumberEntry;
    RMobileONStore::TMobileONEntryV1Pckg iOwnNumberEntryPckg;
    RBuf8 iReplyBuffer;
};

#endif /* CNUMCOMMANDHANDLER_H */
