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

#include "mediabrowse.h"
#include "avrcputils.h"
#include "commandhandlerinterface.h"
#include "internalcommand.h"
#include "mediabrowse.h"
#include "playerstatewatcher.h"
#include "remconcommandinterface.h"
#include "avrcplog.h"
#include "playerinformation.h"

CPlayerWatcherBase::CPlayerWatcherBase(MRemConCommandInterface& aCommandInterface)
	: iCommandInterface(aCommandInterface), iCommands()
	{
	LOG_FUNC;
	}

CPlayerWatcherBase::~CPlayerWatcherBase()
	{
	LOG_FUNC;
	}

void CPlayerWatcherBase::StopWatchingPlayer(TRemConClientId aClientId)
	{
	LOG_FUNC;
	CInternalCommand* command = *iCommands.Find(aClientId);
	__ASSERT_DEBUG(command, AVRCP_PANIC(ENotWatchingPlayer));

	TUint transId = command->RemConCommandId();
	iCommandInterface.MrcciCommandExpired(transId);

	iCommands.Remove(aClientId);
	command->DecrementUsers();
	}

void CPlayerWatcherBase::MessageSent(CAvrcpCommand& /*aCommand*/, TInt /*aSendResult*/)
	{
	LOG_FUNC;
	__DEBUG_ONLY(AVRCP_PANIC(ELowerInterfaceUsedOnInternalHandler));
	}

void CPlayerWatcherBase::MaxPacketSize(TInt /*aMtu*/)
	{
	LOG_FUNC;
	__DEBUG_ONLY(AVRCP_PANIC(ELowerInterfaceUsedOnInternalHandler));
	}

void CPlayerWatcherBase::ReceiveCommandL(const TDesC8& /*aMessageInformation*/, SymbianAvctp::TTransactionLabel /*aTransLabel*/, const TBTDevAddr& /*aAddr*/)
	{
	LOG_FUNC;
	__DEBUG_ONLY(AVRCP_PANIC(ELowerInterfaceUsedOnInternalHandler));
	}

CInternalCommand& CPlayerWatcherBase::FindCommand(TUid __DEBUG_ONLY(aInterfaceUid),
		TUint aTransactionId, TRemConClientId& aFoundClientId)
	{
	__ASSERT_DEBUG(aInterfaceUid == TUid::Uid(KRemConPlayerInformationUid) || aInterfaceUid == TUid::Uid(KRemConMediaBrowseApiUid), AVRCP_PANIC(EResponseForWrongInterface));
	THashMapIter<TRemConClientId, CInternalCommand*> commandIter(iCommands);

	CInternalCommand* command = NULL;
	while(commandIter.NextValue())
		{
		command = *commandIter.CurrentValue();
		if(command->RemConCommandId() == aTransactionId)
			{
			aFoundClientId = *commandIter.CurrentKey();
			break;
			}
		}

	// If command is NULL we reached the end of our iter without finding the match
	__ASSERT_DEBUG(command, AVRCP_PANIC(EUnmatchedResponseFromRemCon));

	return *command;
	}

TInt CPlayerWatcherBase::SendRemConResponse(TUid aInterfaceUid, TUint aTransactionId, RBuf8& aData)
	{
	LOG_FUNC;

	TRemConClientId clientId;
	CInternalCommand& command = FindCommand(aInterfaceUid, aTransactionId, clientId);

	ReceiveUpdate(command, clientId, aData);

	return KErrNone;
	}

void CPlayerWatcherBase::SendReject(TUid aInterfaceUid, TUint aTransactionId)
	{
	TRemConClientId clientId;
	(void)FindCommand(aInterfaceUid, aTransactionId, clientId);

	ReceiveReject(clientId);
	}

void CPlayerWatcherBase::Disconnect()
	{
	LOG_FUNC;
	}

CPlayStatusWatcher* CPlayStatusWatcher::NewL(MPlayStatusObserver& aObserver, MRemConCommandInterface& aCommandInterface)
	{
	LOG_STATIC_FUNC;
	CPlayStatusWatcher* watcher = new(ELeave)CPlayStatusWatcher(aObserver, aCommandInterface);
	return watcher;
	}

CPlayStatusWatcher::CPlayStatusWatcher(MPlayStatusObserver& aObserver,
		MRemConCommandInterface& aCommandInterface)
	: CPlayerWatcherBase(aCommandInterface), iObserver(aObserver)
	{
	LOG_FUNC;
	}

CPlayStatusWatcher::~CPlayStatusWatcher()
	{
	LOG_FUNC;
	}

void CPlayStatusWatcher::StartWatchingPlayerL(TRemConClientId aClientId)
	{
	LOG_FUNC;
	__ASSERT_DEBUG(!iCommands.Find(aClientId), AVRCP_PANIC(EAlreadyWatchingPlayer));

	CInternalCommand* command = CInternalCommand::NewL(TUid::Uid(KRemConPlayerInformationUid),
			0,
			EGetPlayStatusUpdate,
			KNullDesC8);

	CleanupStack::PushL(command);
	iCommands.InsertL(aClientId, command);
	CleanupStack::Pop(command);
	command->IncrementUsers();

	// Initially request uid notification relative to stopped
	SendPlayStatusUpdateRequest(*command, aClientId, MPlayerEventsObserver::EStopped);
	}

void CPlayStatusWatcher::SendPlayStatusUpdateRequest(CInternalCommand& aCommand, TRemConClientId& aClientId, MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus)
	{
	LOG_FUNC;

	RRemConPlayerInformationGetPlayStatusUpdateRequest request;
	request.iStatus = aPlaybackStatus;

	TBuf8<sizeof(MPlayerEventsObserver::TPlaybackStatus)> buf;
	TRAPD(err, request.WriteL(buf));

	// We know how big the request is so this should never fail
	__ASSERT_DEBUG(err == KErrNone, AVRCP_PANIC(EUidUpdateRequestWriteFailure));

	TUint transId = iCommandInterface.MrcciNewTransactionId();
	TRAP(err, aCommand.ResetL(transId, buf));

	if(err == KErrNone)
		{
		iCommandInterface.MrcciNewCommand(aCommand, aClientId);
		}
	else
		{
		// Doom
		iObserver.MpsoError(aClientId);
		}
	}

void CPlayStatusWatcher::ReceiveUpdate(CInternalCommand& aCommand, TRemConClientId aClientId, RBuf8& aData)
	{
	LOG_FUNC;

	// Read 4 byte Big-Endian error code before the payload
	RAvrcpIPCError errorResponse;
	TRAPD(err, errorResponse.ReadL(aData));
	err = err ? err : errorResponse.iError;

	RRemConPlayerInformationGetPlayStatusUpdateResponse response;
	if(!err)
		{
		// Parse the rest of the response (minus error code)
		TRAP(err, response.ReadL(aData.RightTPtr(aData.Length() - KLengthErrorResponse)));
		}

	aData.Close(); // data has been used now

	if(!err)
		{
		iObserver.MpsoPlayStatusChanged(aClientId, response.iStatus);
		SendPlayStatusUpdateRequest(aCommand, aClientId, response.iStatus);
		}
	else
		{
		// Should never get here with a valid player.  This client is
		// sending us junk.
		iObserver.MpsoError(aClientId);
		}
	}

void CPlayStatusWatcher::ReceiveReject(TRemConClientId aClientId)
	{
	LOG_FUNC;
	iObserver.MpsoError(aClientId);
	}

CUidWatcher* CUidWatcher::NewL(MUidObserver& aObserver,
		MRemConCommandInterface& aCommandInterface)
	{
	LOG_STATIC_FUNC;
	CUidWatcher* watcher = new(ELeave)CUidWatcher(aObserver, aCommandInterface);
	return watcher;
	}

CUidWatcher::CUidWatcher(MUidObserver& aObserver,
		MRemConCommandInterface& aCommandInterface)
	: CPlayerWatcherBase(aCommandInterface), iObserver(aObserver)
	{
	LOG_FUNC;
	}

CUidWatcher::~CUidWatcher()
	{
	LOG_FUNC;
	}

void CUidWatcher::StartWatchingPlayerL(TRemConClientId aClientId)
	{
	LOG_FUNC;
	__ASSERT_DEBUG(!iCommands.Find(aClientId), AVRCP_PANIC(EAlreadyWatchingPlayer));

	CInternalCommand* command = CInternalCommand::NewL(TUid::Uid(KRemConMediaBrowseApiUid),
			0,
			EMediaLibraryStateCookieUpdateOperationId,
			KNullDesC8);

	CleanupStack::PushL(command);
	iCommands.InsertL(aClientId, command);
	CleanupStack::Pop(command);
	command->IncrementUsers();

	// Initially request uid notification relative to 0 uid counter
	SendUidUpdateRequest(*command, aClientId, 0);
	}

void CUidWatcher::SendUidUpdateRequest(CInternalCommand& aCommand, TRemConClientId& aClientId, TUint16 aUidCounter)
	{
	LOG_FUNC;
	RRemConUidsChangedRequest request;
	request.iInitialUidCounter = aUidCounter;
	TBuf8<sizeof(TUint16)> buf;
	TRAPD(err, request.WriteL(buf));
	// We know how big the request is so this should never fail
	__ASSERT_DEBUG(err == KErrNone, AVRCP_PANIC(EUidUpdateRequestWriteFailure));

	TUint transId = iCommandInterface.MrcciNewTransactionId();
	TRAP(err, aCommand.ResetL(transId, buf));

	if(err == KErrNone)
		{
		iCommandInterface.MrcciNewCommand(aCommand, aClientId);
		}
	else
		{
		// Doom
		iObserver.MuoError(aClientId);
		}
	}

void CUidWatcher::ReceiveUpdate(CInternalCommand& aCommand, TRemConClientId aClientId, RBuf8& aData)
	{
	LOG_FUNC;
	RRemConUidsChangedResponse response;
	TRAPD(err, response.ReadL(aData));
	aData.Close(); // data has been used now

	if(!err)
		{
		iObserver.MuoUidChanged(aClientId, response.iUidCounter);
		SendUidUpdateRequest(aCommand, aClientId, response.iUidCounter);
		}
	else
		{
		// Should never get here with a valid player.  This client is
		// sending us junk.
		iObserver.MuoError(aClientId);
		}
	}

void CUidWatcher::ReceiveReject(TRemConClientId aClientId)
	{
	LOG_FUNC;
	iObserver.MuoError(aClientId);
	}



