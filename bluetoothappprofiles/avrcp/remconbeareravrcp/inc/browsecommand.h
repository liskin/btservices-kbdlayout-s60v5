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

#ifndef BROWSECOMMAND_H
#define BROWSECOMMAND_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>
#include <e32base.h>
#include <remcon/remconbearerplugin.h>
#include "avrcpcommand.h"
#include "avrcpplayerinfomanager.h"

const TInt KMinLengthSetBrowsedPlayerPdu = 2; // PDU 0x70
const TInt KMinLengthGetFolderItemsPdu = 10; // PDU 0x71
const TInt KMinLengthChangePathPdu = 3; // PDU 0x72
const TInt KMinLengthGetItemAttributesPdu = 12; // PDU 0x73
const TInt KMinLengthSearchPdu = 4; // PDU 0x80
const TInt KMinLengthGeneralReject = 4; // PDU 0xa0

class CRcpRemoteDevice;
class MRemConBearerObserver;
NONSHARABLE_CLASS(CBrowseCommand) : public CAvrcpCommand
	{
public:
	static CBrowseCommand* NewL(const TDesC8& aMessageInformation, 
		TUint aRemConId,
		SymbianAvctp::TTransactionLabel aTransLabel, 
		const TBTDevAddr& aAddr,
		CAvrcpPlayerInfoManager* aPlayerInfoManager);
			
	// Called from handlers
	TInt ProcessIncomingCommandL(TInt aMaxResponse);
	void ProcessOutgoingResponse(RBuf8& aCommandData);
	void SetResult(TInt aErr);
	const TDesC8& CommandData() const;
	
	const TDesC8& Data() const;
	virtual SymbianAvctp::TMessageType MessageType() const;

private:		
	CBrowseCommand(TUint aRemConId,
		SymbianAvctp::TTransactionLabel aTransLabel, 
		const TBTDevAddr& aAddr,
		CAvrcpPlayerInfoManager* aPlayerInfoManager);
		
	~CBrowseCommand();
	void ConstructL(const TDesC8& aMessageInformation);
		
	TInt HandleSetBrowsedPlayer();
	TInt HandleGetFolderItems();
	TInt HandleChangePath();
	TInt HandleGetItemAttributes();
	TInt HandleSearch();
	TInt HandleUnknownPdu();
	
	TInt AppendIncomingPayload(const TPtrC8& aPayload);
	void GenerateRejectPayloadL(TInt aErr);	
			
private:
	RBuf8							iFrame;
	TInt 							iMaxResponse;
	};
	
#endif //BROWSECOMMAND_H
