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

#ifndef AVRCPBEARERINTERFACE_H
#define AVRCPBEARERINTERFACE_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>
#include <bt_sock.h>

class MIncomingCommandHandler;
class MOutgoingCommandHandler;
NONSHARABLE_CLASS(MAvrcpBearer)
	{
public:
	virtual MIncomingCommandHandler* IncomingHandler(const TBTDevAddr& aAddr) = 0;
	virtual MOutgoingCommandHandler* OutgoingHandler(const TBTDevAddr& aAddr) = 0;
	
	virtual void ConnectIndicate(const TBTDevAddr& aBTDevice) = 0;
	virtual void ConnectConfirm(const TBTDevAddr& aBTDevice, TInt aError) = 0;
	virtual void DisconnectIndicate(const TBTDevAddr& aBTDevice) = 0;
	virtual void DisconnectConfirm(const TBTDevAddr& aBTDevice, TInt aError) = 0;
	};

#endif //AVRCPBEARERINTERFACE_H
