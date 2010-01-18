// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef INTERNALHELPER_H
#define INTERNALHELPER_H

#include <e32base.h>
#include "avrcpplayerinfomanager.h"

class CControlCommand;
class CRemConBearerAvrcp;
class CRcpRouter;
NONSHARABLE_CLASS(CInternalCommandHelper) : public CBase, public MPlayerChangeObserver
	{
public:
	static CInternalCommandHelper* NewL(CRemConBearerAvrcp& aBearer, 
			CRcpRouter& aRouter, TRemConClientId& aClientId, 
			CAvrcpPlayerInfoManager& aPlayerInfoManager);
	~CInternalCommandHelper();
	
	void Disconnect();
	void HandleInternalCommand(CControlCommand& aCommand);
	
private:
	CInternalCommandHelper(CRemConBearerAvrcp& aBearer, 
			CRcpRouter& aRouter, TRemConClientId& aClientId,
			CAvrcpPlayerInfoManager& aPlayerInfoManager);
	void ConstructL();
	
	// From MPlayerChangeObserver
	void MpcoAvailablePlayersChanged();
	void MpcoAddressedPlayerChangedLocally(TRemConClientId aClientId);
	void MpcoUidCounterChanged(TRemConClientId aClientId);
	
	void Respond(CControlCommand& aCommand, TInt aErr);
	void HandledCommand(CControlCommand& aCommand);
	
	TInt HandleSetAddressedPlayer(TUint aId, RBuf8& aCommandData);
	void DoHandleSetAddressedPlayerL(RBuf8& aCommandData);
	void AddressedPlayerChangedL(TRemConClientId aClientId);

	TInt HandleRegisterAvailablePlayersNotification(CControlCommand& aCommand);
	void DoHandleRegisterAvailablePlayersNotificationL(RBuf8& aResponseData, CControlCommand& aCommand);

	TInt HandleRegisterAddressedPlayerNotification(CControlCommand& aCommand);
	void DoHandleRegisterAddressedPlayerNotificationL(RBuf8& aResponseData, CControlCommand& aCommand);

	TInt HandleUidChangedNotification( CControlCommand& aCommand);
	void DoHandleUidChangedNotificationL(RBuf8& aResponseData, TUint16 aUidCounter);

	void SendResponse(TUint aId, RBuf8& aData);
	
private:
	CRemConBearerAvrcp& iBearer;
	CRcpRouter& iRouter;
	TRemConClientId& iClientId; 
	
	CAvrcpPlayerInfoManager& iPlayerInfoManager;

	TDblQue<CControlCommand> iInternalCommandQueue;
	};

#endif // INTERNALHELPER_H
