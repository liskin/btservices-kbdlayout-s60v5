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
*     This class is a BTSapSocketHandler's state for sending incoming SAP connection
*
*
*/

// INCLUDE FILES
#include "BTSapSocketHandler.h"
#include "debug.h"

CBTSapSocketHandler::TStateSend::TStateSend(RSocket& aSocket)
    : TState(aSocket)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapSocketHandler::TStateSend:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateSend: Enter: msg size: %d"), iResponseData.Length()));

    iSocket.Write(iResponseData, aStatus);
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintHex(_L8("[BTSap]  Send: %S"), &iResponseData));
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapSocketState CBTSapSocketHandler::TStateSend::Complete(TInt /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateSend: Complete")));

    return EBTSapSocketStateRecv;
    }

void CBTSapSocketHandler::TStateSend::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateSend: Cancel")));

    iSocket.CancelSend();
    }

void CBTSapSocketHandler::TStateSend::SetResponseData(const TDesC8& aResponseData)
    {
    iResponseData = aResponseData;
    }

//  End of File  
