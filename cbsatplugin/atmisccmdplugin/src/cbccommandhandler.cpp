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

#include "cbccommandhandler.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCBCCommandHandler* CCBCCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCBCCommandHandler* self = new (ELeave) CCBCCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCBCCommandHandler::CCBCCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCBCCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    TRACE_FUNC_EXIT
    }

CCBCCommandHandler::~CCBCCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CCBCCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    iReply.Zero();
 
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOther, KCBCSupportedCmdsList);
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeBase):
            {
            iPhone.GetBatteryInfo(iStatus, iBatteryInfo);
            SetActive();
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

void CCBCCommandHandler::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    TRACE_FUNC_EXIT
    }


void CCBCCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    if (iStatus.Int() == KErrNone)
        {
        iReply.Append(KAtCBC);
        // status is mapped according to AT command spec: 
        //    <bcs>:
        //    0 MT is powered by the battery
        //    1 MT has a battery connected, but is not powered by it
        //    2 MT does not have a battery connected
        //    3 Recognized power fault, calls inhibited
        switch(iBatteryInfo.iStatus)
            {
            case (RMobilePhone::EPoweredByBattery):
                {
                iReply.AppendNum(0);
                break;
                }
            case (RMobilePhone::EBatteryConnectedButExternallyPowered):
                {
                iReply.AppendNum(1);
                break;
                }
            case (RMobilePhone::ENoBatteryConnected):
                {
                iReply.AppendNum(2);
                break;
                }
            case (RMobilePhone::EPowerFault):
            default:
                {
                iReply.AppendNum(3);
                break;
                }
            }

        iReply.Append(',');
        iReply.AppendNum(iBatteryInfo.iChargeLevel);
        iReply.Append(KOKCRLF);
        
        iCallback->CreateReplyAndComplete(EReplyTypeOther, iReply);
        }
    else
        {
        iCallback->CreateCMEReplyAndComplete(iStatus.Int());   
        }
    TRACE_FUNC_EXIT
    }

void CCBCCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iPhone.CancelAsyncRequest(EMobilePhoneGetBatteryInfo);
    TRACE_FUNC_EXIT
    }
