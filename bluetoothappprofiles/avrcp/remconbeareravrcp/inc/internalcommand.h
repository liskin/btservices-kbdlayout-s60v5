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

#ifndef INTERNALCOMMAND_H
#define INTERNALCOMMAND_H

#include "avrcpcommand.h"

const SymbianAvctp::TTransactionLabel KDummyTransactionLabel = 0;

NONSHARABLE_CLASS(CInternalCommand) : public CAvrcpCommand
	{
public:
	static CInternalCommand* NewL(TUid aInterfaceUid, 
			TUint aId, 
			TUint aOperationId,
			const TDesC8& aCommandData);
	
	void ResetL(TUint& aId, const TDesC8& aCommandData);
	virtual SymbianAvctp::TMessageType MessageType() const;
	virtual const TDesC8& Data() const;
	
private:
	CInternalCommand(TUid aInterfaceUid, 
			TUint aId, 
			TUint aOperationId);
	~CInternalCommand();
	void ConstructL(const TDesC8& aCommandData);
	};

#endif //INTERNALCOMMAND_H
