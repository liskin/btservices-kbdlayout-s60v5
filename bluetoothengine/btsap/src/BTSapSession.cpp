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
*	  SAP Server session definition.
*
*/

// INCLUDE FILES
#include <BTManClient.h>
#include "BTSapSession.h"
#include "BTSapServer.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code that
// might leave.

// constructor - must pass client to CSession
CBTSapSession::CBTSapSession()
	{
	}

// destructor
CBTSapSession::~CBTSapSession()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSession::~CBTSapSession")));
    TRAPD(err, Server().SessionClosedL());
    if(err != KErrNone)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS,
                        BTSapPrintTrace(_L("[BTSap]  CBTSapSession::~CBTSapSession: Error when closing the session! %d"), err));
        }
	}

// Two-phased constructor.
CBTSapSession* CBTSapSession::NewL()
	{
    return new (ELeave) CBTSapSession();
	}

void CBTSapSession::CreateL()
	{
	Server().SessionOpened();
	}

// ---------------------------------------------------------
// Server
// Return a reference to CBTSapServer
// ---------------------------------------------------------
CBTSapServer& CBTSapSession::Server()
	{
	return *STATIC_CAST(CBTSapServer*, CONST_CAST(CServer2*, CSession2::Server()));
	}

// ---------------------------------------------------------

// ---------------------------------------------------------
// ServiceL
// Calls DispatchMessage under trap harness
//
// ---------------------------------------------------------
void CBTSapSession::ServiceL(const RMessage2& aMessage)
	{
	TRAPD(err, DispatchMessageL(aMessage));

    if(err == KErrPermissionDenied)
        {
        // Fail if the client doesn't have the required capabilities
        aMessage.Complete(err);
        }
    else if (err != KErrNone)
		{
        // A bad descriptor error implies a badly programmed client, so panic it;
		PanicClient(aMessage, err);
		}
	}

// ---------------------------------------------------------
// DispatchMessageL
// service a client request; test the opcode and then do
// appropriate servicing
//
// ---------------------------------------------------------
void CBTSapSession::DispatchMessageL(const RMessage2 &aMessage)
	{
	TInt retVal = KErrNone;

    // Leave with KErrPermissionDenied if the client doesn't have Local Services capability
    aMessage.HasCapabilityL(ECapabilityLocalServices);

	switch (aMessage.Function())
		{
		case EBTSapManageService:
			{
			retVal = Server().ManageService(aMessage.Int0());
			aMessage.Complete(retVal);

			BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: ManageServiceL: %d"), retVal));
			break;
			}

		case EBTSapAcceptSapConnection:
			{
			retVal = Server().AcceptSapConnection();

			BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: AcceptSapConnection: %d"), retVal));
			aMessage.Complete(retVal);
			break;
			}

		case EBTSapRejectSapConnection:
			{
			TBTSapRejectReason reason = STATIC_CAST(TBTSapRejectReason, aMessage.Int0());
			retVal = Server().RejectSapConnection(reason);
			aMessage.Complete(retVal);

			BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: RejectSapConnection: %d"), retVal));
			break;
			}

		case EBTSapDisconnectSapConnection:
			{
			TBTSapDisconnectType type =	 STATIC_CAST(TBTSapDisconnectType, aMessage.Int0());
			retVal = Server().DisconnectSapConnection(type);
			aMessage.Complete(retVal);

			BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: DisconnectSapConnection: %d"), retVal));
			break;
			}

		case EBTSapIsConnected:
			{
			retVal = Server().IsSapConnected();
			aMessage.Complete(retVal);

			BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: IsSapConnected: %d"), retVal));
			break;
			}

		case EBTSapGetRemoteBTAddress:
			{
			TBTDevAddr addr;
			retVal = Server().GetRemoteBTAddress(addr);

			TBTDevAddrPckgBuf pckg(addr);
			aMessage.WriteL(0, pckg);
			aMessage.Complete(retVal);

			TBuf<12> buf; 
			addr.GetReadable(buf);
			BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: GetRemoteBTAddress: %d, addr: %S"), retVal, &buf));
			break;
			}

		default:
			{
			BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSession: DispatchMessage: Bad request #")));
			User::Leave(KErrNotSupported);
			break;
			}
		}
	}

// End of File
