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

CBTSapServerState::TStateDisconnect::TStateDisconnect(CBTSapServerState& aServerState)
    : TStateIdle(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateDisconnect:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateDisconnect: Enter")));

    iStatus = &aStatus;
    User::RequestComplete(iStatus, KErrNone);
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateDisconnect::Complete(TInt /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateDisconnect: Complete")));

    iResponseMessage.SetMsgID(EDisconnectResponse);
    iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

    return EStateNotConnected;
    }

void CBTSapServerState::TStateDisconnect::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateDisconnect: Cancel")));

    TStateIdle::Cancel();
    }

TInt CBTSapServerState::TStateDisconnect::ChangeState(TBTSapServerState& aNextState)
    {
    aNextState = EStateDisconnect;
    return KErrNotSupported;
    }

//  End of File  
