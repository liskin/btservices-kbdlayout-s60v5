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



/**
 @file
 @internalComponent
 @released
*/

#include <avctpservices.h>
#include <bttypes.h>
#include <e32base.h>
#include <remconaddress.h>
#include <remconbeareravrcp.h>
#include <remcon/remconbearerobserver.h>
#include "controlcommand.h"
#include "browsecommand.h"
#include "avrcpbrowsingcommandhandler.h"
#include "avrcpincomingcommandhandler.h"
#include "avrcpoutgoingcommandhandler.h"
#include "avrcpremotedevice.h"
#include "avrcprouter.h"
#include "avrcputils.h"
#include "avrcpplayerinfomanager.h"
#include "bulkbearer.h"
#include "controlbearer.h"
#include "mediabrowse.h"
#include "nowplaying.h"

#include "avrcplog.h"



/** Allocates and constructs a new CRemConBearerAvrcp object.

@param aParams Bearer construction params used for base class construction.
@return A new CRemConBearerAvrcp object.
@leave System wide error code
*/
CRemConBearerAvrcp* CRemConBearerAvrcp::NewL(TBearerParams& aParams)
	{
	LOG_STATIC_FUNC
	CRemConBearerAvrcp* bearer = new(ELeave) CRemConBearerAvrcp(aParams);
	CleanupStack::PushL(bearer);
	
	TRAPD(err, bearer->ConstructL());
	
	if(err == KErrNoMemory)
		{
		User::Leave(err);
		}
	// else KErrNone or some other error -- swallow it.
	
	CleanupStack::Pop(bearer);
	return bearer;
	}

/** Constructor.

@param aParams Bearer construction params used for base class construction.
*/
CRemConBearerAvrcp::CRemConBearerAvrcp(TBearerParams& aParams) : 
	CRemConBearerPlugin(aParams), iRemotes(_FOFF(CRcpRemoteDevice, iLink)),
	iReadyCommands(_FOFF(CAvrcpCommand, iReadyLink)), iReadyResponses(_FOFF(CAvrcpCommand, iReadyLink)),
	iReadyNotifyResponses(_FOFF(CControlCommand, iReadyLink)), 
	iReadyNotifyCommands(_FOFF(CAvrcpCommand, iReadyLink))
	{
	LOG_FUNC
	}

/** Second phase construction.

@leave System wide error code
*/	
void CRemConBearerAvrcp::ConstructL()
	{
	LOG_FUNC
	
	// Set-up TLS
	LEAVEIFERRORL(Dll::SetTls(reinterpret_cast<TAny*>(EControlThread)));
	
	// New up a router.  This opens and uses the RAvctp.
	// On initialisation it should begin listening.
	iRouter = CControlRouter::NewL(iAvctp, *this);
	iTimer = CDeltaTimer::NewL(CActive::EPriorityStandard);
	iPlayerInfoManager = CAvrcpPlayerInfoManager::NewL(Observer(), *this);
	iInternalHandler = &iPlayerInfoManager->InternalCommandHandler();
	
	// Create the bulk bearer - actual usage occurs in the bulk thread.
	iBulkBearer = CAvrcpBulkBearer::NewL(iAvctp, *iPlayerInfoManager);
	iConstructionComplete = ETrue;
	}

/** Destructor.
*/
CRemConBearerAvrcp::~CRemConBearerAvrcp()
	{
	LOG_FUNC
	
	while (!iRemotes.IsEmpty())
		{
		CRcpRemoteDevice* remote = iRemotes.First();
		remote->iLink.Deque();
		remote->Disconnect(EFalse);
		delete remote;
		}
	
	delete iRouter;
	
	delete iBulkBearer;
	
	// Delete the timer last as pending events
	// are cancelled in remote->Disconnect
	
	delete iTimer;
	
	delete iPlayerInfoManager;
	
	Dll::FreeTls();
	}

//---------------------------------------------------------------------
// RemConBearer control functions
//---------------------------------------------------------------------	

/** Called by RemCon to retrieve a response on a connection. 

This must only be called as a result of a NewResponse upcall.

@param aInterfaceUid The UID of the outer-layer client API specifying the response.
@param aId The command identifier used as a cookie for command/response matching.
@param aOperationId The ID of the response operation in the outer-layer client API.
@param aData API-specific message data. On success, ownership is returned.
@param aAddr The connection.
*/	
TInt CRemConBearerAvrcp::GetResponse(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr)
	{
	LOG_FUNC
	
	// If object only partially constructed, swallow the request
	__ASSERT_DEBUG(iConstructionComplete, AVRCP_PANIC(EAvrcpNoResponsesAvailable));	
	
	
	TInt result = KErrNotFound;
	
	if(!iReadyResponses.IsEmpty())
		{
		CAvrcpCommand* command = iReadyResponses.First();
		TBTDevAddr btAddr;
		command->GetCommandInfo(aInterfaceUid, aId, aOperationId, aCommandData, btAddr);
		AvrcpUtils::BTToRemConAddr(btAddr, aAddr);
		
		// Remove command from queue first because calling
		// DecrementUsers() may delete command
		command->iReadyLink.Deque();
		command->DecrementUsers();
		result = KErrNone;
		}
	else
		{
		// RemCon is trying to pick up a response we don't think we have
		__ASSERT_DEBUG(EFalse, AVRCP_PANIC(EAvrcpNoResponsesAvailable));	
		}
		
	return result;
	}

/** Called by RemCon to retrieve a response on a connection. 

This must only be called as a result of a NewResponse upcall.

@param aInterfaceUid The UID of the outer-layer client API specifying the response.
@param aId The command identifier used as a cookie for command/response matching.
@param aOperationId The ID of the response operation in the outer-layer client API.
@param aData API-specific message data. On success, ownership is returned.
@param aAddr The connection.
*/	
TInt CRemConBearerAvrcp::GetNotifyResponse(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr,
		TRemConMessageSubType& aSubMessageType)
	{
	LOG_FUNC
	
	// If object only partially constructed, swallow the request
	__ASSERT_DEBUG(iConstructionComplete, AVRCP_PANIC(EAvrcpNoResponsesAvailable));	
	
	
	TInt result = KErrNotFound;
	
	if(!iReadyNotifyResponses.IsEmpty())
		{
		CControlCommand* command = iReadyNotifyResponses.First();
		TBTDevAddr btAddr;
		command->GetCommandInfo(aInterfaceUid, aId, aOperationId, aCommandData, btAddr);
		AvrcpUtils::BTToRemConAddr(btAddr, aAddr);
	
		switch ( command->Frame().Type() )
	        {
	        case AVC::EInterim:
			    aSubMessageType = ERemConNotifyResponseInterim;
	            break;
		    case AVC::EChanged: 
	            aSubMessageType = ERemConNotifyResponseChanged;
		        break;
		    case AVC::ERejected: // fall into default
		    case AVC::ENotImplemented:// fall into default
		    default:
		        aSubMessageType = ERemConMessageDefault;
		        break;
		    }

		// Remove command from queue first because calling
		// DecrementUsers() may delete command
		command->iReadyLink.Deque();
		command->DecrementUsers();
		result = KErrNone;
		}
	else
		{
		// RemCon is trying to pick up a response we don't think we have
		__ASSERT_DEBUG(EFalse, AVRCP_PANIC(EAvrcpNoResponsesAvailable));	
		}
		
	return result;
	}
	
/** Called by RemCon to send a command on a connection. 

@param aInterfaceUid The UID of the outer-layer client API specifying the command.
@param aOperationId The ID of the command operation in the outer-layer client API.
@param aId The command identifier used as a cookie for command/response matching.
@param aData API-specific message data. On success, ownership is passed. 
@param aAddr The connection.
@return Error. This request is synchronous. It returns KErrNone when AVRCP has
taken responsibility for sending the message. This involves checking that the message
is well-formed and adding it to the send queue.
*/
TInt CRemConBearerAvrcp::SendCommand(TUid aInterfaceUid, 
		TUint aOperationId, 
		TUint aId,  
		RBuf8& aData, 
		const TRemConAddress& aAddr)
	{
	LOG_FUNC
	// RemCon retains ownership of the data in aData until
	// this function returns KErrNone.
	
	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return KErrNotSupported;
		}
	
	
	TBTDevAddr btAddr;
	TInt err = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);

	if(err == KErrNone)
		{
		CRcpRemoteDevice* remote = RemoteDevice(btAddr);
		__ASSERT_ALWAYS(remote, AVRCP_PANIC(EAvrcpNotConnected));
		
		TRAP(err, remote->OutgoingHandler().SendCommandL(aInterfaceUid, 
				aOperationId,
				aId, 
				aData, 
				btAddr));
		}

	return err;
	}


/** Called by RemCon to retrieve a command on a connection. 

This must only be called as a result of a NewCommand upcall.

@param aInterfaceUid The UID of the outer-layer client API specifying the command.
@param aId The command identifier used as a cookie for command/response matching.
@param aOperationId The ID of the command operation in the outer-layer client API.
@param aData API-specific message data. On success, ownership is returned.
@param aAddr The connection.
*/	
TInt CRemConBearerAvrcp::GetCommand(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr)
	{
	TInt result = KErrNotFound;
	
	if(!iReadyCommands.IsEmpty())
		{
		CAvrcpCommand* command = GetFirstCommand(iReadyCommands, aInterfaceUid, aId, aOperationId, aCommandData, aAddr);
		if(command)
			{
			// Remove command from queue first because calling
			// DecrementUsers() may delete command
			command->iReadyLink.Deque();
			command->DecrementUsers();
			result = KErrNone;
			}
		}
	else
		{
		__DEBUGGER();
		}
	
	return result;
	}

/** Called by RemCon to retrieve a notify command on a connection. 

This must only be called as a result of a NewNotifyCommand upcall.

@param aInterfaceUid The UID of the outer-layer client API specifying the command.
@param aId The command identifier used as a cookie for command/response matching.
@param aOperationId The ID of the command operation in the outer-layer client API.
@param aData API-specific message data. On success, ownership is returned.
@param aAddr The connection.
*/	
TInt CRemConBearerAvrcp::GetNotifyCommand(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr)
	{
	TInt result = KErrNotFound;
	
	CAvrcpCommand* command = GetFirstCommand(iReadyNotifyCommands, aInterfaceUid, aId, aOperationId, aCommandData, aAddr);
	if(command)
		{		
		// Remove command from queue first because calling
		// DecrementUsers() may delete command
		command->iReadyLink.Deque();
		command->DecrementUsers();
		result = KErrNone;
		}
	
	return result;
	}

/**
Internal method, called by GetCommand() and GetNotifyCommand()

@return The first command from the Queue.  NULL if the queue is empty.  The command remains 
		on the queue.
*/
CAvrcpCommand* CRemConBearerAvrcp::GetFirstCommand(TDblQue<CAvrcpCommand>& aQue, TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TRemConAddress& aAddr)
	{
	LOG_FUNC

	// If object only partially constructed, swallow the request
	__ASSERT_DEBUG(iConstructionComplete, AVRCP_PANIC(EAvrcpNoResponsesAvailable));	
	
	CAvrcpCommand* command = NULL;
	
	if(! aQue.IsEmpty())
		{
		command = aQue.First();

		// Calling GetCommandInfo transfers the command data to RemCon.  This means
		// once we have called it we are committed to returning KErrNone.
		TBTDevAddr btAddr;
		command->GetCommandInfo(aInterfaceUid, aId, aOperationId, aCommandData, btAddr);
		AvrcpUtils::BTToRemConAddr(btAddr, aAddr);
		}
	else
		{
		// RemCon is trying to pick up a command we don't think we have
		__ASSERT_DEBUG(EFalse, AVRCP_PANIC(EAvrcpNoResponsesAvailable));
		}
		
	return command;
	}
/** Called by RemCon to send a response on a connection. 

@param aInterfaceUid The UID of the outer-layer client API specifying the response.
@param aOperationId The ID of the response operation in the outer-layer client API.
@param aId The command identifier used as a cookie for command/response matching
@param aData API-specific message data. On success, ownership is passed.
@param aAddr The connection.
@return Error. This request is synchronous. It is completed by AVRCP when it has 
taken responsibility for sending the message. This involves checking that the message 
is well-formed and adding it to the send queue.
*/	
TInt CRemConBearerAvrcp::SendResponse(TUid aInterfaceUid, 
		TUint /*aOperationId*/, 
		TUint aId, 
		RBuf8& aData, 
		const TRemConAddress& aAddr)
	{
	LOG_FUNC
	// RemCon retains ownership of the data in aData until
	// this function returns KErrNone.
	
	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return KErrNotSupported;
		}
	
	
	TBTDevAddr btAddr;
	TInt err = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);
	__ASSERT_DEBUG(err == KErrNone, AVRCP_PANIC(EInvalidBtAddrInResponse));
	
	if(btAddr != TBTDevAddr(0))
		{
		CRcpRemoteDevice* remote = RemoteDevice(btAddr);
		__ASSERT_ALWAYS(remote, AVRCP_PANIC(EAvrcpNotConnected));

		err = remote->IncomingHandler().SendRemConResponse(aInterfaceUid, aId, aData);
		}
	else
		{
		err = iInternalHandler->SendRemConResponse(aInterfaceUid, aId, aData);
		}
		
	return err;
	}

/**	Called by RemCon when either
	a) The TSP does not address a command to any clients
	b) The TSP does not permit the response from any commands
	c) All the clients disconnect from remcon before the response is sent
	to send a null response on a connection. The connection must already
	exist
	@param aInterfaceUid The UID of the outer-layer client API that the command
	was sent to
	@param aOperationId The ID of the command operation sent to remcon
	@param aId The command identifier used as a cookie for command/response 
	matching.
	@param aAddr The connection.
	*/

void CRemConBearerAvrcp::SendReject(TUid aInterfaceUid,
		TUint /* aOperationId */,
		TUint aTransactionId,
		const TRemConAddress& aAddr)
	{
	LOG_FUNC
	
	__ASSERT_DEBUG(iConstructionComplete, AVRCP_PANIC(EAvrcpNotFullyConstructed));
	
	TBTDevAddr btAddr;
	__DEBUG_ONLY(TInt err = )AvrcpUtils::RemConToBTAddr(aAddr, btAddr);
	__ASSERT_DEBUG(err == KErrNone, AVRCP_PANIC(EInvalidBtAddrInResponse));
	
	if(btAddr != TBTDevAddr(0))
		{
		CRcpRemoteDevice* remote = RemoteDevice(btAddr);
		__ASSERT_ALWAYS(remote, AVRCP_PANIC(EAvrcpNotConnected));

		remote->IncomingHandler().SendReject(aInterfaceUid, aTransactionId);
		}
	else
		{
		iInternalHandler->SendReject(aInterfaceUid, aTransactionId);
		}

	}

/** Called by RemCon to establish a bearer-level connection to another party.
 
Completion is signalled back in ConnectConfirm.

@param aAddr The RemCon address to connect to.
*/	
void CRemConBearerAvrcp::ConnectRequest(const TRemConAddress& aAddr)
	{
	LOG_FUNC

	if (!iConstructionComplete)
		{
		// Object only partially constructed, error the request
		Observer().ConnectConfirm(aAddr, KErrNotSupported);
		return;
		}
	
	
	// Get a bluetooth address from the TRemConAddr
	TBTDevAddr btAddr;
	TInt convErr = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);
	if(convErr != KErrNone)
		{
		Observer().ConnectConfirm(aAddr, convErr);
		return;
		}

	// Address for internal commands
	if(btAddr == TBTDevAddr(0))
		{
		Observer().ConnectConfirm(aAddr, KErrNone);
		return;
		}
	
	CRcpRemoteDevice* remote = RemoteDevice(btAddr);
	if(remote)
		{
		// Already have a connection to this address
		Observer().ConnectConfirm(aAddr, KErrNone);
		return;
		}
	
	TRAPD(devErr, remote = CRcpRemoteDevice::NewL(btAddr, *iRouter, *this, Observer(), *iTimer, *iPlayerInfoManager));
	if(devErr)
		{
		Observer().ConnectConfirm(aAddr, devErr);
		return;
		}

	iRemotes.AddLast(*remote);
	devErr = iRouter->ConnectRequest(btAddr);
	if(devErr != KErrNone)
		{
		remote->iLink.Deque();
		delete remote;

		// Error with connect, generate errored ConnectConfirm.
		Observer().ConnectConfirm(aAddr, devErr);
		return;
		}
	
	return;
	}

/** Called by RemCon to destroy a bearer-level connection to another party. 

Completion is signalled back in DisconnectConfirm.

@param aAddr The RemCon address to disconnect from.
*/	
void CRemConBearerAvrcp::DisconnectRequest(const TRemConAddress& aAddr)
	{
	LOG_FUNC

	__ASSERT_DEBUG(iConstructionComplete, AVRCP_PANIC(EAvrcpDisconnectRequestWhilePartiallyConstructed));
	
	TBTDevAddr btAddr;
	TInt err = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);
	
	if(!err)
		{
		// Address for internal commands
		if(btAddr == TBTDevAddr(0))
			{
			Observer().DisconnectConfirm(aAddr, KErrNone);
			}
		else
			{
			CRcpRemoteDevice *remote = RemoteDevice(btAddr);
			if(remote)
				{
				// calling disconnect gives the remote the opportunity
				// to do anything necessary to commands still on its 
				// queue before we delete it.
				remote->Disconnect(ETrue);
				remote->iLink.Deque();
				delete remote;
				
				err = iRouter->DisconnectRequest(btAddr);
				// If this failed generate an errored DisconnectConfirm now,
				// otherwise we'll be prodded with a DisconnectConfirm from
				// the router at a later date.
				if(err != KErrNone)
					{
					Observer().DisconnectConfirm(aAddr, err);
					}
				}
			else
				{
				__ASSERT_DEBUG(EFalse, AVRCP_PANIC(EAvrcpMismatchedConnectDisconnect));
				Observer().DisconnectConfirm(aAddr, KErrNotFound);
				}
			}
		}
	else
		{
		Observer().DisconnectConfirm(aAddr, err);
		}
	}

/** Called by RemCon to get the security policy.

This is the list of capabilities required to make/destroy connections 
over the bearer, and to send and receive messages over the bearer.

@return The bearer's security policy.
*/
TSecurityPolicy CRemConBearerAvrcp::SecurityPolicy() const
	{
	LOG_FUNC
	return TSecurityPolicy(ECapabilityLocalServices);
	}

/** Called by RemCon when either (a) the number of controller clients changes 
from 0 to 1 or from 1 to 0, or (b) the number of target clients changes 
from 0 to 1 or from 1 to 0.

@param aControllerPresent Whether there are any controller clients connected.
@param aTargetPresent Whether there are any target clients connected.
*/	
void CRemConBearerAvrcp::ClientStatus(TBool aControllerPresent, TBool aTargetPresent)
	{
	LOG_FUNC

	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return;
		}
	
	iPlayerInfoManager->ClientStatus(aControllerPresent, aTargetPresent);
	}

/**
Called by RemCon when a client has become available for addressing.  Once this
call has been made the bearer may use the provided TRemConClientId to address
incoming commands and notifys to this client until RemCon calls ClientNotAvailable
with this TRemConClientId.

@param aId A unique identifier for this client, that can be used when addressing 
incoming commands.
@param aClientType The basic type of this client
@param aClientSubType More detailed type information on this client
@param aName The name of this client in UTF-8.
*/
void CRemConBearerAvrcp::ClientAvailable(TRemConClientId& aId, TPlayerType aClientType, TPlayerSubType aClientSubType, const TDesC8& aName)
	{
	LOG_FUNC

	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return;
		}
	
	iPlayerInfoManager->ClientAvailable(aId, aClientType, aClientSubType, aName);
	}

/**
Called by RemCon when a client is no longer available for addressing.  Once this
call has been made the bearer shall not use this client id when addressing incoming
commands and notifys until informed that the client is available again via
ClientAvailable.

@param aId The client that has ceased to be available.
*/
void CRemConBearerAvrcp::ClientNotAvailable(TRemConClientId& aId)
	{
	LOG_FUNC

	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return;
		}
	
	iPlayerInfoManager->ClientNotAvailable(aId);
	}

TInt CRemConBearerAvrcp::SetLocalAddressedClient(TRemConClientId& aId)
	{
	LOG_FUNC

	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return KErrNotSupported;
		}
	
	return iPlayerInfoManager->SetLocalAddressedClient(aId);
	}

void CRemConBearerAvrcp::ControllerFeaturesUpdated(RArray<TUid>& aSupportedInterfaces)
	{
	LOG_FUNC
	
	if(!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return;
		}
	
	// This is a best effort attempt at keeping the sdp record accurate.  If we
	// failed to update it then just live with it.
	TRAP_IGNORE(iPlayerInfoManager->ControllerFeaturesUpdatedL(aSupportedInterfaces));
	}

//---------------------------------------------------------------------
// Data notifications from the command handlers
//---------------------------------------------------------------------

/** Called from incoming handlers to notify that a command
is ready for Remcon.

@param aCommand The command that is ready.
*/
void CRemConBearerAvrcp::MrcciNewCommand(CAvrcpCommand& aCommand)
	{
	LOG_FUNC
	
	// Need to put the command on the queue straight
	// away in case RemCon collects it synchronously
	iReadyCommands.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
	
	// Unaddressed variant
	TInt err =  Observer().NewCommand(remAddr);
		
	if(err != KErrNone)
		{
		HandleUndeliveredCommand(aCommand, remAddr);
		}
	}

/** Called from incoming handlers to notify that a command
is ready for Remcon.

@param aCommand The command that is ready.
*/
void CRemConBearerAvrcp::MrcciNewCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId)
	{
	LOG_FUNC
	
	// Need to put the command on the queue straight
	// away in case RemCon collects it synchronously
	iReadyCommands.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
	
	// if this is the null client id then RemCon will address it
	TInt err =  Observer().NewCommand(remAddr, aClientId);
		
	if(err != KErrNone)
		{
		HandleUndeliveredCommand(aCommand, remAddr);
		}
	}

/** Called from incoming handlers to notify that a notify command
is ready for Remcon.

@param aCommand The command that is ready.
*/
void CRemConBearerAvrcp::MrccciNewNotifyCommand(CAvrcpCommand& aCommand)
	{
	LOG_FUNC
	
	// Need to put the command on the queue straight
	// away in case RemCon collects it synchronously
	iReadyNotifyCommands.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
		
	// Will be addressed by RemCon
	TInt err = Observer().NewNotifyCommand(remAddr);
		
	if(err != KErrNone)
		{
		HandleUndeliveredCommand(aCommand, remAddr);
		}
	}

/** Called from incoming handlers to notify that a notify command
is ready for Remcon.

@param aCommand The command that is ready.
*/
void CRemConBearerAvrcp::MrccciNewNotifyCommand(CAvrcpCommand& aCommand, const TRemConClientId& aClientId)
	{
	LOG_FUNC
	
	// Need to put the command on the queue straight
	// away in case RemCon collects it synchronously
	iReadyNotifyCommands.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
		
	// if this is the null client id then RemCon will address it
	TInt err = Observer().NewNotifyCommand(remAddr, aClientId);
		
	if(err != KErrNone)
		{
		HandleUndeliveredCommand(aCommand, remAddr);
		}
	}


/** Called from outgoing handlers to notify that a response
is ready for RemCon.

@param aCommand The response that is ready.
*/
void CRemConBearerAvrcp::MrccciNewResponse(CAvrcpCommand& aCommand)
	{
	LOG_FUNC
	
	// Need to put the response on the queue straight
	// away in case RemCon collects it synchronously
	iReadyResponses.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
	TInt err = Observer().NewResponse(remAddr);
	
	if(err != KErrNone)
		{
		// RemCon is not going to pick this response up
		aCommand.iReadyLink.Deque();
		aCommand.DecrementUsers();
		}
	}

/** Get a new transaction id for an incoming command.

@return The new command id.
*/
TUint CRemConBearerAvrcp::MrcciNewTransactionId()
	{
	LOG_FUNC
	return Observer().NewTransactionId();
	}

void CRemConBearerAvrcp::MrcciCommandExpired(TUint aTransactionId)
    {
    LOG_FUNC
    Observer().CommandExpired(aTransactionId);
    }

void CRemConBearerAvrcp::MrccciSetAddressedClient(const TRemConClientId& aClientId)
	{
	LOG_FUNC
	Observer().SetRemoteAddressedClient(TUid::Uid(KRemConBearerAvrcpImplementationUid), aClientId);
	}

void CRemConBearerAvrcp::MrccciRegisterForLocalAddressedClientUpdates()
	{
	// Addressed player observers are registered with RemCon on a per-bearer basis
	// so we aggregate interest from remote here
	iRemotesInterestedInLocalAddressedClient++;
	if(iRemotesInterestedInLocalAddressedClient == 1)
		{
		// Weren't any registered before, tell RemCon we care
		Observer().RegisterLocalAddressedClientObserver(TUid::Uid(KRemConBearerAvrcpImplementationUid));
		}
	
#ifdef __DEBUG
	TInt numRemotes = 0;
	CRcpRemoteDevice* remote = NULL;
	
	TDblQueIter<CRcpRemoteDevice> iter(iRemotes);
	while (iter++)
		{
		numRemotes++;
		}
	__ASSERT_DEBUG(iRemotesInterestedInLocalAddressedClient <= numRemotes, AVRCP_PANIC(ETooManyRemotesRegisterForLocalAddressedPlayerUpdates));
#endif
	}

void CRemConBearerAvrcp::MrccciUnregisterForLocalAddressedClientUpdates()
	{
#ifdef __DEBUG
	TInt numRemotes = 0;
	CRcpRemoteDevice* remote = NULL;
	
	TDblQueIter<CRcpRemoteDevice> iter(iRemotes);
	while (iter++)
		{
		numRemotes++;
		}
	__ASSERT_DEBUG(iRemotesInterestedInLocalAddressedClient <= numRemotes, AVRCP_PANIC(ETooManyRemotesRegisterForLocalAddressedPlayerUpdates));
	__ASSERT_DEBUG(iRemotesInterestedInLocalAddressedClient > 0, AVRCP_PANIC(ETooFewRemotesRegisterForLocalAddressedPlayerUpdates));
#endif

	iRemotesInterestedInLocalAddressedClient--;
	if(iRemotesInterestedInLocalAddressedClient == 0)
		{
		// No-one left who cares.  Tell RemCon not to bother updating
		// us anymore.
		Observer().UnregisterLocalAddressedClientObserver(TUid::Uid(KRemConBearerAvrcpImplementationUid));
		}
	}

/** Called from outgoing handlers to notify that a response
for a notify command is ready for RemCon. 

@param aCommand The response that is ready.
*/
void CRemConBearerAvrcp::MrccciNewNotifyResponse(CControlCommand& aCommand)
	{
	LOG_FUNC
	
	// Need to put the response on the queue straight
	// away in case RemCon collects it synchronously
	iReadyNotifyResponses.AddLast(aCommand);
	aCommand.IncrementUsers();
	
	TRemConAddress remAddr;
	AvrcpUtils::BTToRemConAddr(aCommand.RemoteAddress(), remAddr);
	TInt err = Observer().NewNotifyResponse(remAddr);
	
	if(err != KErrNone)
		{
		// RemCon is not going to pick this response up
		aCommand.iReadyLink.Deque();
		aCommand.DecrementUsers();
		}
	}
	
/** Called by RemCon to send a notify command on a connection. 

@param aInterfaceUid The UID of the outer-layer client API specifying the command.
@param aOperationId The ID of the command operation in the outer-layer client API.
@param aId The command identifier used as a cookie for command/response matching, the transaction ID.
@param aData API-specific message data. On success, ownership is passed. 
@param aAddr The connection.
@return Error. This request is synchronous. It returns KErrNone when AVRCP has
taken responsibility for sending the message. This involves checking that the message
is well-formed and adding it to the send queue.
*/
TInt CRemConBearerAvrcp::SendNotifyCommand(TUid aInterfaceUid, 
		TUint aOperationId, 
		TUint aId,  
		RBuf8& aData, 
		const TRemConAddress& aAddr)
	{
	LOG_FUNC
	// RemCon retains ownership of the data in aData until
	// this function returns KErrNone.
	
	if (!iConstructionComplete)
		{
		// Object only partially constructed, swallow the request
		return KErrNotSupported;
		}
	
	
	TBTDevAddr btAddr;
	TInt err = AvrcpUtils::RemConToBTAddr(aAddr, btAddr);

	if(err == KErrNone)
		{
		CRcpRemoteDevice* remote = RemoteDevice(btAddr);
		__ASSERT_ALWAYS(remote, AVRCP_PANIC(EAvrcpNotConnected));
		
		TRAP(err, remote->OutgoingHandler().SendNotifyCommandL(aInterfaceUid, 
				aOperationId,
				aId, 
				aData, 
				btAddr));
		}

	return err;
	}

//---------------------------------------------------------------------
// Control notifications from the router
//---------------------------------------------------------------------

/** Called when a connection comes in from a remote.

@param aBTDevice The address of the device initiating the connection.
*/
void CRemConBearerAvrcp::ConnectIndicate(const TBTDevAddr& aBTDevice)
	{
	LOG_FUNC
	CRcpRemoteDevice *remote = NULL;
	// We new up the device here even though we may end up deleting it
	// later in the function because we need to know if we've successfully
	// got a remote before we can tell RemCon.  RemCon may optionally
	// drop the connection in which case we delete remote straight away.
	TRAPD(devErr, remote = CRcpRemoteDevice::NewL(aBTDevice, *iRouter, *this, Observer(), *iTimer, *iPlayerInfoManager));
	if(!devErr)
		{
		iRemotes.AddLast(*remote);
		
		TRemConAddress remoteAddr;
		AvrcpUtils::BTToRemConAddr(aBTDevice, remoteAddr);

		TInt err = Observer().ConnectIndicate(remoteAddr);
		if(err)
			{
			// We need to drop this connection.  Disconnect and delete
			// the remote NOW.  When we get the disconnect confirm we
			// will then know that we shouldn't tell RemCon.
			remote->Disconnect(ETrue);
			remote->iLink.Deque();
			delete remote;
			
			err = iRouter->DisconnectRequest(aBTDevice);
			}
		}
	}

/** Called to confirm an outgoing connection.

@param aBTDevice The device the outgoing connection is to.
@param aError The result of the connection attempt.
*/	
void CRemConBearerAvrcp::ConnectConfirm(const TBTDevAddr& aBTDevice, TInt aError)
	{
	LOG_FUNC
	TRemConAddress remoteAddr;
	AvrcpUtils::BTToRemConAddr(aBTDevice, remoteAddr);
	
	if(aError != KErrNone)
		{
		CRcpRemoteDevice *remote = RemoteDevice(aBTDevice);
		if(remote)
			{
			// calling disconnect gives the remote the opportunity
			// to do anything necessary to commands still on its 
			// queue before we delete it.
			remote->Disconnect(ETrue);
			remote->iLink.Deque();
			delete remote;
			}
		}

	Observer().ConnectConfirm(remoteAddr, aError);
	}

/** Called when a remote disconnects.

@param aBTDevice The address of the remote that has disconnected.
*/	
void CRemConBearerAvrcp::DisconnectIndicate(const TBTDevAddr& aBTDevice)
	{
	LOG_FUNC
	TRemConAddress remoteAddr;
	AvrcpUtils::BTToRemConAddr(aBTDevice, remoteAddr);
	
	CRcpRemoteDevice *remote = RemoteDevice(aBTDevice);
	if(remote)
		{
		// calling disconnect gives the remote the opportunity
		// to do anything necessary to commands still on its 
		// queue before we delete it.
		remote->Disconnect(ETrue);
		remote->iLink.Deque();
		delete remote;
		}

	Observer().DisconnectIndicate(remoteAddr);
	}

/** Called to confirm a locally initiated disconnect.

@param aBTDevice The address of the disconnected remote.
@param aError The result of the disconnect attempt.
*/	
void CRemConBearerAvrcp::DisconnectConfirm(const TBTDevAddr& aBTDevice, TInt aError)
	{
	LOG_FUNC
	TRemConAddress remoteAddr;
	AvrcpUtils::BTToRemConAddr(aBTDevice, remoteAddr);

	Observer().DisconnectConfirm(remoteAddr, aError);
	}

/** Called to get a bearer interface.

@param aUid The uid of the desired interface.
@return An instance of the desired interface, NULL if
		one could not be found.
*/	
TAny* CRemConBearerAvrcp::GetInterface(TUid aUid)
	{
	LOG_FUNC
	
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConBearerInterface1) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConBearerInterface*>(this)
			);
		}
	else if ( aUid == TUid::Uid(KRemConBearerInterface2) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConBearerInterfaceV2*>(this)
			);
		}
	else if ( aUid == TUid::Uid(KRemConBearerInterface3) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConBearerInterfaceV3*>(this)
			);
		}
	else if ( aUid == TUid::Uid(KRemConBearerBulkInterface1) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConBearerBulkInterface*>(iBulkBearer)
			);
		}

	return ret;
	}

//---------------------------------------------------------------------
// Utility functions
//---------------------------------------------------------------------

MIncomingCommandHandler* CRemConBearerAvrcp::IncomingHandler(const TBTDevAddr& aAddr)
	{
	CRcpRemoteDevice* remote = RemoteDevice(aAddr);
	if(remote)
		{
		return &remote->IncomingHandler();
		}
	else
		{
		return NULL;
		}
	}

MOutgoingCommandHandler* CRemConBearerAvrcp::OutgoingHandler(const TBTDevAddr& aAddr)
	{
	CRcpRemoteDevice* remote = RemoteDevice(aAddr);
	if(remote)
		{
		return &remote->OutgoingHandler();
		}
	else
		{
		return NULL;
		}
	}

/** Utility function to get the CRcpRemoteDevice that
handles a given BT address.

@param aAddr The address to get the CRcpRemoteDevice for.
@return A pointer to a remote device, or NULL if not found.
*/
CRcpRemoteDevice* CRemConBearerAvrcp::RemoteDevice(const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	
	CRcpRemoteDevice* remote = NULL;
	
	TDblQueIter<CRcpRemoteDevice> iter(iRemotes);
	while (iter)
		{
		remote = iter++;
		if(remote->RemoteAddress() == aAddr)
			{
			break;
			}
		remote = NULL;
		}
	
	return remote;
	}

void CRemConBearerAvrcp::HandleUndeliveredCommand(CAvrcpCommand& aCommand, const TRemConAddress& aAddr)
	{
	TUid interfaceUid;
	TUint remconId, operationId;
	RBuf8 commandData;
	TBTDevAddr btAddr;
	
	// Calling GetCommandInfo transfers the command data to us. 
	aCommand.GetCommandInfo(interfaceUid, remconId, operationId, commandData, btAddr);
	SendReject(interfaceUid, operationId, remconId, aAddr);
	commandData.Close();

	// RemCon is not going to pick this command up
	aCommand.iReadyLink.Deque();
	aCommand.DecrementUsers();
	}
