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

#ifndef ABSOLUTEVOLUMESENDER_H
#define ABSOLUTEVOLUMESENDER_H

#include <e32base.h>
#include <absolutevolumeapicontroller.h>
#include <remconinterfaceselector.h>

/**
@file
@internalComponent
@released
*/

NONSHARABLE_CLASS(CAbsVolSender) : public CActive
	{
public:
	static CAbsVolSender* NewL(CRemConInterfaceSelector& aIfSel, MAbsVolSenderObserver& aObserver);
	~CAbsVolSender();
	
	void SendNotify(const TDesC8& aData);
	void SendSetAbsoluteVolume(TUint& aNumRemotes, const TDesC8& aData);

private:
	void RunL();
	void DoCancel();
	
	CAbsVolSender(CRemConInterfaceSelector& aIfSel, MAbsVolSenderObserver& aObserver);
	
private:
	CRemConInterfaceSelector& iIfSel;
	MAbsVolSenderObserver& iObserver;
	TBool iSendingNotify;
	};

#endif // ABSOLUTEVOLUMESENDER_H
