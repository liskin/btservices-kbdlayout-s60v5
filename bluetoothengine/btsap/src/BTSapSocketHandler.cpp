/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies).
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
*	  This class handles data exchange between BTSap server and client
*
*/


// INCLUDE FILES
#include <bt_sock.h>
#include <btextnotifiers.h> // Needed to check the NOTIFIERS_SUPPORT_PASSKEY_MIN_LENGTH flag
#include <btengdiscovery.h>
#include <btengconstants.h>
#include "BTSapSocketHandler.h"
#include "BTSapServerState.h"
#include "BTSapSecurityHandler.h"
#include "debug.h"

// Old versions of RAN always try to connect to the same channel after pairing
// To solve this problem, we try to use always the same channel
const TUint KBTSapProposedChannel = 21; // 21 is big enough and my lucky number ;)
const TInt KListenQueSize = 1;

CBTSapSocketHandler::CBTSapSocketHandler(CBTSapServerState& aServerState, CBTSapRequestHandler& aRequestHandler)
	: CActive(CActive::EPriorityHigh), 
	  iServerState(aServerState),
	  iChannel(0),
	  iStateListen(iSockServ, iSocket, iListener, iChannel),
	  iStateRecv(iSocket, aRequestHandler, &iSniffHandler),
	  iStateSend(iSocket),
	  iStateCheckSecurity(iSocket, iSecurityHandler),
	  iCurrentState(EBTSapSocketStateListen),
	  iNextState(EBTSapSocketStateRecv)
	{
	iStateArray.Append(&iStateListen);
	iStateArray.Append(&iStateRecv);
	iStateArray.Append(&iStateSend);
	iStateArray.Append(&iStateCheckSecurity);
	
	CActiveScheduler::Add(this);
	}

// Destructor.
CBTSapSocketHandler::~CBTSapSocketHandler()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSocketHandler")));

	Cancel();
	iStateArray.Close();

	TRAPD(err, UnregisterBTSapServiceL());
    if(err != KErrNone)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSocketHandler - Couldn't unregister SAP Service!!!")));
        }

	delete iSecurityHandler;
	iSocket.Close();
	iListener.Close();
	iSockServ.Close();
	delete iBtDiscovery;
    delete iSniffHandler;
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::NewL()
// ---------------------------------------------------------
//
CBTSapSocketHandler* CBTSapSocketHandler::NewL(CBTSapServerState& aServerState, CBTSapRequestHandler& aRequestHandler)
	{
	CBTSapSocketHandler* self = new (ELeave) CBTSapSocketHandler(aServerState, aRequestHandler);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::ConstructL
// ---------------------------------------------------------
//
void CBTSapSocketHandler::ConstructL()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: ConstructL")));

	RegisterBTSapServiceL();

	iSecurityHandler = CBTSapSecurityHandler::NewL();
    // Now create the Sniff Handler
    iSniffHandler = CBTSapSniffHandler::NewL(iSocket, iSockServ);
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::DoCancel
// ---------------------------------------------------------
//
void CBTSapSocketHandler::DoCancel()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: DoCancel")));
	iStateArray[iCurrentState]->Cancel();
	}

// ---------------------------------------------------------
// RunL
// ---------------------------------------------------------
//
void CBTSapSocketHandler::RunL()
	{
	TInt status = iStatus.Int();
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: RunL: %d"), status));
	TBool disconnecting = EFalse;
	
	if (iNextState == EBTSapSocketStateListen)
        {
		// In case we are here, we are disconnecting existing connection
		disconnecting = ETrue;
        }

	if (status >= KErrNone)
		{
		TBTSapSocketState nextState = iStateArray[iCurrentState]->Complete(status);
		
		if (iCurrentState == EBTSapSocketStateListen)
		    {
		    // The remote side has created SAP connection
		    // In order to save RAM, the SAP states are not created until here
		    iServerState.CreateStatesL();
		    }

		if (iNextState == EBTSapSocketStateRecv)
			{
			iNextState = nextState;
			}

		ChangeState();
		}
	else
		{
		// BT link loss or other socket errors
        iCurrentState = EBTSapSocketStateRecvWrong;
		iServerState.ChangeState(EStateNotConnected);
		}
		
    if (disconnecting)
        {
        // Disconnect has been completed, notify caller
        iServerState.DisconnectCompleteL(status);
        }
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::Listen
// ---------------------------------------------------------
void CBTSapSocketHandler::Listen()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: Listen")));
	
	iNextState = EBTSapSocketStateListen;

	if (iCurrentState != EBTSapSocketStateSend)
		{
		Cancel();
		ChangeState();
		}
		
    // In order to save RAM, release the SAP states here and create
    // them when the connection has been created.
    TRAP_IGNORE( iServerState.ReleaseStatesL() );

    // Make sure the sniffer is disabled when we're listening for an incoming connection
    iSniffHandler->Disable();
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::Send
// ---------------------------------------------------------
void CBTSapSocketHandler::Send(const TDes8& aResponseData)
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: Send")));

	if (iCurrentState == EBTSapSocketStateRecv ||
		iCurrentState == EBTSapSocketStateSend)
		{
		iStateSend.SetResponseData(aResponseData);
		iNextState = EBTSapSocketStateSend;

		if (iCurrentState == EBTSapSocketStateRecv)
			{
			Cancel();
			ChangeState();
			}
		}
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::GetRemoteBTAddress
// ---------------------------------------------------------
TInt CBTSapSocketHandler::GetRemoteBTAddress(TBTDevAddr& aBTDevAddr)
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: GetRemoteBTAddress >>")));
	TInt retVal = KErrDisconnected;

	if (iCurrentState == EBTSapSocketStateRecv ||
		iCurrentState == EBTSapSocketStateSend)
		{
		TBTSockAddr addr;
		iSocket.RemoteName(addr);
		aBTDevAddr = addr.BTAddr();
		retVal = KErrNone;
		}

	BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: GetRemoteBTAddress: %d"), retVal));
	return retVal;
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::IsSapConnected
// ---------------------------------------------------------
TBool CBTSapSocketHandler::IsSapConnected()
    {
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::IsSapConnected")));
    return (iCurrentState != EBTSapSocketStateListen);
    }

// ---------------------------------------------------------
// CBTSapSocketHandler::ChangeState
// ---------------------------------------------------------
void CBTSapSocketHandler::ChangeState()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: ChangeState: %d"), iNextState));

	iCurrentState = iNextState;
	iNextState = EBTSapSocketStateRecv;
	iStateArray[iCurrentState]->Enter(iStatus);
	SetActive();
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::RegisterBTSapServiceL
// ---------------------------------------------------------
void CBTSapSocketHandler::RegisterBTSapServiceL()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler::RegisterBTSapServiceL")));
	
	User::LeaveIfError(iSockServ.Connect());
	TProtocolDesc pInfo;
	iBtDiscovery = CBTEngDiscovery::NewL();
	TInt err = KErrNone;

	// Old version of RAN always use the same channel, so try to get a fixed one
	iChannel = KBTSapProposedChannel;

    User::LeaveIfError(iSockServ.FindProtocol(TProtocolName(KRFCOMMDesC), pInfo));  // Or other protocol
    User::LeaveIfError(iListener.Open( iSockServ, pInfo.iAddrFamily, pInfo.iSockType, pInfo.iProtocol));

    TRfcommSockAddr addr;
    TBTServiceSecurity sec;

    sec.SetAuthentication( EMitmRequired );
    sec.SetAuthorisation(ETrue);
    sec.SetEncryption(ETrue);
    sec.SetPasskeyMinLength(KRequiredPassKeyLen);
    addr.SetSecurity(sec);
    addr.SetPort(iChannel);
   
    // On return, _should_ contain a free RFCOMM port
    err = iListener.Bind(addr);
    
    if(err)
        {
        TInt freeChnl;
        err = iListener.GetOpt(KRFCOMMGetAvailableServerChannel, KSolBtRFCOMM, freeChnl);
        if (err == KErrNone)
            {
            err = iListener.SetLocalPort(freeChnl);
            iChannel = freeChnl;
            }
        }
    if(err)
        {
        // Try to bind using KRfcommPassiveAutoBind
        iChannel = KRfcommPassiveAutoBind;
        addr.SetPort(iChannel);
        err = iListener.Bind(addr);
        }
    
    if (!err)
        {
		// Update with the actual value
		iChannel = iListener.LocalPort();
		
        // Set the socket to listening
        User::LeaveIfError(iListener.Listen(KListenQueSize));
        }
    else
        {
		BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSocketHandler: RegisterBTSapServiceL: Couldn't register dynamic channel!!! (err = %d)"), err));
		User::Leave( err );		
        }

	// register BTSap service profile
	TUUID uuid(EBTProfileSAP);
	User::LeaveIfError( iBtDiscovery->RegisterSdpRecord(uuid, iChannel, iSdpHandle) );
	}
	
// ---------------------------------------------------------
// CBTSapSocketHandler::UnregisterBTSapServiceL
// ---------------------------------------------------------
void CBTSapSocketHandler::UnregisterBTSapServiceL()
	{
	if(iSdpHandle)
		{
		User::LeaveIfError(iBtDiscovery->DeleteSdpRecord(iSdpHandle));
		}
	}

// ---------------------------------------------------------
// CBTSapSocketHandler::TState::TState
// ---------------------------------------------------------
CBTSapSocketHandler::TState::TState(RSocket& aSocket)
	: iSocket(aSocket)
	{
	}

//	End of File	 
