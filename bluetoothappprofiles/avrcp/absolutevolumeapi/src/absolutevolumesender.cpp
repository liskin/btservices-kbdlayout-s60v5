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

#include "absolutevolumesender.h"
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_REMCONABSOLUTEVOLUME);
#endif

CAbsVolSender* CAbsVolSender::NewL(CRemConInterfaceSelector& aIfSel, MAbsVolSenderObserver& aObserver)
	{
	LOG_STATIC_FUNC
	return new(ELeave)CAbsVolSender(aIfSel, aObserver);
	}

CAbsVolSender::CAbsVolSender(CRemConInterfaceSelector& aIfSel, MAbsVolSenderObserver& aObserver)
	: CActive(EPriorityStandard), iIfSel(aIfSel), iObserver(aObserver)
	{
	LOG_FUNC
	CActiveScheduler::Add(this);
	}

CAbsVolSender::~CAbsVolSender()
	{
	LOG_FUNC
	Cancel();
	}

void CAbsVolSender::RunL()
	{
	LOG_LINE
	LOG_FUNC
	iObserver.MavsoSendComplete(iStatus.Int());
	}

void CAbsVolSender::DoCancel()
	{
	LOG_FUNC
	LOG1(_L8("\tiSendingNotify = %d"), iSendingNotify);
	
	if(iSendingNotify)
		{
		iIfSel.SendCancel(ERemConNotifyCommand);
		}
	else
		{
		iIfSel.SendCancel(ERemConCommand);
		}
	}

void CAbsVolSender::SendNotify(const TDesC8& aData)
	{
	LOG_FUNC
	iIfSel.SendNotify(iStatus, 
			TUid::Uid(KRemConAbsoluteVolumeControllerApiUid),
			KRemConAbsoluteVolumeNotification,
			ERemConNotifyCommand,
			ERemConNotifyCommandAwaitingInterim,
			aData);
	iSendingNotify = ETrue;
	
	SetActive();
	}

void CAbsVolSender::SendSetAbsoluteVolume(TUint& aNumRemotes, const TDesC8& aData)
	{
	LOG_FUNC
	iIfSel.Send(iStatus, 
			TUid::Uid(KRemConAbsoluteVolumeControllerApiUid), 
			KRemConSetAbsoluteVolume,
			aNumRemotes,
			ERemConCommand,
			aData);
	iSendingNotify = EFalse;
	
	SetActive();
	}
