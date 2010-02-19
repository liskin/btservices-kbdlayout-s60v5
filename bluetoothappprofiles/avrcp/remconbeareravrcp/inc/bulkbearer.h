// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file
 @internalComponent
 @released
*/

#ifndef BULKBEARER_H
#define BULKBEARER_H

#include <e32base.h>
#include <remcon/remconbearerbulkinterface.h>

#include "remconcommandinterface.h"

class CBrowseCommand;
class CRcpBrowsingCommandHandler;
class TBTDevAddr;
class TRemConAddress;

NONSHARABLE_CLASS(CAvrcpBulkBearer) : public CBase, public MRemConBearerBulkInterface,
	public MAvrcpBearer, public MRemConBulkCommandInterface
	{
public:
	static CAvrcpBulkBearer* NewL(RAvctp& aAvctp, CAvrcpPlayerInfoManager& aPlayerInfoManager);
	~CAvrcpBulkBearer();
		
private:	// Interface to command handlers, from MRemConCommandInterface
	void MrcciNewCommand(CAvrcpCommand& aCommand);
	void MrcciNewCommand(CAvrcpCommand& aCommand,const TRemConClientId& aClientId);
	
	TUint MrcciNewTransactionId();
    void MrcciCommandExpired(TUint aTransactionId);
	
	TInt MrcbciSetAddressedClient(const TRemConAddress& aAddr, const TRemConClientId& aClient);
	void MrcbciRemoveAddressing(const TRemConAddress& aAddr);

private: // Interface to router, from MAvrcpBearer
	MIncomingCommandHandler* IncomingHandler(const TBTDevAddr& aAddr);
	MOutgoingCommandHandler* OutgoingHandler(const TBTDevAddr& aAddr);
	
	void ConnectIndicate(const TBTDevAddr& aBTDevice);
	void ConnectConfirm(const TBTDevAddr& aBTDevice, TInt aError);
	void DisconnectIndicate(const TBTDevAddr& aBTDevice);
	void DisconnectConfirm(const TBTDevAddr& aBTDevice, TInt aError);

private: // interface to RemCon, from MRemConBulkBearerInterface
	virtual TInt MrcbbiGetCommand(TUid& aInterfaceUid, 
		TUint& aTransactionId, 
		TUint& aOperationId, 
		RBuf8& aData, 
		TRemConAddress& aAddr);

	virtual TInt MrcbbiSendResponse(TUid aInterfaceUid, 
		TUint aOperationId, 
		TUint aTransactionId, 
		RBuf8& aData, 
		const TRemConAddress& aAddr);

	virtual void MrcbbiSendReject(TUid aInterfaceUid, 
			TUint aOperationId, 
			TUint aTransactionId, 
			const TRemConAddress& aAddr);
	
	virtual TInt MrcbbiStartBulk(MRemConBearerBulkObserver& aObserver);
	virtual void MrcbbiStopBulk();
	
	virtual void MrcbbiBulkClientAvailable(const TRemConClientId& aId);
	virtual void MrcbbiBulkClientNotAvailable(const TRemConClientId& aId);
	
private:
	explicit CAvrcpBulkBearer(CAvrcpPlayerInfoManager& aPlayerInfoManager, RAvctp& aAvctp);
	
	// utility functions
	TBool Operational() const;
	void DoConnectIndicateL(const TBTDevAddr& aBTDevice);
	void DoStartBulkL();
	static TBool CompareBrowsingCommandHandlerByBDAddr(const TBTDevAddr* aKey, const CRcpBrowsingCommandHandler& aHandler);
	void DoNewCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId);
private: // unowned
	MRemConBearerBulkObserver* iObserver;
	CAvrcpPlayerInfoManager& iPlayerInfoManager;
	RAvctp& iAvctp;
	MIncomingCommandHandler* iInternalHandler; 
	
	TDblQue<CAvrcpCommand>	iReadyBrowseCommands;

private: // owned 
	CBulkRouter* iRouter;
	RPointerArray<CRcpBrowsingCommandHandler> iBrowseHandlers;
	
	};

#endif //BULKBEARER_H
