/* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "ussdreadmessageimpl.h"

#include "cusdcommandhandler.h"
#include "debug.h"

CUSSDReadMessageImpl* CUSSDReadMessageImpl::NewL(MUSSDCallback* aCallback, RMobileUssdMessaging& aUssdSession)
    {
    TRACE_FUNC_ENTRY
    CUSSDReadMessageImpl* self = new(ELeave) CUSSDReadMessageImpl(aCallback, aUssdSession);
    TRACE_FUNC_EXIT
    return self;
    }

CUSSDReadMessageImpl::CUSSDReadMessageImpl(MUSSDCallback* aCallback, 
                                             RMobileUssdMessaging& aUssdSession):
    CActive(CActive::EPriorityStandard),
    iCallback(aCallback),
    iUssdSession(aUssdSession)
    {
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    TRACE_FUNC_EXIT
    }

CUSSDReadMessageImpl::~CUSSDReadMessageImpl()
    {    
    TRACE_FUNC_ENTRY
    Cancel();
    TRACE_FUNC_EXIT
    }

void CUSSDReadMessageImpl::RunL()
    {
    TRACE_FUNC_ENTRY
    iCallback->HandleReadMessageComplete(iStatus.Int());
    TRACE_FUNC_EXIT
    }

void CUSSDReadMessageImpl::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iUssdSession.CancelAsyncRequest(EMobileUssdMessagingReceiveMessage);
    TRACE_FUNC_EXIT
    }

TInt CUSSDReadMessageImpl::ReadUSSDMessage(TDes8& aReceivedData,
                                            RMobileUssdMessaging::TMobileUssdAttributesV1& aUssdReadAttribute)
    {
    TRACE_FUNC_ENTRY
    
    TInt retCode = KErrNone;
    if(!IsActive())
        {
        TPckg<RMobileUssdMessaging::TMobileUssdAttributesV1> msgAttributes(aUssdReadAttribute);
        iUssdSession.ReceiveMessage(iStatus, aReceivedData, msgAttributes);
        SetActive();
        }
    else
        {
        retCode = KErrInUse;
        }
        
    TRACE_FUNC_EXIT
    return retCode;
    }
