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
*     This class is a BTSapServer's state for Init state
*
*
*/


// INCLUDE FILES
#include "BTSapServerState.h"
#include "debug.h"

CBTSapServerState::TStateInit::TStateInit(CBTSapServerState& aServerState)
    : TState(aServerState)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateInit:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateInit: Enter")));
    iStatus = &aStatus;
    aStatus = KRequestPending;
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateInit::Complete(TInt /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateInit: Complete")));
    return EStateNotConnected;
    }

void CBTSapServerState::TStateInit::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateInit: Cancel")));

    User::RequestComplete(iStatus, KErrCancel);
    }

//  End of File  
