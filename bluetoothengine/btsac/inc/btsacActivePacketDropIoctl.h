/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ?Description
*
*/


#ifndef __BTSACACTIVEPACKETDROPIOCTL_H__
#define __BTSACACTIVEPACKETDROPIOCTL_H__

#include <e32base.h>
#include <e32std.h>
#include <es_sock.h> 

/**
 *  An observer pattern. 
 *
 *  This class defines an observer interface for BT Audio Streamer. 
 *
 *  @lib btaudiostreamer.lib
 *  @since S60 v3.1 
 */
class MActivePacketDropIoctlObserver
    {
    public:
    virtual void PacketsDropped(TInt aPacketCount) = 0; 
    };

class CActivePacketDropIoctl : public CActive
	{
public:
	static CActivePacketDropIoctl* NewL(MActivePacketDropIoctlObserver &aObserver, RSocket &aPendingSocket);
	CActivePacketDropIoctl(MActivePacketDropIoctlObserver &aObserver, RSocket &aPendingSocket);
	~CActivePacketDropIoctl();
	void DoCancel();
	void RunL();
	void Start();
private:
	MActivePacketDropIoctlObserver &iObserver;
	RSocket iPendingSocket;
	TPckgBuf<TInt> iPacketsLostPkgBuf;
	};

#endif 
