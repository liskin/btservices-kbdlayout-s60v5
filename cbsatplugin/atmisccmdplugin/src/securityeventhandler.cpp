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

#include "securityeventhandler.h"

#include "clckcommandhandler.h"
#include "debug.h"

/**
 * AO class for monitoring security events
 */
CSecurityEventHandler* CSecurityEventHandler::NewL(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CSecurityEventHandler* self = new(ELeave) CSecurityEventHandler(aCLCKHandler,aPhone);
    TRACE_FUNC_EXIT
    return self;
    }
      
CSecurityEventHandler::CSecurityEventHandler(CCLCKCommandHandler* aCLCKHandler, RMobilePhone& aPhone) :
    CActive(EPriorityNormal),
    iCLCKHandler(aCLCKHandler),
    iPhone(aPhone)
    {
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    TRACE_FUNC_EXIT
    }

CSecurityEventHandler::~CSecurityEventHandler()
    {
    Cancel();
    }

void CSecurityEventHandler::Start()
    {
    TRACE_FUNC_ENTRY
    iPhone.NotifySecurityEvent(iStatus, iSecurityEvent);
    SetActive();
    TRACE_FUNC_EXIT
    }

void CSecurityEventHandler::RunL()
    {
    TRACE_FUNC_ENTRY    
    Trace(KDebugPrintD, "iStatus.Int(): ", iStatus.Int());
    Trace(KDebugPrintD, "iSecurityEvent: ", iSecurityEvent);
    
    iCLCKHandler->HandleSecurityEvent(iStatus.Int(), iSecurityEvent);
    TRACE_FUNC_EXIT
    }

void CSecurityEventHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iPhone.CancelAsyncRequest(EMobilePhoneVerifySecurityCode);
    TRACE_FUNC_EXIT
    }

