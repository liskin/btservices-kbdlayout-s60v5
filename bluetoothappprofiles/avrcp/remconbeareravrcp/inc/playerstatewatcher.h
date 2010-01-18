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

#ifndef PLAYERSTATEWATCHER_H
#define PLAYERSTATEWATCHER_H

#include <e32base.h>
#include <e32hashtab.h>
#include <remcon/clientid.h>
#include <playerinformationtargetobserver.h>
#include "commandhandlerinterface.h"

class MRemConCommandInterface;
class CInternalCommand;
class CAvrcpCommand;
NONSHARABLE_CLASS(CPlayerWatcherBase) : public CBase, public MIncomingCommandHandler
	{
public:
	virtual void StartWatchingPlayerL(TRemConClientId aClientId) = 0;
	void StopWatchingPlayer(TRemConClientId aClientId);
	
protected:
	virtual void ReceiveUpdate(CInternalCommand& aCommand, TRemConClientId aClientId, RBuf8& aData) = 0;
	virtual void ReceiveReject(TRemConClientId aClientId) = 0;

private: // from MIncomingCommandHandler

	// lower interface 
	void MessageSent(CAvrcpCommand& aCommand, TInt aSendResult);
	void MaxPacketSize(TInt aMtu);
	void ReceiveCommandL(const TDesC8& aMessageInformation, SymbianAvctp::TTransactionLabel aTransLabel, const TBTDevAddr& aAddr);
	
	// upper interface
	TInt SendRemConResponse(TUid aInterfaceUid, TUint aTransactionId, RBuf8& aData) ;
	void SendReject(TUid aInterfaceUid, TUint aTransactionId);
	void Disconnect();
	
protected:
	CPlayerWatcherBase(MRemConCommandInterface& aCommandInterface);
	~CPlayerWatcherBase();
	
private:
	CInternalCommand& FindCommand(TUid aInterfaceUid, TUint aTransactionId, 
			TRemConClientId& aFoundClientId);

	
protected:
	MRemConCommandInterface& iCommandInterface;
	RHashMap<TRemConClientId, CInternalCommand*> iCommands;
	};

NONSHARABLE_CLASS(MPlayStatusObserver)
	{
public:
	virtual void MpsoPlayStatusChanged(TRemConClientId aId, MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus) = 0;
	virtual void MpsoError(TRemConClientId aId) = 0;
	};

NONSHARABLE_CLASS(CPlayStatusWatcher) : public CPlayerWatcherBase
	{
public:
	static CPlayStatusWatcher* NewL(MPlayStatusObserver& aObserver,
			MRemConCommandInterface& aCommandInterface);
	~CPlayStatusWatcher();

	void StartWatchingPlayerL(TRemConClientId aClientId);
	
private:
	CPlayStatusWatcher(MPlayStatusObserver& aObserver,
			MRemConCommandInterface& aCommandInterface);
	
	void SendPlayStatusUpdateRequest(CInternalCommand& aCommand, TRemConClientId& aClientId, MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus);
	void ReceiveUpdate(CInternalCommand& aCommand, TRemConClientId aClientId, RBuf8& aData);
	void ReceiveReject(TRemConClientId aClientId);

private:
	MPlayStatusObserver& iObserver;
	};


NONSHARABLE_CLASS(MUidObserver)
	{
public:
	virtual void MuoUidChanged(TRemConClientId aId, TUint16 aUidCounter) = 0;
	virtual void MuoError(TRemConClientId aId) = 0;
	};

NONSHARABLE_CLASS(CUidWatcher) : public CPlayerWatcherBase
	{
public:
	static CUidWatcher* NewL(MUidObserver& aObserver,
			MRemConCommandInterface& aCommandInterface);
	~CUidWatcher();
	
	void StartWatchingPlayerL(TRemConClientId aClientId);

private:
	CUidWatcher(MUidObserver& aObserver,
			MRemConCommandInterface& aCommandInterface);
	
	void SendUidUpdateRequest(CInternalCommand& aCommand, TRemConClientId& aClientId, TUint16 aUidCounter);
	void ReceiveUpdate(CInternalCommand& aCommand, TRemConClientId aClientId, RBuf8& aData);
	void ReceiveReject(TRemConClientId aClientId);

private:
	MUidObserver& iObserver;
	};

#endif //PLAYERSTATEWATCHER_H
