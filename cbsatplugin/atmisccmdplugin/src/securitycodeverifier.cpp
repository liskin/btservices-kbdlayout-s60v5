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

#include "securitycodeverifier.h"

#include "clckcommandhandler.h"
#include "debug.h"

/**
 * AO class for verifing security codes
 */
CSecurityCodeVerifier* CSecurityCodeVerifier::NewL(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CSecurityCodeVerifier* self = new(ELeave) CSecurityCodeVerifier(aCLCKHandler, aPhone);
    TRACE_FUNC_EXIT
    return self;
    }
      
CSecurityCodeVerifier::CSecurityCodeVerifier(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone) :
    CActive(EPriorityNormal),
    iCLCKHandler(aCLCKHandler),
    iPhone(aPhone)
    {
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    TRACE_FUNC_EXIT
    }

CSecurityCodeVerifier::~CSecurityCodeVerifier()
    {
    Cancel();
    }

void CSecurityCodeVerifier::Start(TDesC8& aPassword, RMobilePhone::TMobilePhoneSecurityCode& aSecurityCode)
    {
    TRACE_FUNC_ENTRY
    RMobilePhone::TMobilePassword code;
    code.Copy(aPassword);

    iPhone.VerifySecurityCode(iStatus, aSecurityCode, code, code);
    SetActive();
    TRACE_FUNC_EXIT
    }

void CSecurityCodeVerifier::RunL()
    {
    TRACE_FUNC_ENTRY    
    Trace(KDebugPrintD, "iStatus.Int(): ", iStatus.Int());
    iCLCKHandler->HandlePasswordVerification(iStatus.Int());
    TRACE_FUNC_EXIT
    }

void CSecurityCodeVerifier::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iPhone.CancelAsyncRequest(EMobilePhoneNotifySecurityEvent);
    TRACE_FUNC_EXIT
    }
