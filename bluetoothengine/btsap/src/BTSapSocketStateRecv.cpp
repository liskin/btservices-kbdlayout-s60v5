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
*     This class is a BTSapSocketHandler's state for receiving incoming SAP connection
*
*/

// INCLUDE FILES

#include "BTSapSocketHandler.h"
#include "BTSapRequestHandler.h"
#include "debug.h"

CBTSapSocketHandler::TStateRecv::TStateRecv(RSocket& aSocket,
                                            CBTSapRequestHandler& aRequestHandler,
                                            CBTSapSniffHandler** aSniffHandler)
    : TState(aSocket),
      iRequestHandler(aRequestHandler),
      iSniffHandler(aSniffHandler)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapSocketHandler::TStateRecv:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateRecv: Enter")));

    // Start the sniffer, which will enter sniff-mode after time-out
    (*iSniffHandler)->Enable();

    iRequestData.SetLength(0);
    iSocket.RecvOneOrMore(iRequestData, 0, aStatus, iSockXfrLength);
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapSocketState CBTSapSocketHandler::TStateRecv::Complete(TInt /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateRecv: Complete")));
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintHex(_L8("[BTSap]  Recv: %S"), &iRequestData));

    // We pause the sniffer, to allow time for the response to be sent
    (*iSniffHandler)->Pause();

    iRequestHandler.HandleSapData(iRequestData);
    return EBTSapSocketStateRecv;
    }

void CBTSapSocketHandler::TStateRecv::Cancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateRecv: Cancel")));

    iSocket.CancelRecv();
    }

//  End of File  
