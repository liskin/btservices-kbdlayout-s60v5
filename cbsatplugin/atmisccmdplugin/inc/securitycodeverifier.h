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

#ifndef SECURITYCODEVERIFIER_H
#define SECURITYCODEVERIFIER_H

#include <etelmm.h>

class CCLCKCommandHandler;

/**
 * AO class handling Secirity Code verification requests
 */
NONSHARABLE_CLASS(CSecurityCodeVerifier) : public CActive
    {
public:
    static CSecurityCodeVerifier* NewL(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone);
    ~CSecurityCodeVerifier();
    void Start(TDesC8& aPassword, RMobilePhone::TMobilePhoneSecurityCode& aSecurityCode);

private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave
    
private:
    CSecurityCodeVerifier(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone);
    void ConstructL();

private:
    CCLCKCommandHandler* iCLCKHandler;
    RMobilePhone& iPhone;
    RMobilePhone::TMobilePhoneSecurityEvent iSecurityEvent;
    };

#endif /* SECURITYCODEVERIFIER_H */
