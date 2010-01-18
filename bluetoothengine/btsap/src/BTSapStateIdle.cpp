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

CBTSapServerState::TStateIdle::TStateIdle(CBTSapServerState& aServerState)
    : TState(aServerState)
    {
    }

// ---------------------------------------------------------------------
// Enter
// calling NotifyCardStatus() of RMmCustomAPI
// ----------------------------------------------------------------------
void CBTSapServerState::TStateIdle:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateIdle: Enter")));

    iStatus = &aStatus;
    aStatus = KRequestPending;
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateIdle::Complete(TInt /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateIdle: Complete")));

    return EStateIdle;
    }

void CBTSapServerState::TStateIdle::Cancel()
    {
    if (*iStatus == KRequestPending)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  TStateIdle: Cancel")));
        User::RequestComplete(iStatus, KErrCancel);
        }
    }

TInt CBTSapServerState::TStateIdle::DisconnectSapConnection(TBTSapDisconnectType aType)
    {
    // set iResponse: disconnect_ind
    iResponseMessage.SetMsgID(EDisconnectIndication);
    iResponseMessage.AddParameter(EParaDisconnectionType, aType);
    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

    return KErrNone;
    }

void CBTSapServerState::TStateIdle::SimCardStatusChanged(TCardStatus aCardStatus)
    {
    // status_ind
    iResponseMessage.SetMsgID(EStatusIndication);
    iResponseMessage.AddParameter(EParaStatusChange, aCardStatus);
    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());
    }

TInt CBTSapServerState::TStateIdle::ChangeState(TBTSapServerState& aNextState)
    {
    TInt retVal = KErrNone;

    if (aNextState == EStateIdle || aNextState == EStateConnect)
        {
        aNextState = EStateIdle;
        retVal = KErrNotSupported;
        }

    return retVal;
    }

//  End of File  
