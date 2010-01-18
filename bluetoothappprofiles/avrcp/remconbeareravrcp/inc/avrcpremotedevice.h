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

#ifndef AVRCPREMOTEDEVICE_H
#define AVRCPREMOTEDEVICE_H

/**
@file
@internalComponent
@released
*/

#include <e32std.h>

class CRemConBearerAvrcp;
class MIncomingCommandHandler;
class MOutgoingCommandHandler;
class CRcpIncomingCommandHandler;
class CRcpOutgoingCommandHandler;
class CRcpRouter;
class MRemConBearerObserver;
/** 
CRcpRemoteDevice represents one remote RCP device.  It manages
the sending and receiving of commands between that device and the 
local device.
*/
NONSHARABLE_CLASS(CRcpRemoteDevice) : public CBase
	{
public:
	static CRcpRemoteDevice* NewL(const TBTDevAddr& aAddr,
		CRcpRouter& aRouter, 
		CRemConBearerAvrcp& aBearer,
		MRemConBearerObserver& aObserver,
		CDeltaTimer& aTimer,
		CAvrcpPlayerInfoManager& aPlayerInfoManager);
		
	virtual ~CRcpRemoteDevice();
	
	void Disconnect(TBool aClearQueue);

	// Utility	
	const TBTDevAddr& RemoteAddress() const;
	MIncomingCommandHandler& IncomingHandler() const;
	MOutgoingCommandHandler& OutgoingHandler() const;

private:
	CRcpRemoteDevice(const TBTDevAddr& aAddr,
		CRcpRouter& aRouter, 
		CRemConBearerAvrcp& aBearer,
		CDeltaTimer& aTimer);
		
	void ConstructL(MRemConBearerObserver& aObserver,
					CAvrcpPlayerInfoManager& aPlayerInfoManager);

public:
	TDblQueLink						iLink;			// Used by bearer and router to manage remotes
private:
	TBTDevAddr						iDevice;
	
	CRcpIncomingCommandHandler*		iIncoming;
	CRcpOutgoingCommandHandler*		iOutgoing;

	CRcpRouter&						iRouter;
	CRemConBearerAvrcp&				iBearer;
	CDeltaTimer&					iTimer;
	};


#endif // AVRCPREMOTEDEVICE_H
