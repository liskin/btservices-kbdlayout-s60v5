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

#ifndef AVRCPCOMMAND_H
#define AVRCPCOMMAND_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>
#include <bluetoothav.h>
#include <bluetooth/avctptypes.h>
#include "internaltypes.h"

class CAvrcpPlayerInfoManager;
/**
This class is a base class for all AVRCP commands.  It provides 
the functionality that is common between the AV/C and Browse 
commands.
*/
NONSHARABLE_CLASS(CAvrcpCommand) : public CBase
	{
public:
	// called from Router
	void Sent();
	virtual SymbianAvctp::TMessageType MessageType() const = 0;
	
	// Called by users of command
	void DecrementUsers();
	void IncrementUsers();

	// Called from bearer
	void GetCommandInfo(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TBTDevAddr& aAddr);

	virtual const TDesC8& Data() const = 0;
	
	const TBTDevAddr& RemoteAddress() const;
	SymbianAvctp::TTransactionLabel TransactionLabel() const;
	TUint RemConCommandId() const;
	
	const TUid& RemConInterfaceUid() const;
	TUint RemConOperationId() const;
	
	TBool KnownToBearer() const;

protected:
	explicit CAvrcpCommand(TUint aRemConId,
			SymbianAvctp::TTransactionLabel aTransactionLabel, 
			const TBTDevAddr& aAddr);
	
public:
	TBTDblQueLink	iHandlingLink;	// Used to form handling queue in command handlers
	TBTDblQueLink	iReadyLink;		// Used to form ready command queue in CRemConBearerAvrcp
	TBTDblQueLink	iSendLink;		// Used to form send queue in CRcpRouter

protected:
	TUint							iRemConId;
	SymbianAvctp::TTransactionLabel	iTransactionLabel;
		
	TBTDevAddr						iRemoteAddr;

	TUid							iInterfaceUid;
	TUint							iOperationId;
	RBuf8							iCommandData;
	
	TUint							iUsers;
	TBool							iKnownToBearer;
	
	CAvrcpPlayerInfoManager*  		iPlayerInfoManager;
	};

#endif //AVRCPCOMMAND_H
