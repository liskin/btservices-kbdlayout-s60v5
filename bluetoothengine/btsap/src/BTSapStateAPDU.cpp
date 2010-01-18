/*
* Copyright (c) 2004-2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*     This class is a BTSapServer's state for setting up connection with SAP client
*
*/

// INCLUDE FILES
#include "BTSapServerState.h"
#include "BTSapSocketHandler.h"
#include "debug.h"

CBTSapServerState::TStateAPDU::TStateAPDU(CBTSapServerState& aServerState)
    : TStateIdle(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateAPDU::Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateAPDU::Enter")));

    iStatus = &aStatus;
    TInt retValue = iServerState.BTSapRequestMessage().GetParameter(EParaCommandAPDU, iApduParameters.iCmdData);
    if(retValue == KErrNone)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintHex(_L8("[BTSap]  TStateAPDU:: iApduCmd %S"), &iApduParameters.iCmdData));
        }
    else
        {
        iServerState.BTSapRequestMessage().GetParameter(EParaCommandAPDU7816, iApduParameters.iCmdData);
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintHex(_L8("[BTSap]  TStateAPDU:: iApduCmd (7816) %S"), &iApduParameters.iCmdData));
        }

    iServerState.SubscriptionModule().SendAPDUReq(aStatus, iApduParameters);

    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintHex(_L8("[BTSap]  TStateAPDU::Enter iApduRsp %S"), &iApduParameters.iRspData));
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateAPDU::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintHex(_L8("[BTSap]  TStateAPDU::Complete iApduRsp %S"), &iApduParameters.iRspData));

    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L8("[BTSap]  TStateAPDU::Complete aReason %d"), aReason));

    iResponseMessage.SetMsgID(ETransferAPDUResponse);
    iResponseMessage.AddParameter(EParaResultCode, iResponseMessage.ToResultCode(aReason));

    if(iResponseMessage.ToResultCode(aReason) == EResultCodeOK)
        {
        iResponseMessage.AddParameter(EParaResponseAPDU, iApduParameters.iRspData);
        }

    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

    return EStateIdle;
    }

void CBTSapServerState::TStateAPDU::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateAPDU: Cancel")));

    iServerState.SubscriptionModule().CancelAsyncRequest(ECustomSendAPDUReqV2IPC);
    TStateIdle::Cancel();
    }

TInt CBTSapServerState::TStateAPDU::ChangeState(TBTSapServerState& aNextState)
    {
    return TState::ChangeState(aNextState);
    }

//  End of File
