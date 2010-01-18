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

CBTSapServerState::TStatePowerOn::TStatePowerOn(CBTSapServerState& aServerState)
    : TStateIdle(aServerState), iResultCode(EResultCodeReserved), iCardStatus(ECardStatusReserved)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStatePowerOn:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStatePowerOn: Enter")));

    if (iResultCode == EResultCodeReserved)
        {
        iStatus = &aStatus;
        iServerState.SubscriptionModule().PowerSimOn(aStatus);
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
TBTSapServerState CBTSapServerState::TStatePowerOn::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStatePowerOn: Complete: %d"), aReason));

    TBTSapServerState state = EStatePowerOn;

    if (iResultCode == EResultCodeReserved)
        {
        iResultCode = iResponseMessage.ToResultCode(aReason);
        }

    if (iCardStatus != ECardStatusReserved ||
        aReason != KErrNone)
        {
        iResponseMessage.SetMsgID(EPowerSimOnResponse);
        iResponseMessage.AddParameter(EParaResultCode, iResultCode);
        iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

        iResultCode = EResultCodeReserved;
        iCardStatus = ECardStatusReserved;
        state = EStateIdle;
        }

    return state;
    }

void CBTSapServerState::TStatePowerOn::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStatePowerOn: Cancel")));

    if (iResultCode == EResultCodeReserved)
        {
        iServerState.SubscriptionModule().CancelAsyncRequest(ECustomPowerSimOnIPC);
        }
    else
        {
        iResultCode = EResultCodeReserved;
        iCardStatus = ECardStatusReserved;
        TStateIdle::Cancel();
        }
    }

TInt CBTSapServerState::TStatePowerOn::ChangeState(TBTSapServerState& aNextState)
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

void CBTSapServerState::TStatePowerOn::SimCardStatusChanged(TCardStatus aCardStatus)
    {
    iCardStatus = aCardStatus;
    User::RequestComplete(iStatus, KErrNone);
    }

//  End of File  
