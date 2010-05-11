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

#ifndef AVRCPINCOMINGCOMMANDHANDLER_H
#define AVRCPINCOMINGCOMMANDHANDLER_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>
#include <e32base.h>
#include <remcon/clientid.h>

#include "controlcommand.h"
#include "avrcpplayerinfomanager.h"
#include "commandhandlerinterface.h"

class CPassthroughHelper;
class CRcpRouter;
class MRemConControlCommandInterface;
class CAVCFrame;
class TBTDevAddr;
NONSHARABLE_CLASS(CRcpIncomingCommandHandler) : public CBase, public MPlayerChangeObserver, public MIncomingCommandHandler
	{
public:
	static CRcpIncomingCommandHandler* NewL(MRemConControlCommandInterface& aCommandInterface, 
		MRemConBearerObserver& aObserver,
		CRcpRouter& aRouter,
		CDeltaTimer& aTimer,
		CAvrcpPlayerInfoManager& aPlayerInfoManager, TBTDevAddr& aDevice);
	void ConstructL();
	~CRcpIncomingCommandHandler();
	
	void Disconnect();
			
	inline CAvrcpPlayerInfoManager& AvrcpPlayerInfoManager() const;
	
private: // from MIncomingCommandHandler
	void MaxPacketSize(TInt aMtu);
	void ReceiveCommandL(const TDesC8& aMessageInformation, SymbianAvctp::TTransactionLabel aTransLabel, const TBTDevAddr& aAddr);
	void MessageSent(CAvrcpCommand& aCommand, TInt aSendResult);
	
	TInt SendRemConResponse(TUid aInterfaceUid, TUint aId, RBuf8& aData);
	void SendReject(TUid aInterfaceUid, TUint aId);

private:
	CRcpIncomingCommandHandler(MRemConControlCommandInterface& aCommandInterface, 
		MRemConBearerObserver& aObserver,
		CRcpRouter& aRouter,
		CDeltaTimer& aTimer,
		CAvrcpPlayerInfoManager& aPlayerInfoManager,
		TBTDevAddr& aDevice);
	
	// From MPlayerChangeObserver
	void MpcoAvailablePlayersChanged();
	void MpcoAddressedPlayerChangedLocally(TRemConClientId aClientId);
	void MpcoUidCounterChanged(TRemConClientId aClientId);

	void Respond(CControlCommand& aCommand, TInt aErr);
	void HandledCommand(CControlCommand& aCommand);
	
	void HandleInternalCommand(CControlCommand& aCommand);
	void HandleRemConCommand(CControlCommand& aCommand);

	TInt HandleSetAddressedPlayer(TUint aId, RBuf8& aCommandData);
	void DoHandleSetAddressedPlayerL(RBuf8& aCommandData);
	void AddressedPlayerChangedL(TRemConClientId aClientId);

	TInt HandleRegisterAvailablePlayersNotification(CControlCommand& aCommand);
	void DoHandleRegisterAvailablePlayersNotificationL(RBuf8& aResponseData, CControlCommand& aCommand);

	TInt HandleRegisterAddressedPlayerNotification(CControlCommand& aCommand);
	void DoHandleRegisterAddressedPlayerNotificationL(RBuf8& aResponseData, CControlCommand& aCommand);

	TInt HandleUidChangedNotification( CControlCommand& aCommand);
	void DoHandleUidChangedNotificationL(RBuf8& aResponseData, TUint16 aUidCounter);

	void SendInternalResponse(TUint aId, RBuf8& aData);
	void SendResponse(TDblQue<CControlCommand>& aCommandQueue, TUint aId, RBuf8& aData);
	
	void EnterAddressedMode();

	// Timer functions
	TBool DuplicateNotify(TDblQue<CControlCommand>& aCommandQueue, const CControlCommand& aCommand) const;
	CControlCommand* FindNotify(TDblQue<CControlCommand>& aCommandQueue, TUid aInterfaceUid, TUint aOperationId);
private:
	TDblQue<CControlCommand>	iCommandQueue;
	TDblQue<CControlCommand>	iInternalCommandQueue;
	
	CAVRCPFragmenter*					iFragmenter;
	MRemConControlCommandInterface&		iCommandInterface;
	MRemConBearerObserver&				iObserver;
	CRcpRouter&							iRouter;
	CDeltaTimer& 						iTimer;
	TRemConClientId						iClientId;	// We store the client id even if we aren't
													// in addressed mode as we may switch into
													// it and need to know the current local
													// addressed player
	TBool					iAddressedMode;
	
	CAvrcpPlayerInfoManager&	iPlayerInfoManager;
	TBTDevAddr&					iDevice;
	
	CPassthroughHelper*			iPassthroughHelper;
	};

CAvrcpPlayerInfoManager& CRcpIncomingCommandHandler::AvrcpPlayerInfoManager() const
	{
	return iPlayerInfoManager;
	}

#endif // AVRCPINCOMINGCOMMANDHANDLER_H
