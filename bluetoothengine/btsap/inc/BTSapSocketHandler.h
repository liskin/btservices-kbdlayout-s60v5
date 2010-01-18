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
*	  This class handles data exchange via SAP Socketection initiated by a client
*
*/


#ifndef BT_SAP_SOCKET_HANDLER_H
#define BT_SAP_SOCKET_HANDLER_H

//	INCLUDES
#include <e32base.h>
#include <btsdp.h>

#include "BTSapMessage.h" // KMaxMsgSize
#include "BTSapSniffHandler.h"

enum TBTSapSocketState
	{
	EBTSapSocketStateListen,
	EBTSapSocketStateRecv,
	EBTSapSocketStateSend,
	EBTSapSocketStateCheckSecurity,
	EBTSapSocketStateRecvWrong
	};

// FORWARD DECLARATIONS
class CBTEngDiscovery;
class CBTSapServerState;
class CBTSapRequestHandler;
class CBTSapSecurityHandler;
class TBTDevAddr;

// CLASS DECLARATION

/**
* SAP Bluetooth socket connection handler class.
*/
class CBTSapSocketHandler : public CActive
	{
private:

	class TState
		{
	public:
		TState(RSocket& aSocket);
		virtual void Enter(TRequestStatus& aStatus) = 0;
		virtual TBTSapSocketState Complete(TInt aReason = KErrNone) = 0;
		virtual void Cancel() = 0;

	protected:
		RSocket& iSocket;
		};

	class TStateListen : public TState
		{
	public:
		TStateListen(RSocketServ& aSockServ, RSocket& aSocket,	 RSocket& aListener, TUint aChannel);
		void Enter(TRequestStatus& aStatus);
		TBTSapSocketState Complete(TInt aReason);
		void Cancel();

	private:
		RSocketServ& iSockServ;
		RSocket& iListener;
		TUint& iChannel;
		};

	class TStateRecv : public TState
		{
	public:
		TStateRecv(RSocket& aSocket, CBTSapRequestHandler& aRequestHandler, CBTSapSniffHandler** aSniffHandler);
		void Enter(TRequestStatus& aStatus);
		TBTSapSocketState Complete(TInt aReason);
		void Cancel();

	private:
		TBuf8<KMaxMsgSize> iRequestData;
		TSockXfrLength iSockXfrLength;
		CBTSapRequestHandler& iRequestHandler;
        CBTSapSniffHandler** iSniffHandler;
		};

	class TStateSend : public TState
		{
	public:
		TStateSend(RSocket& aSocket);
		void Enter(TRequestStatus& aStatus);
		TBTSapSocketState Complete(TInt aReason);
		void Cancel();
		void SetResponseData(const TDesC8& aResponseData);

	private:
		TBuf8<KMaxMsgSize> iResponseData;
		};

	class TStateCheckSecurity : public TState
		{
	public:
		TStateCheckSecurity(RSocket& aSocket, CBTSapSecurityHandler*& aSecurityHandler);
		void Enter(TRequestStatus& aStatus);
		TBTSapSocketState Complete(TInt aReason);
		void Cancel();

	private:
		CBTSapSecurityHandler*& iSecurityHandler;
		};

public:
	
	// Constructors
	static CBTSapSocketHandler* NewL(CBTSapServerState& aServerState, CBTSapRequestHandler& aRequestHandler);
	
	// Destructor
	~CBTSapSocketHandler();
		
private:	 // From CActive
   
	/**
	* DoCancel() has to be implemented by all the classes deriving CActive.
	* Request cancellation routine.
	* @param none
	* @return none 
	*/
	void DoCancel();
	
	/**
	* RunL is called by framework after request is being completed.
	* @param nones
	* @return none 
	*/
	void RunL();

public:	   
	
	/**
	* Listen for incoming SAP socket connection
	* @param	None
	* @return	None.
	*/		  
	void Listen();
	
	/**
	* Send response message back to the SAP client
	* @return	None.
	*/	  
	void Send(const TDes8& aResponseData);

	TInt GetRemoteBTAddress(TBTDevAddr& aBTDevAddr);

    TBool IsSapConnected();

private:

	// Two-phase constructor
	void ConstructL();

	// Default constructor
	CBTSapSocketHandler(CBTSapServerState& aServerState, CBTSapRequestHandler& aRequestHandler);

	void RegisterBTSapServiceL();

	void UnregisterBTSapServiceL();

private:

	void ChangeState();

private:

	CBTSapServerState& iServerState;
	CBTSapSecurityHandler* iSecurityHandler;

	TSdpServRecordHandle iSdpHandle; // Registering handle 
	CBTEngDiscovery* iBtDiscovery; // Bluetooth engine
	RSocketServ iSockServ; // Socket server
	RSocket iSocket; // Data socket 
	RSocket iListener; // Listenning socket for incoming connection
	TUint iChannel; // Local BT channel

	TStateListen iStateListen;
	TStateRecv iStateRecv;
	TStateSend iStateSend;
	TStateCheckSecurity iStateCheckSecurity;
	RPointerArray <TState> iStateArray;
	TBTSapSocketState iCurrentState;
	TBTSapSocketState iNextState;

    CBTSapSniffHandler* iSniffHandler;
	};

#endif		// BT_SAP_SOCKET_HANDLER_H	 
			
// End of File
