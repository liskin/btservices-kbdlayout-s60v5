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

#ifndef AVRCPOUTGOINGCOMMANDHANDLER_H
#define AVRCPOUTGOINGCOMMANDHANDLER_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>
#include <avctpservices.h>
#include "commandhandlerinterface.h"

using namespace SymbianAvctp;

class CControlCommand;
class CRcpRouter;
class CRemConBearerAvrcp;
class MRemConBearerObserver;
NONSHARABLE_CLASS(CRcpOutgoingCommandHandler) : public CBase, public MOutgoingCommandHandler
	{
public:
	// Construction/Destruction
	static CRcpOutgoingCommandHandler* NewL(CRemConBearerAvrcp& aBearer, 
		MRemConBearerObserver& aObserver,
		CRcpRouter& aRouter,
		CDeltaTimer& aTimer);
	~CRcpOutgoingCommandHandler();
	
	// Called from bearer
	void Disconnect(TBool aClearQueue);
		
	// Delta timer callback functions
	static TInt ResponseExpiry(TAny* aExpiryInfo);
	static TInt ReleaseExpiry(TAny* aExpiryInfo);
	
private:
	// From MOutgoingCommandHandler 
	void MaxPacketSize(TInt aMtu);
	void ReceiveResponse(const TDesC8& aMessageInformation, TTransactionLabel aTransLabel, TBool aIpidBitSet);	
	void MessageSent(CAvrcpCommand& aCommand, TInt aSendResult);
	
	void SendCommandL(TUid aInterfaceUid, 
		TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr);
	
	void SendNotifyCommandL(TUid aInterfaceUid, 
			TUint aCommand, 
			TUint aId,  
			RBuf8& aCommandData, 
			const TBTDevAddr& aAddr);

private:
	CRcpOutgoingCommandHandler(CRemConBearerAvrcp& aBearer, 
		MRemConBearerObserver& aObserver,
		CRcpRouter& aRouter,
		CDeltaTimer& aTimer);
	
	// Internal send utility funcs
	void CleanupUnreleased();
	CControlCommand& SendCommandL(TUid aInterfaceUid,
		TUint aCommand, 
		TUint aId, 
		RBuf8& aCommandData, 
		TBool isClick, 
		const TBTDevAddr& aAddr,
		TBool aKnownToBearer,
		TBool aNotify);	

	void HandleCoreApiCommandL(TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr);

	void GenerateFailureResult(CControlCommand& aCommand, TInt aResult);
	void GenerateCommandL(CControlCommand& aCommand, TRemConCoreApiButtonAction aButtonAct);
		
	// Timer functions
	void ResponseExpiry(CControlCommand& aCommand);
	void ReleaseExpiry(CControlCommand& aCommand);
	void StartResponseTimer(CControlCommand& aCommand);
	void StartReleaseTimer(CControlCommand& aCommand);
	void ClearQueue(TDblQue<CControlCommand>& aQue);
	void ProcessDisconnect(TDblQue<CControlCommand>& aQue, TBool aClearQueue);
	CControlCommand* FindInQueue(TDblQue<CControlCommand>& aQue, SymbianAvctp::TTransactionLabel aTransLabel);
	void ProcessReceiveResponse(CAVCFrame* aFrame, 
								TBool aIpidBitSet,
								CControlCommand* aCommand, 
								TBool aNotify);
private:
	TTransactionLabel		iCurrentTrans;
	TDblQue<CControlCommand>	iCommandQueue;
	TDblQue<CControlCommand>	iNotifyCommandQueue;
	
	CRemConBearerAvrcp&		iBearer;
	MRemConBearerObserver&	iObserver;
	CRcpRouter&				iRouter;
	CDeltaTimer&			iTimer;
	
	CControlCommand*			iUnreleasedCommand;
	TBool					iUnreleasedHasResponse;
	TInt					iReleaseTimerExpiryCount;
	};

#endif // AVRCPOUTGOINGCOMMANDHANDLER_H
