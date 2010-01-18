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

CBTSapServerState::TStateReset::TStateReset(CBTSapServerState& aServerState)
    : TStateIdle(aServerState), iResultCode(EResultCodeReserved), iCardStatus(ECardStatusReserved)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateReset:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateReset: Enter")));

    if (iResultCode == EResultCodeReserved)
        {
        iStatus = &aStatus;
        iServerState.SubscriptionModule().SimWarmReset(aStatus);
        }
    else
        {
        aStatus = KRequestPending;
        }
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateReset::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateReset: Complete: %d"), aReason));

    if (iResultCode == EResultCodeReserved)
        {
        iResultCode = iResponseMessage.ToResultCode(aReason);
        }

    TBTSapServerState state;

    if (iCardStatus == ECardStatusReserved &&
        aReason == KErrNone)
        {
        iResultCode = iResponseMessage.ToResultCode(aReason);
        state = EStateReset;
        }
    else
        {
        iResponseMessage.SetMsgID(EResetSimResponse);
        iResponseMessage.AddParameter(EParaResultCode, iResultCode);
        iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

        iResultCode = EResultCodeReserved;
        iCardStatus = ECardStatusReserved;
        state = EStateIdle;
        }

    return state;
    }

void CBTSapServerState::TStateReset::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateReset: Cancel")));

    if (iResultCode == EResultCodeReserved)
        {
        iServerState.SubscriptionModule().CancelAsyncRequest(ECustomSimWarmResetIPC);
        }
    else
        {
        iResultCode = EResultCodeReserved;
        iCardStatus = ECardStatusReserved;
        TStateIdle::Cancel();
        }
    }

TInt CBTSapServerState::TStateReset::ChangeState(TBTSapServerState& aNextState)
    {
    TInt retVal = KErrNone;

    if (aNextState != EStatePowerOff &&
        aNextState != EStateDisconnect)
        {
        aNextState = EStateIdle;
        retVal = KErrNotSupported;
        }

    return retVal;
    }

void CBTSapServerState::TStateReset::SimCardStatusChanged(TCardStatus aCardStatus)
    {
    iCardStatus = aCardStatus;
    User::RequestComplete(iStatus, KErrNone);
    }

//  End of File  
