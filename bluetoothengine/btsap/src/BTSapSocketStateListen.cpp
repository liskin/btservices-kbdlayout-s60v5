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
*	  This class is a BTSapSocketHandler's state for Listenning incoming SAP connection
*
*
*/


// INCLUDE FILES
#include <bt_sock.h>
#include "BTSapSocketHandler.h"
#include "debug.h"

CBTSapSocketHandler::TStateListen::TStateListen(
	RSocketServ& aSockServ, RSocket& aSocket,	RSocket& aListener, TUint aChannel)
	: TState(aSocket), iSockServ(aSockServ), iListener(aListener), iChannel(aChannel)
	{
	}

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapSocketHandler::TStateListen::Enter(TRequestStatus& aStatus)
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateListen::Enter")));

	iSocket.Close(); // make sure it won't be opened twice
    // Open a blank socket (an incoming connection will be transferred to this socket)
    TInt err = iSocket.Open(iSockServ);
    if (err)
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete(status, err);
        return;
        }
    // Pass the blank socket to the listening socket
    iListener.Accept(iSocket, aStatus); 
	}

// --------------------------------------------------------------------------------------------
// Complete
// --------------------------------------------------------------------------------------------
//
TBTSapSocketState CBTSapSocketHandler::TStateListen::Complete(TInt /*aReason*/)
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateListen: Complete")));

#ifdef _BTSAP_MT	
	// this is for easy module testing
	TBuf<16>  name;
	_LIT(KSapModuleTestName, "Mu-Sap-MT");

	CBTMCMSettings::GetLocalBTName(name);
	DEB(Fmt(_L("[BTSap]  CBTSapSocketHandler::TStateListen: local name: %S"), &name));

	if (name == KSapModuleTestName) return EBTSapSocketStateRecv;
#endif

	return EBTSapSocketStateCheckSecurity;
	}

void CBTSapSocketHandler::TStateListen::Cancel()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::TStateListen: Cancel")));

    iListener.CancelAccept();
	}
    
//	End of File	 
