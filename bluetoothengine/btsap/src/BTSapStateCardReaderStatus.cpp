/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
*     This class is a BTSapServer's state for setting up connection with BTSap client
*
*
*/


// INCLUDE FILES
#include "BTSapServerState.h"
#include "BTSapSocketHandler.h"
#include "debug.h"

CBTSapServerState::TStateCardReaderStatus::TStateCardReaderStatus(CBTSapServerState& aServerState)
    : TStateIdle(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateCardReaderStatus:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateCardReaderStatus: Enter")));

    iStatus = &aStatus;
    iServerState.SubscriptionModule().GetSimCardReaderStatus(aStatus, iCardReaderStatus);
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateCardReaderStatus::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateCardReaderStatus: Complete")));

    iResponseMessage.SetMsgID(ETransferCardReaderStatusResponse);
    iResponseMessage.AddParameter(EParaResultCode, iResponseMessage.ToResultCode(aReason));
    iResponseMessage.AddParameter(EParaCardReaderStatus, iCardReaderStatus.GetStatus());
    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());
    return EStateIdle;
    }

void CBTSapServerState::TStateCardReaderStatus::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateCardReaderStatus: Cancel")));

    iServerState.SubscriptionModule().CancelAsyncRequest(ECustomGetSimCardReaderStatusIPC);
    TStateIdle::Cancel();
    }

TInt CBTSapServerState::TStateCardReaderStatus::ChangeState(TBTSapServerState& aNextState)
    {
    return TState::ChangeState(aNextState);
    }

//  End of File  
