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
#include "BTSapSocketHandler.h"
#include "BTSapServerState.h"
#include "debug.h"

CBTSapServerState::TStateATR::TStateATR(CBTSapServerState& aServerState)
    : TStateIdle(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateATR:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateATR: Enter")));

    iStatus = &aStatus;
    iATR.Zero();
    iServerState.SubscriptionModule().GetATR(aStatus, iATR);
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateATR::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateATR: Complete")));

    iResponseMessage.SetMsgID(ETransferATRResponse);
    iResponseMessage.AddParameter(EParaResultCode, iResponseMessage.ToResultCode(aReason));

    if (aReason == EResultCodeOK)
        {
        iResponseMessage.AddParameter(EParaATR, iATR);
        }

    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());
    return EStateIdle;
    }

void CBTSapServerState::TStateATR::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateATR: Cancel")));

    iServerState.SubscriptionModule().CancelAsyncRequest(ECustomGetATRIPC);
    TStateIdle::Cancel();
    }

TInt CBTSapServerState::TStateATR::ChangeState(TBTSapServerState& aNextState)
    {
    return TState::ChangeState(aNextState);
    }

//  End of File  
