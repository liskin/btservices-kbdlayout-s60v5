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

#ifndef REMCONCOMMANDINTERFACE_H
#define REMCONCOMMANDINTERFACE_H

#include <e32base.h>
#include <remcon/clientid.h>

class CControlCommand;
class CAvrcpCommand;
class TBTDevAddr;
class TRemConAddress;

NONSHARABLE_CLASS(MRemConCommandInterface)
	{
public:
	virtual void MrcciNewCommand(CAvrcpCommand& aCommand) = 0;
	virtual void MrcciNewCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId) = 0;

	virtual TUint MrcciNewTransactionId() = 0;
	};

NONSHARABLE_CLASS(MRemConBulkCommandInterface) : public MRemConCommandInterface
	{
public:
	virtual TInt MrcbciSetAddressedClient(const TRemConAddress& aAddr, const TRemConClientId& aClient) = 0;
	virtual void MrcbciRemoveAddressing(const TRemConAddress& aAddr) = 0;
	};

NONSHARABLE_CLASS(MRemConControlCommandInterface) : public MRemConCommandInterface
	{
public:
	virtual void MrccciNewNotifyCommand(CAvrcpCommand& aCommand) = 0;
	virtual void MrccciNewNotifyCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId) = 0;

	virtual void MrccciNewResponse(CAvrcpCommand& aCommand) = 0;
	virtual void MrccciNewNotifyResponse(CControlCommand& aCommand) = 0;
	
	virtual void MrccciSetAddressedClient(const TRemConClientId& aClient) = 0;

	};

#endif //REMCONCOMMANDINTERFACE_H
