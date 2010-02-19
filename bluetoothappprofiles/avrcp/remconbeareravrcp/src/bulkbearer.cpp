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



/**
 @file
 @internalComponent
 @released
*/

#include <remconaddress.h>
#include <remconbeareravrcp.h>
#include <remcon/remconbearerbulkobserver.h>

#include "avrcpbrowsingcommandhandler.h"
#include "avrcprouter.h"
#include "avrcputils.h"
#include "browsecommand.h"
#include "bulkbearer.h"
#include "playerstatewatcher.h"

#include "avrcplog.h"

#ifdef _DEBUG
PANICCATEGORY("avrcpbulk");
#endif // _DEBUG

CAvrcpBulkBearer* CAvrcpBulkBearer::NewL(RAvctp& aAvctp, CAvrcpPlayerInfoManager& aPlayerInfoManager)
	{
	LOG_STATIC_FUNC
	CAvrcpBulkBearer* bulkBearer = new(ELeave) CAvrcpBulkBearer(aPlayerInfoManager, aAvctp);
	return bulkBearer;
	}

CAvrcpBulkBearer::~CAvrcpBulkBearer()
	{
	LOG_FUNC
	ASSERT_DEBUG(!iRouter); // Should already be stopped...
	}

CAvrcpBulkBearer::CAvrcpBulkBearer(CAvrcpPlayerInfoManager& aPlayerInfoManager, RAvctp& aAvctp)
	: iPlayerInfoManager(aPlayerInfoManager)
	, iAvctp(aAvctp)
	, iReadyBrowseCommands(_FOFF(CAvrcpCommand, iReadyLink))
	{
	LOG_FUNC
	}

MIncomingCommandHandler* CAvrcpBulkBearer::IncomingHandler(const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	
	MIncomingCommandHandler* handler = NULL;
	TInt ix = iBrowseHandlers.Find(aAddr, CAvrcpBulkBearer::CompareBrowsingCommandHandlerByBDAddr);
	if(ix >= 0)
		{
		handler = iBrowseHandlers[ix];
		}

	return handler;
	}

MOutgoingCommandHandler* CAvrcpBulkBearer::OutgoingHandler(const TBTDevAddr& /*aAddr*/)
	{
	LOG_FUNC
	// We've received a response, but we haven't sent a command.  Naughty remote,
	// just ignore it.
	return NULL;
	}

void CAvrcpBulkBearer::DoConnectIndicateL(const TBTDevAddr& aBTDevice)
	{
	LOG_FUNC
	ASSERT_BULK_THREAD;
	
	ASSERT_DEBUG(Operational());
	
	CRcpBrowsingCommandHandler* handler = CRcpBrowsingCommandHandler::NewL(*this, *iRouter, iPlayerInfoManager, aBTDevice);
	CleanupStack::PushL(handler);
	
	iBrowseHandlers.AppendL(handler);
	
	CleanupStack::Pop(handler);
	}
	
void CAvrcpBulkBearer::ConnectIndicate(const TBTDevAddr& aBTDevice)
	{
	LOG_FUNC
	// If we failed to allocate a handler for this connection the router will
	// not be able to find it when it checks, and will tell AVCTP that we're 
	// not interested in this connection.
	TRAP_IGNORE(DoConnectIndicateL(aBTDevice));
	}

void CAvrcpBulkBearer::ConnectConfirm(const TBTDevAddr& IF_FLOGGING(aBTDevice), TInt IF_FLOGGING(aError))
	{
	LOG_FUNC
	LOGBTDEVADDR(aBTDevice);
	LOG1(_L("\taError = %d"), aError);
	
	// Outlandish!  We did not ask for this!
	__ASSERT_DEBUG(EFalse, AVRCP_PANIC(EAvrcpConnectConfirmOnBrowseChannel));
	}

void CAvrcpBulkBearer::DisconnectIndicate(const TBTDevAddr& aBTDevice)
	{
	LOG_FUNC
	ASSERT_BULK_THREAD;
	
	CRcpBrowsingCommandHandler* handler = NULL;
	TInt ix = iBrowseHandlers.Find(aBTDevice, CAvrcpBulkBearer::CompareBrowsingCommandHandlerByBDAddr);
	if(ix >= 0)
		{
		handler = iBrowseHandlers[ix];
		delete handler;
		iBrowseHandlers.Remove(ix);
		}
	else
		{
		ASSERT_DEBUG(EFalse);
		}
	}

void CAvrcpBulkBearer::DisconnectConfirm(const TBTDevAddr& IF_FLOGGING(aBTDevice), TInt IF_FLOGGING(aError))
	{
	LOG_FUNC
	LOGBTDEVADDR(aBTDevice);
	LOG1(_L("\taError = %d"), aError);
	
	// Also outlandish!  Connections on browse channel are all passive.
	__ASSERT_DEBUG(EFalse, AVRCP_PANIC(EAvrcpDisconnectConfirmOnBrowseChannel));
	}

void CAvrcpBulkBearer::MrcciNewCommand(CAvrcpCommand& aCommand)
	{
	LOG_FUNC

	DoNewCommand(aCommand, KNullClientId);
	}

// This overload is used when we want to address a stateless command that may
// be interleaved with commands from other controllers.  The only command
// this is currently used for is the internal UidCounterUpdate command.
void CAvrcpBulkBearer::MrcciNewCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId)
	{
	LOG_FUNC
	// Verify that it's an internal command
	__ASSERT_DEBUG(aCommand.RemoteAddress() == TBTDevAddr(0), AvrcpUtils::Panic(ESpecificAddressUsedForBrowsingCommand));
		
	DoNewCommand(aCommand, aClientId);
	}

void CAvrcpBulkBearer::DoNewCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId)
	{
	LOG_FUNC

	// Need to put the command on the queue straight
	// away in case RemCon collects it synchronously
	iReadyBrowseCommands.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
	
	TInt err = (aClientId == KNullClientId) ? iObserver->NewCommand(remAddr) : iObserver->NewCommand(remAddr, aClientId);
		
	if(err != KErrNone)
		{
		TUid interfaceUid;
		TUint remconId, operationId;
		RBuf8 commandData;
		TBTDevAddr btAddr;
		
		// Calling GetCommandInfo transfers ownership of the command data.  
		aCommand.GetCommandInfo(interfaceUid, remconId, operationId, commandData, btAddr);
		MrcbbiSendReject(interfaceUid, operationId, remconId, remAddr);
		commandData.Close();

		// RemCon is not going to pick this command up
		aCommand.iReadyLink.Deque();
		aCommand.DecrementUsers();
		}
	}

TUint CAvrcpBulkBearer::MrcciNewTransactionId()
	{
	LOG_FUNC
	return iObserver->NewTransactionId();
	}

void CAvrcpBulkBearer::MrcciCommandExpired(TUint aTransactionId)
    {
    LOG_FUNC
    iObserver->CommandExpired(aTransactionId);
    }

TInt CAvrcpBulkBearer::MrcbciSetAddressedClient(const TRemConAddress& aAddr, const TRemConClientId& aClient)
	{
	LOG_FUNC
	return iObserver->SetAddressedClient(aAddr, aClient);
	}

void CAvrcpBulkBearer::MrcbciRemoveAddressing(const TRemConAddress& aAddr)
	{
	LOG_FUNC
	iObserver->RemoveAddressing(aAddr);
	}

TInt CAvrcpBulkBearer::MrcbbiGetCommand(TUid& aInterfaceUid, 
	TUint& aTransactionId, 
	TUint& aOperationId, 
	RBuf8& aData, 
	TRemConAddress& aAddr)
	{
	LOG_FUNC
	TInt result = KErrNotFound;
	
	if(!iReadyBrowseCommands.IsEmpty())
		{
		CAvrcpCommand* command = iReadyBrowseCommands.First();

		// Calling GetCommandInfo transfers the command data to RemCon.  This means
		// once we have called it we are committed to returning KErrNone.
		TBTDevAddr btAddr;
		command->GetCommandInfo(aInterfaceUid, aTransactionId, aOperationId, aData, btAddr);
		AvrcpUtils::BTToRemConAddr(btAddr, aAddr);
		
		// Remove command from queue first because calling
		// DecrementUsers() may delete command
		command->iReadyLink.Deque();
		command->DecrementUsers();
		result = KErrNone;
		}
	else
		{
		__DEBUGGER();
		}
	
	return result;
	}

TInt CAvrcpBulkBearer::MrcbbiSendResponse(TUid aInterfaceUid, 
	TUint /*aOperationId*/, 
	TUint aTransactionId, 
	RBuf8& aData, 
	const TRemConAddress& aAddr)
	{
	LOG_FUNC
	TBTDevAddr btAddr;
	TInt err = KErrNone;
	
	err = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);
	__ASSERT_DEBUG(err == KErrNone, AvrcpUtils::Panic(EInvalidBtAddrInResponse));
	
	if(btAddr != TBTDevAddr(0))
		{
		MIncomingCommandHandler* handler = IncomingHandler(btAddr);
		__ASSERT_ALWAYS(handler, AVRCP_PANIC(EAvrcpNotConnected));

		err = handler->SendRemConResponse(aInterfaceUid, aTransactionId, aData);
		}
	else
		{
		err = iInternalHandler->SendRemConResponse(aInterfaceUid, aTransactionId, aData);
		}
		
	return err;
	}

void CAvrcpBulkBearer::MrcbbiSendReject(TUid aInterfaceUid, 
	TUint /*aOperationId*/, 
	TUint aTransactionId, 
	const TRemConAddress& aAddr)
	{
	LOG_FUNC
	
	TBTDevAddr btAddr;
	TInt err = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);
	__ASSERT_DEBUG(err == KErrNone, AvrcpUtils::Panic(EInvalidBtAddrInResponse));

	if(btAddr != TBTDevAddr(0))
		{
		IncomingHandler(btAddr)->SendReject(aInterfaceUid, aTransactionId);
		}
	else
		{
		iInternalHandler->SendReject(aInterfaceUid, aTransactionId);
		}
	}

TInt CAvrcpBulkBearer::MrcbbiStartBulk(MRemConBearerBulkObserver& aObserver)
	{
	LOG_FUNC
	iObserver = &aObserver;
	TRAPD(err, DoStartBulkL());
	if(err != KErrNone)
		{
		MrcbbiStopBulk();
		}
	return err;
	}

void CAvrcpBulkBearer::DoStartBulkL()
	{
	LOG_FUNC
	LEAVEIFERRORL(Dll::SetTls(reinterpret_cast<TAny*>(EBulkThread)));
	iInternalHandler = iPlayerInfoManager.BulkStartedL(*this);
	iRouter = CBulkRouter::NewL(iAvctp, *this);
	}

void CAvrcpBulkBearer::MrcbbiStopBulk()
	{
	LOG_FUNC
	WEAK_ASSERT_BULK_THREAD;
	
	iPlayerInfoManager.BulkStopped();
	iInternalHandler = NULL;
	
	delete iRouter;
	iRouter = NULL;
	
	iBrowseHandlers.ResetAndDestroy();
	
	iObserver = NULL; // the observer is no longer valid.
	
	Dll::FreeTls();
	}

TBool CAvrcpBulkBearer::Operational() const
	{
	LOG_FUNC
	ASSERT_DEBUG(!iRouter == !iObserver); // internal consistency check
	return !!iRouter;
	}

void CAvrcpBulkBearer::MrcbbiBulkClientAvailable(const TRemConClientId& aId)
	{
	LOG_FUNC
	iPlayerInfoManager.BulkClientAvailable(aId);
	}

void CAvrcpBulkBearer::MrcbbiBulkClientNotAvailable(const TRemConClientId& aId)
	{
	LOG_FUNC
	iPlayerInfoManager.BulkClientNotAvailable(aId);
	}


TBool CAvrcpBulkBearer::CompareBrowsingCommandHandlerByBDAddr(const TBTDevAddr* aKey, const CRcpBrowsingCommandHandler& aHandler)
	{
	LOG_STATIC_FUNC
	return aKey && aHandler.BtAddr() == *aKey;
	}



