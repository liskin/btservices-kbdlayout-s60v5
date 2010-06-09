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

#include "ussdsendmessageimpl.h"

#include "cusdcommandhandler.h"
#include "debug.h"

CUSSDSendMessageImpl* CUSSDSendMessageImpl::NewL(MUSSDCallback* aCallback, 
                                                   RMobileUssdMessaging& aUssdSession)
    {
    TRACE_FUNC_ENTRY
    CUSSDSendMessageImpl* self = new(ELeave) CUSSDSendMessageImpl(aCallback, aUssdSession);
    TRACE_FUNC_EXIT
    return self;
    }

CUSSDSendMessageImpl::CUSSDSendMessageImpl(MUSSDCallback* aCallback, 
                                             RMobileUssdMessaging& aUssdSession):
    CActive(CActive::EPriorityStandard),
    iCallback(aCallback),
    iUssdSession(aUssdSession)
    {
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    TRACE_FUNC_EXIT
    }

void CUSSDSendMessageImpl::RunL()
    {
    TRACE_FUNC_ENTRY
    iCallback->HandleSendMessageComplete(iStatus.Int());
    TRACE_FUNC_EXIT
    }

void CUSSDSendMessageImpl::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iUssdSession.CancelAsyncRequest(EMobileUssdMessagingSendMessage);   
    TRACE_FUNC_EXIT
    }

TInt CUSSDSendMessageImpl::SendUSSDCmd(const TDesC8& aCmd, 
                                        RMobileUssdMessaging::TMobileUssdAttributesV1& aUSSDAttribute)
    {    
    TRACE_FUNC_ENTRY
    TInt retCode = KErrNone;
    if(!IsActive())
        {
        // Get USSD messaging caps
        RMobileUssdMessaging::TMobileUssdCapsV1 ussdCaps;
        RMobileUssdMessaging::TMobileUssdCapsV1Pckg ussdCapsPckg(ussdCaps);
   
        iUssdSession.GetCaps(ussdCapsPckg);   
        ussdCaps=ussdCapsPckg();
        
        if(ussdCaps.iUssdFormat != RMobileUssdMessaging::KCapsPackedString)
            {
            retCode =  KErrArgument;
            }
        else
            {
            // we only support packed strings?
            aUSSDAttribute.iFormat = RMobileUssdMessaging::EFormatPackedString;
            TPckg<RMobileUssdMessaging::TMobileUssdAttributesV1> msgAttributes(aUSSDAttribute);
            iUssdSession.SendMessage(iStatus, aCmd, msgAttributes);
            SetActive();
            }   
        }
    else
        {
        retCode = KErrInUse;
        }
    TRACE_FUNC_EXIT
    return retCode;
    }

CUSSDSendMessageImpl::~CUSSDSendMessageImpl()
    {    
    TRACE_FUNC_ENTRY
    Cancel();
    TRACE_FUNC_EXIT
    }
