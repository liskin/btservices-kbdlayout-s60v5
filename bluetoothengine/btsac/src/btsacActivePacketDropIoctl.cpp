/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:   
*
*/




// INCLUDE FILES
#include "debug.h"
#include "btsacActivePacketDropIoctl.h" 
#include <bluetoothav.h>		// Protocol definitions, e.g., TSBCCodecCapabilities

// ================= MEMBER FUNCTIONS =======================

//////////////////////////////////////////////////////////////////

CActivePacketDropIoctl* CActivePacketDropIoctl::NewL(MActivePacketDropIoctlObserver &aObserver, RSocket &aPendingSocket)
	{
	CActivePacketDropIoctl* self = new (ELeave) CActivePacketDropIoctl(aObserver, aPendingSocket);
	return self;
	}

CActivePacketDropIoctl::CActivePacketDropIoctl(MActivePacketDropIoctlObserver &aObserver, RSocket &aPendingSocket) : CActive(0), iObserver(aObserver)
	{
	TRACE_FUNC
	iPendingSocket = aPendingSocket;
	CActiveScheduler::Add(this);
	}

CActivePacketDropIoctl::~CActivePacketDropIoctl()
	{
	TRACE_FUNC
	Cancel();
	}

void CActivePacketDropIoctl::DoCancel()
	{
	TRACE_FUNC
	iPendingSocket.CancelIoctl(); 
	}

void CActivePacketDropIoctl::Start()
	{
	TRACE_FUNC
	if(!IsActive())
		{
		iPendingSocket.Ioctl(ENotifyAvdtpMediaPacketDropped, iStatus, &iPacketsLostPkgBuf, KSolBtAVDTPMedia);	
		SetActive();
		}
	}

void CActivePacketDropIoctl::RunL()
	{
	TRACE_INFO((_L("CActivePacketDropIoctl::RunL() Packets Dropped Notification! %d packets lost"), iPacketsLostPkgBuf()))	
	iObserver.PacketsDropped(iPacketsLostPkgBuf());
	Start();
	}

//  End of File  
