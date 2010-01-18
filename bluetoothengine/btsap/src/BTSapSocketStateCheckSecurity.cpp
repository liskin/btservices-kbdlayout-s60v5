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
*     This class is a BTSapSocketHandler's state for security checking (length of 
*     encryption key and pass key)
*
*
*/


// INCLUDE FILES
#include "BTSapSecurityHandler.h"
#include "BTSapSocketHandler.h"
#include "debug.h"

CBTSapSocketHandler::TStateCheckSecurity::TStateCheckSecurity(RSocket& aSocket, CBTSapSecurityHandler*& aSecurityHandler)
    : TState(aSocket), iSecurityHandler(aSecurityHandler)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapSocketHandler::TStateCheckSecurity:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateCheckSecurity: Enter")));

    iSecurityHandler->CheckSapSecurity(iSocket, aStatus);
    }

// --------------------------------------------------------------------------------------------
// Complete
// --------------------------------------------------------------------------------------------
//
TBTSapSocketState CBTSapSocketHandler::TStateCheckSecurity::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateCheckSecurity: Complete")));

    TBTSapSecurityCheckResult result = TBTSapSecurityCheckResult(aReason);

    switch (result)
        {
        case EEncryptionKeyTooShort:
            // start notifer here
            break;

        case EPassKeyTooShort:
            // start notifier here
            break;

        default:
            break;
        }

    return (result == ESecurityOK ? EBTSapSocketStateRecv : EBTSapSocketStateListen);
    }

void CBTSapSocketHandler::TStateCheckSecurity::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateCheckSecurity: Cancel")));

    iSecurityHandler->Cancel();
    }

//  End of File  
