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
 * Description :
 *
 */


#ifndef SECURITYEVENTHANDLER_H
#define SECURITYEVENTHANDLER_H

#include <etelmm.h>

class CCLCKCommandHandler;

/**
 * AO class handling Security Events notifications
 */
NONSHARABLE_CLASS(CSecurityEventHandler) : public CActive
    {
public:
    static CSecurityEventHandler* NewL(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone);
    ~CSecurityEventHandler();
    void Start();
    
private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave

private:
    CSecurityEventHandler(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone);
    void ConstructL();
    
private:
    CCLCKCommandHandler* iCLCKHandler;
    RMobilePhone& iPhone;
    RMobilePhone::TMobilePhoneSecurityEvent iSecurityEvent;
    };

#endif /* SECURITYEVENTHANDLER_H */
