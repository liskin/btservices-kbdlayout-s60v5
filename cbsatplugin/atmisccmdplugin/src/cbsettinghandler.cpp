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

#include "cbsettinghandler.h"

#include "clckcommandhandler.h"
#include "debug.h"

/**
 * AO class for setting call barring(s)
 */
CCBSettingHandler* CCBSettingHandler::NewL(RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCBSettingHandler* self = new(ELeave) CCBSettingHandler(aPhone);
    TRACE_FUNC_EXIT
    return self;
    }
      
CCBSettingHandler::CCBSettingHandler(RMobilePhone& aPhone) :
    CActive(EPriorityNormal),
    iPhone(aPhone)
    {
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    TRACE_FUNC_EXIT
    }

CCBSettingHandler::~CCBSettingHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iClassArray.Close();
    TRACE_FUNC_EXIT
    }

void CCBSettingHandler::Start(TRequestStatus& aReqStatus, TInt aInfoClass, 
        RMobilePhone::TMobilePhoneCBCondition aCondition,
        RMobilePhone::TMobilePhoneCBChangeV1* aCBInfo)
    {
    TRACE_FUNC_ENTRY
    
    iCBInfo = aCBInfo;
    iCondition = aCondition;
    aReqStatus = KRequestPending;
    iQueuedStatus = &aReqStatus;
    
    if ((aInfoClass & EInfoClassVoice) == EInfoClassVoice)
        {
        iClassArray.Append(RMobilePhone::ETelephony);
        }
   // TODO: confirm ECircuitDataService is not needed 
    if ((aInfoClass & EInfoClassData) == EInfoClassData)
        {
        iClassArray.Append(RMobilePhone::EAllBearer);
        }
    
    if ((aInfoClass & EInfoClassFax) == EInfoClassFax)
        {
        iClassArray.Append(RMobilePhone::EFaxService);
        }
    
    if ((aInfoClass & EInfoClassSMS) == EInfoClassSMS)
        {
        iClassArray.Append(RMobilePhone::EShortMessageService);
        }

    if ((aInfoClass & EInfoClassSyncData) == EInfoClassSyncData)
        {
        iClassArray.Append(RMobilePhone::ESyncData);
        }

    if ((aInfoClass & EInfoClassASyncData) == EInfoClassASyncData)
        {
        iClassArray.Append(RMobilePhone::EAsyncData);
        }
    
    if ((aInfoClass & EInfoClassPacketData) == EInfoClassPacketData)
        {
        iClassArray.Append(RMobilePhone::EPacketData);
        }
    
    if ((aInfoClass & EInfoClassPadAccess) == EInfoClassPadAccess)
        {
        iClassArray.Append(RMobilePhone::EPadAccess);
        }
    
    SetCBForServiceGroup();
    
    TRACE_FUNC_EXIT
    }

void CCBSettingHandler::SetCBForServiceGroup()
    {
    TRACE_FUNC_ENTRY
    
    Trace(KDebugPrintD, "iClassArray.Count(): ", iClassArray.Count());
    if (iClassArray.Count() != 0)
        {
        iCBInfo->iServiceGroup = iClassArray[0];
        iPhone.SetCallBarringStatus(iStatus, iCondition, *iCBInfo);
        iClassArray.Remove(0);
        SetActive();
        }
    else
        {
        Complete(KErrNone);
        }
    TRACE_FUNC_EXIT
    }

void CCBSettingHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    
    Trace(KDebugPrintD, "iStatus.Int(): ", iStatus.Int());
    
    if (iStatus.Int() != KErrNone )
        {
        Complete(iStatus.Int());
        }
    else
        {
        SetCBForServiceGroup();
        }
    TRACE_FUNC_EXIT
    }

void CCBSettingHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iPhone.CancelAsyncRequest(EMobilePhoneSetCallBarringStatus);
    
    Complete(KErrCancel);

    TRACE_FUNC_EXIT
    }

void CCBSettingHandler::Complete(TInt aError)
    {
    if (iQueuedStatus != NULL)
        {
        User::RequestComplete(iQueuedStatus, aError);
        iQueuedStatus = NULL;
        }
    }
