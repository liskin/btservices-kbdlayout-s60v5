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
// avrcpbrowsingincomingcommandhandler.h
//

#ifndef AVRCPBROWSINGINCOMINGCOMMANDHANDLER_H
#define AVRCPBROWSINGINCOMINGCOMMANDHANDLER_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>
#include <e32base.h>
#include "avrcpplayerinfomanager.h"
#include "commandhandlerinterface.h"
#include "remconcommandinterface.h"

class CBulkRouter;
class CAvrcpBulkBearer;
class TBTDevAddr;
class CBrowseCommand;
NONSHARABLE_CLASS(CRcpBrowsingCommandHandler) : public CBase, public MIncomingCommandHandler
	{
public:
	static CRcpBrowsingCommandHandler* NewL(MRemConBulkCommandInterface& aCommandInterface,
		CBulkRouter& aRouter,
		CAvrcpPlayerInfoManager& aPlayerInfoManager,
		const TBTDevAddr& aAddr);
	~CRcpBrowsingCommandHandler();
	
	void Disconnect();
	const TBTDevAddr& BtAddr() const;

private: // from MIncomingCommandHandler
	void MessageSent(CAvrcpCommand& aCommand, TInt aSendResult);
	void MaxPacketSize(TInt aMtu);
	void ReceiveCommandL(const TDesC8& aMessageInformation, SymbianAvctp::TTransactionLabel aTransLabel, const TBTDevAddr& aAddr);
	
	TInt SendRemConResponse(TUid aInterfaceUid, TUint aId, RBuf8& aData);
	void SendReject(TUid aInterfaceUid, TUint aTransactionId);

private:
	CRcpBrowsingCommandHandler(MRemConBulkCommandInterface& aCommandInterface,
		CBulkRouter& aRouter,
		CAvrcpPlayerInfoManager& aPlayerInfoManager,
		const TBTDevAddr& aAddr);
	
	void Respond(CBrowseCommand& aCommand, TInt aErr);
	void HandledCommand(CBrowseCommand& aCommand);
	
	void HandleInternalCommand(CBrowseCommand& aCommand);
	TInt HandleGetFolderItems(TUint aId, RBuf8& aCommandData);
	void DoHandleGetFolderItemsL(RBuf8& aCommandData, RBuf8& aResponseData);
	
	TInt SendInternalResponse(TUint aId, RBuf8& aData);
	TInt SendResponse(TDblQue<CBrowseCommand>& aCommandQueue, TUint aId, RBuf8& aData);
		
	TBool HandleSetBrowsedPlayer(CBrowseCommand& aCommand);

private:
	TDblQue<CBrowseCommand>			iCommandQueue;
	TDblQue<CBrowseCommand>			iInternalCommandQueue;
	
	MRemConBulkCommandInterface&	iCommandInterface;
	CBulkRouter&					iRouter;
	
	TInt							iMtu;
	
	CAvrcpPlayerInfoManager&		iPlayerInfoManager;
	TBTDevAddr						iAddr;
	};

#endif //AVRCPBROWSINGINCOMINGCOMMANDHANDLER_H
