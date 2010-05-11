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

#ifndef PASSTHROUGHHELPER_H
#define PASSTHROUGHHELPER_H

#include <e32base.h>

class CRcpRouter;
class MRemConControlCommandInterface;
class CControlCommand;
NONSHARABLE_CLASS(CPassthroughHelper) : public CBase
	{
public:
	static CPassthroughHelper* NewL(CRcpRouter& aRouter, MRemConControlCommandInterface& aCommandInterface, CDeltaTimer& aTimer);
	~CPassthroughHelper();
	
	void Disconnect();
	
	void HandlePassthrough(CControlCommand& aCommand); // FIXME for now
private:
	CPassthroughHelper(CRcpRouter& aRouter, MRemConControlCommandInterface& aCommandInterface, CDeltaTimer& aTimer);
	void ConstructL();
	
	void Respond(CControlCommand& aCommand, TInt aErr);
	void HandledCommand(CControlCommand& aCommand);

	void NewPress(CControlCommand& aCommand);
	void BalanceHandledCommand(CControlCommand& aCommand);
	
	// Timer functions
	void HoldExpiry(CControlCommand& aCommand);
	void ReleaseExpiry(CControlCommand& aCommand);
	void StartHoldTimer(CControlCommand& aCommand);
	void StartReleaseTimer(CControlCommand& aCommand);
	
	// Delta timer callback functions
	static TInt HoldExpiry(TAny* aExpiryInfo);
	static TInt ReleaseExpiry(TAny* aExpiryInfo);

private:
	CRcpRouter& iRouter;
	MRemConControlCommandInterface& iCommandInterface;
	CDeltaTimer& iTimer;
	CControlCommand* iPreviousPassthrough;
	};

#endif // PASSTHROUGHHELPER_H
