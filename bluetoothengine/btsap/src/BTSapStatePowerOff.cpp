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

CBTSapServerState::TStatePowerOff::TStatePowerOff(CBTSapServerState& aServerState)
    : TStateIdle(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStatePowerOff:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStatePowerOff: Enter")));

    iStatus = &aStatus;

    iServerState.SubscriptionModule().PowerSimOff(aStatus);
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStatePowerOff::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStatePowerOff: Complete: %d"), aReason));

    iResponseMessage.SetMsgID(EPowerSimOffResponse);
    TResultCode resultCode = iResponseMessage.ToResultCode(aReason);

    // SubscriptionModule always send the same error code: KErrAlreadyExists when powered on/off already
    if (resultCode == EResultCodeAlreadyPowerOn)
        {
        resultCode = EResultCodePowerOff;
        }

    iResponseMessage.AddParameter(EParaResultCode, resultCode);
    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());
    return EStateIdle;
    }

void CBTSapServerState::TStatePowerOff::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStatePowerOff: Cancel")));

    iServerState.SubscriptionModule().CancelAsyncRequest(ECustomPowerSimOffIPC);

    TStateIdle::Cancel();
    }

TInt CBTSapServerState::TStatePowerOff::ChangeState(TBTSapServerState& aNextState)
    {
    TInt retVal = KErrNone;

    if (aNextState != EStateDisconnect)
        {
        aNextState = EStateIdle;
        retVal = KErrNotSupported;
        }

    return retVal;
    }

//  End of File  
