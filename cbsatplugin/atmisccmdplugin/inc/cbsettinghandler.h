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

#ifndef CBSETTTINGHANDLER_H
#define CBSETTTINGHANDLER_H

#include <etelmm.h>

class CCLCKCommandHandler;

/**
 * AO class handling Call Barring setting
 */
NONSHARABLE_CLASS(CCBSettingHandler) : public CActive
    {
public:
    static CCBSettingHandler* NewL(RMobilePhone& aPhone);
    ~CCBSettingHandler();
    void Start(TRequestStatus& aReqStatus, TInt aInfoClass, RMobilePhone::TMobilePhoneCBCondition aCondition, 
            RMobilePhone::TMobilePhoneCBChangeV1* aCBInfo);
    void SetCBForServiceGroup();

private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave
    
private:
    CCBSettingHandler(RMobilePhone& aPhone);
    void ConstructL();
    void Complete(TInt aError);
    
private:
    RMobilePhone& iPhone;
    RArray<RMobilePhone::TMobileService> iClassArray;
    RMobilePhone::TMobilePhoneCBCondition iCondition;
    RMobilePhone::TMobilePhoneCBChangeV1* iCBInfo;
    
    TRequestStatus* iQueuedStatus;
    };

#endif // CBSETTTINGHANDLER_H
