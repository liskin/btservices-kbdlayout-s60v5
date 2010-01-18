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
*     This class is a BTSapServer's state for NotConnected state
*
*
*/


// INCLUDE FILES
#include "BTSapServerState.h"
#include "BTSapSocketHandler.h"
#include "BTSapSimCardStatusNotifier.h"
#include "debug.h"

CBTSapServerState::TStateNotConnected::TStateNotConnected(CBTSapServerState& aServerState)
    : TState(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateNotConnected:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateNotConnected: Enter")));
    iStatus = &aStatus;
    aStatus = KRequestPending;

    NotifySapState(ESapNotConnected);

    iServerState.BTSapSimCardStatusNotifier().Cancel();
    iServerState.BTSapSocketHandler().Listen();
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateNotConnected::Complete(TInt /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateNotConnected: Complete")));
    return EStateConnect;
    }

void CBTSapServerState::TStateNotConnected::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateNotConnected: Cancel")));

    //iServerState.BTSapSocketHandler().Cancel();
    User::RequestComplete(iStatus, KErrCancel);
    }

TInt CBTSapServerState::TStateNotConnected::ChangeState(TBTSapServerState& aNextState)
    {
    TInt retVal = KErrNone;

    if (aNextState != EStateConnect)
        {
        aNextState = EStateNotConnected;
        retVal = KErrNotSupported;
        }

    return retVal;
    }

//  End of File  
