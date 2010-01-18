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

#ifndef COMMANDHANDLERINTERFACE_H
#define COMMANDHANDLERINTERFACE_H

/**
@file
@internalComponent
@released
*/

#include <avctpservices.h>

class CAvrcpCommand;
NONSHARABLE_CLASS(MAvrcpCommandHandler)
	{
public:
	virtual void MessageSent(CAvrcpCommand& aCommand, TInt aSendResult) = 0;
	};

NONSHARABLE_CLASS(MIncomingCommandHandler) : public MAvrcpCommandHandler
	{
public:
	// lower interface 
	virtual void MaxPacketSize(TInt aMtu) = 0;
	virtual void ReceiveCommandL(const TDesC8& aMessageInformation, SymbianAvctp::TTransactionLabel aTransLabel, const TBTDevAddr& aAddr) = 0;
	
	// upper interface
	virtual TInt SendRemConResponse(TUid aInterfaceUid, TUint aId, RBuf8& aData) = 0;
	virtual void SendReject(TUid aInterfaceUid, TUint aTransactionId) = 0;
	};

NONSHARABLE_CLASS(MOutgoingCommandHandler) : public MAvrcpCommandHandler
	{
public:
	// lower interface
	virtual void ReceiveResponse(const TDesC8& aMessageInformation, SymbianAvctp::TTransactionLabel aTransLabel, TBool aIpidBitSet) = 0;
	
	// upper interface
	virtual void SendCommandL(TUid aInterfaceUid, 
		TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr) = 0;
	
	virtual void SendNotifyCommandL(TUid aInterfaceUid, 
		TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr) = 0;
	};

#endif // COMMANDHANDLERINTERFACE_H
