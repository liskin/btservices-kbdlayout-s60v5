// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef AVRCPROUTER_H
#define AVRCPROUTER_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>
#include <e32base.h>
#include "avrcp.h"
#include "avrcpbearerinterface.h"


class CAvrcpCommand;
class CBrowseCommand;

NONSHARABLE_CLASS(CRcpRouter) : public CBase, public MAvctpEventNotify
	{		
private:
	enum TAvrcpRouterState
		{
		EAvrcpRouterSending,
		EAvrcpRouterCanSend
		};

public:
	~CRcpRouter();
		
	// Data functions called from command handlers
	void AddToSendQueue(CAvrcpCommand& aCommand);
	void RemoveFromSendQueue(CAvrcpCommand& aCommand);
	
	// Control functions called from bearer
	TInt ConnectRequest(const TBTDevAddr& aAddr);	
	TInt DisconnectRequest(const TBTDevAddr& aAddr);
	
	// MAvctpEventNotify functions
	virtual void MaenAttachIndicate(const TBTDevAddr& aBTDevice, TInt aMtu, TBool& aAccept);
				 
	virtual void MaenAttachConfirm(const TBTDevAddr& aBTDevice, TInt aMtu, TInt aConnectResult);
	
	virtual void MaenDetachIndicate(const TBTDevAddr& aBTDevice); 
	
	virtual void MaenDetachConfirm(const TBTDevAddr& aBTDevice, TInt aDisconnectResult); 
	
	virtual void MaenMessageReceivedIndicate(const TBTDevAddr& aBTDevice,
					SymbianAvctp::TTransactionLabel aTransactionLabel,
					SymbianAvctp::TMessageType aType,
					TBool aIpidBitSet,
					const TDesC8& aMessageInformation);
					
	virtual void MaenMessageSendComplete(const TBTDevAddr& aBTDevice, 
					SymbianAvctp::TTransactionLabel aTransactionLabel,   
					TInt aSendResult); 

	virtual void MaenCloseComplete();
	
	virtual void MaenErrorNotify(const TBTDevAddr& aBTDevice, TInt aError);			
	
	virtual void MaenExtensionInterfaceL(TUid aInterface, void*& aObject); 	
		
protected:
	explicit CRcpRouter(MAvrcpBearer& aBearer);
	
private:
	// Utility functions
	void Send();
	
protected:
	MAvctpChannel*				iChannel;
	MAvrcpBearer&				iBearer;
	
	TDblQue<CAvrcpCommand>		iSendQueue;
	TAvrcpRouterState			iState;
//	TAvctpChannel				iChannel;

	};

NONSHARABLE_CLASS(CControlRouter) : public CRcpRouter
	{
public:
	static CControlRouter* NewL(RAvctp& aAvctp, MAvrcpBearer& aBearer);
	~CControlRouter();
	
private:
	explicit CControlRouter(RAvctp& aAvctp, MAvrcpBearer& aBearer);
	void ConstructL();
	
private:
	RAvctp&	iAvctp;
	};

NONSHARABLE_CLASS(CBulkRouter) : public CRcpRouter
	{
public:
	static CBulkRouter* NewL(RAvctp& aAvctp, MAvrcpBearer& aBearer);
	~CBulkRouter();
	
private:
	explicit CBulkRouter(RAvctp& aAvctp, MAvrcpBearer& aBearer);
	void ConstructL();
	
private:
	RAvctp&	iAvctp;
	};
#endif // AVRCPROUTER_H
