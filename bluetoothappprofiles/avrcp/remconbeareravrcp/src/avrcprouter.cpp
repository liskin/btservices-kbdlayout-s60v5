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

#include <bluetoothav.h>

#include "avrcpcommand.h"
#include "avrcplog.h"
#include "avrcprouter.h"
#include "avrcputils.h"
#include "commandhandlerinterface.h"

using namespace SymbianAvctp;

/** Constructor.

@param aRemotes The list of remote devices known to AVRCP.
@param aBearer The bearer.
@return A partially constructed CRcpRouter.
*/
CRcpRouter::CRcpRouter(MAvrcpBearer& aBearer)
	: iChannel(NULL)
	, iBearer(aBearer)
	, iSendQueue(_FOFF(CAvrcpCommand, iSendLink))
	, iState(EAvrcpRouterCanSend)
	{
	LOG_FUNC
	}

/** Destructor.
*/	
CRcpRouter::~CRcpRouter()
	{
	LOG_FUNC
	// The lifetime of the Router is the same as that of the bearer plugin.
	// This means that there is no point worrying about the stuff on our
	// send queue as we won't be around to pass the results up to RemCon.
	while (!iSendQueue.IsEmpty())
		{
		CAvrcpCommand *command = iSendQueue.First();
		command->iSendLink.Deque();
		command->DecrementUsers();
		}
	}
	
//------------------------------------------------------------------
// Data functions called from command handlers
//------------------------------------------------------------------	

/** Adds command to send queue.

This message will be sent as soon as the iAvcp becomes
free for sending, unless the command is removed before
then.

@param aCommand The command to send.
@see CRcpRouter::RemoveFromSendQueue
*/
void CRcpRouter::AddToSendQueue(CAvrcpCommand& aCommand)
	{
	LOG_FUNC
	
	__ASSERT_ALWAYS(!aCommand.iSendLink.IsQueued(), AVRCP_PANIC(ECommandAlreadyQueuedForSending));

	iSendQueue.AddLast(aCommand);
	aCommand.IncrementUsers();

	if(iState == EAvrcpRouterCanSend)
		{
		Send();
		}	
	}

/** Remove command from send queue.

@param aCommand The command that is not to be sent.
*/	
void CRcpRouter::RemoveFromSendQueue(CAvrcpCommand& aCommand)
	{
	LOG_FUNC
	
	if(iSendQueue.IsFirst(&aCommand))
		{
		// If this is at the front of the queue it's currently being
		// sent.  Cancel that and start off the next thing.
		iChannel->MacCancelSend();
		aCommand.iSendLink.Deque();
		iState = EAvrcpRouterCanSend;
		if(!iSendQueue.IsEmpty())
			{
			Send();
			}
		}
	else
		{
		// Still waiting to be sent so we can just sneak it out of
		// the queue.
		aCommand.iSendLink.Deque();
		}
	}
	
//------------------------------------------------------------------
// Control functions called from bearer
//------------------------------------------------------------------

/** Try to bring up an explicit connection to a remote.

@param aAddr The address of the remote.
@return System wide error.  KErrNone if this request will be
		attempted, and generate a ConnectConfirm.
*/
TInt CRcpRouter::ConnectRequest(const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	return iChannel->MacAttachRequest(aAddr);
	}

/** Try to bring down an explicit connection to a remote.

@param aAddr The address of the remote.
@return System wide error.  KErrNone if the disconnect will be
		attempted, and generate a DisconnectConfirm.
*/	
TInt CRcpRouter::DisconnectRequest(const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	return iChannel->MacDetachRequest(aAddr);
	}
	
//------------------------------------------------------------------
// MAvctpEventNotify functions called from RAvctp
//------------------------------------------------------------------	

/** AVCTP Connection Indicate.

This is called when a remote device has connected 
to us. NB we don't return the configuration status as 
in Appendix A - AVCTP Upper Interface of [R2]. If aAccept 
is not changed the connection will be refused however 
this may not result in the actual disconnection of the 
device if another RAvctp client did accept the connection.

@param aBTDevice the address of the device connected to
@param aAccept this parameter is provided so that the client 
	of RAvctp can indicate whether they want to accept the 
	connection.  The meaning of a refusal is that you don't 
	care whether or not the connection is there or not.	
@see RAvctp			   
*/
void CRcpRouter::MaenAttachIndicate(const TBTDevAddr& aBTDevice, TInt aMtu, TBool& aAccept)
	{
	LOG_FUNC
	aAccept = ETrue;
	
	// if we have already received data from this device we will have 
	// informed RemCon about it already, so don't pass up the connection
	// now
	if(!iBearer.IncomingHandler(aBTDevice))
		{
		iBearer.ConnectIndicate(aBTDevice);
		}
	
	MIncomingCommandHandler* handler = iBearer.IncomingHandler(aBTDevice);
	if(handler)
		{
		handler->MaxPacketSize(aMtu);
		aAccept = ETrue;
		}
	else
		{
		aAccept = EFalse;
		}
	}

/** AVCTP Connection Confirm.

This is a response to RAvctp::ConnectRequest and passes on the 
result of the Connection attempt. NB we don't return the 
configuration status as in Appendix A - AVCTP Upper 
Interface of [R2]. If the aConnectResult is KErrNone 
then iAvctp is now connected.

@param aBTDevice the address of the device connected to
@param aConnectResult connection result - one of the 
					  system-wide error codes. 
@see RAvctp					  
*/	
void CRcpRouter::MaenAttachConfirm(const TBTDevAddr& aBTDevice, TInt aMtu, TInt aConnectResult)
	{
	LOG_FUNC
	iBearer.ConnectConfirm(aBTDevice, aConnectResult);

	MIncomingCommandHandler* handler = iBearer.IncomingHandler(aBTDevice);
	if(handler)
		{
		handler->MaxPacketSize(aMtu);
		}
	}

/** AVCTP Disconnection Indication.

Indicates that a remote device has disconnected from us.
It is only called if the device had been explicitly Connected to.
 
@param aBTDevice the address of the disconnecting device
@see RAvctp
*/	
void CRcpRouter::MaenDetachIndicate(const TBTDevAddr& aBTDevice)
	{
	LOG_FUNC
	iBearer.DisconnectIndicate(aBTDevice);
	}

/** AVCTP Disconnection Confirm.

@param aBTDevice the address of the disconnected device
@param aDisconnectResult will be one of the system-wide 
						 error codes. If KErrTimedOut is 
						 returned then the RAvctp will be 
						 disconnected.
@see RAvctp	
*/	
void CRcpRouter::MaenDetachConfirm(const TBTDevAddr& aBTDevice, TInt aDisconnectResult)
	{
	LOG_FUNC
	iBearer.DisconnectConfirm(aBTDevice, aDisconnectResult);
	}

/** AVCTP Message received indication.

This method is called when a message has been received from 
the given device on the RAvctp's PID. 

Note that because AVCTP is a connectionless protocol, it is 
perfectly possible to get a MaenMessageReceivedIndicate event 
from a device that you have not either explicitly connected to.
For instance even if you don't accept a MaenConnectIndicate 
you may still receive messages from that remote device.

@param aBTDevice address of the device sending us an AVCTP message
@param aTransactionLabel message transaction label
@param aType type of message 
@param aIpidBitSet this will be set to true only if a message has been received indicating 
				   that the profile corresponding to the originally sent message is not valid.
				   If RAvctp was used to send the message then this response will have come from
				   the remote device aBTDevice.
@param aMessageInformation contains only the AVCTP Command / Response Message Information and not the whole packet.
                           Ownership transferred to client.
@see RAvctp
*/	
void CRcpRouter::MaenMessageReceivedIndicate(const TBTDevAddr& aBTDevice,
		SymbianAvctp::TTransactionLabel aTransactionLabel,
		SymbianAvctp::TMessageType aType,
		TBool aIpidBitSet,
		const TDesC8& aMessageInformation)
	{
	LOG_FUNC
	AVRCPLOG(aMessageInformation)
		
	// Find the right handler
	if(aType == ECommand)
		{
		MIncomingCommandHandler* handler = iBearer.IncomingHandler(aBTDevice);
		
		if(!handler)
			{
			iBearer.ConnectIndicate(aBTDevice);
			
			handler = iBearer.IncomingHandler(aBTDevice);
			}
		
		if(handler)
			{
			// If this leaves the handler is rejecting handling this command, just
			// ignore it.
			TRAP_IGNORE(handler->ReceiveCommandL(aMessageInformation, aTransactionLabel, aBTDevice));
			}
		}
	else
		{
		MOutgoingCommandHandler* handler = iBearer.OutgoingHandler(aBTDevice);
		if(handler)
			{
			handler->ReceiveResponse(aMessageInformation, aTransactionLabel, aIpidBitSet);
			}
		}
	}

/** AVCTP Message send complete.

This method is called when a RAvctp has attempted to send 
the message defined by aTransactionLabel and aBTDevice. 
@param aTransactionLabel The transaction label of the message 
						 that has been sent
@param aBTDevice the device to which the send has completed
@param aSendResult KErrNone if the send was successful or one 
				   of the system-wide error codes
@see RAvctp
*/	
void CRcpRouter::MaenMessageSendComplete(const TBTDevAddr& aBTDevice, 
		       		SymbianAvctp::TTransactionLabel aTransactionLabel,   
					TInt aSendResult)
	{
	LOG_FUNC
	__ASSERT_ALWAYS(!iSendQueue.IsEmpty(), AvrcpUtils::Panic(EAvrcpNoOutstandingSend));
	
	CAvrcpCommand* command = iSendQueue.First();
	
	__ASSERT_ALWAYS(command->TransactionLabel() == aTransactionLabel, AvrcpUtils::Panic(EAvrcpUnknownAvctpTransId));
	
	MAvrcpCommandHandler* handler = NULL;
	if(command->MessageType() == ECommand)
		{
		handler = iBearer.OutgoingHandler(aBTDevice);
		}
	else 
		{
		handler = iBearer.IncomingHandler(aBTDevice);
		}
	
	if(handler)
		{
		handler->MessageSent(*command, aSendResult);
		}
		
	// Deque before calling Decrement because Decrement handling may involve
	// deleting command
	command->iSendLink.Deque();
	command->DecrementUsers();

	// Now able to do another send. Toggle our state and check if there
	// are any commands waiting.
	iState = EAvrcpRouterCanSend;
	if(!iSendQueue.IsEmpty())
		{
		Send();
		}
	}

/** AVCTP Close Complete.

This is the response to the CloseGracefully() that has been
called on a RAvctp object. It is the last event that will be 
called until the RAvctp object is Open()'d again.
@see RAvctp
*/
void CRcpRouter::MaenCloseComplete()
	{
	LOG_FUNC
	}

/** AVCTP error notification.

Note an errored device does not indicate that the device has 
been disconnected. If it has then a MaenDisconnectIndicate 
event will be used to indicate this.

@param aBTDevice the remote device associated with the error or TBTDevAddr(0) for a general error
@param aError system wide error
@see RAvctp
*/
void CRcpRouter::MaenErrorNotify(const TBTDevAddr& /*aBTDevice*/, TInt /*aError*/)
	{
	LOG_FUNC
	}

/**
Returns a null aObject if the extension is not implemented,
or a pointer to another interface if it is.

@param aInterface UID of the interface to return
@param aObject system wide error
@see RAvctp
*/	
void CRcpRouter::MaenExtensionInterfaceL(TUid /*aInterface*/, void*& aObject)
	{
	LOG_FUNC
	aObject = NULL;
	}

//------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------

/** Issue a send to AVCTP.

This sends the first message on the command queue.
*/
void CRcpRouter::Send()
	{
	LOG_FUNC
	__ASSERT_ALWAYS(!iSendQueue.IsEmpty(), AvrcpUtils::Panic(EAvrcpNoOutstandingSend));
	
	CAvrcpCommand* command = iSendQueue.First();

#ifdef _DEBUG	
	TInt err = 
#endif // _DEBUG
	iChannel->MacSendMessage(command->RemoteAddress(),
		command->TransactionLabel(),
		command->MessageType(),
		command->Data());
	
	__ASSERT_DEBUG(err == KErrNone, AvrcpUtils::Panic(EAvrcpSendingMessageFailed));
	
	iState = EAvrcpRouterSending;
	}

/** Factory funtion.

@param aAvctp An open RAvctp instance.
@param aBearer The bearer.
@return A fully constructed CRcpRouter.
@leave System wide error codes.
*/
CControlRouter* CControlRouter::NewL(RAvctp& aAvctp, MAvrcpBearer& aBearer)
	{
	LOG_STATIC_FUNC
	CControlRouter* router = new(ELeave) CControlRouter(aAvctp, aBearer);
	CleanupStack::PushL(router);
	router->ConstructL();
	CleanupStack::Pop(router);
	return router;
	}

CControlRouter::~CControlRouter()
	{
	LOG_FUNC
	iAvctp.Close(RAvctp::ENormal);
	}

CControlRouter::CControlRouter(RAvctp& aAvctp, MAvrcpBearer& aBearer)
	: CRcpRouter(aBearer)
	, iAvctp(aAvctp)
	{
	LOG_FUNC
	}

void CControlRouter::ConstructL()
	{
	LOG_FUNC
	LEAVEIFERRORL(iAvctp.Open(*this, KAvrcpPid, iChannel));
	}

/** Factory funtion.

@param aAvctp An open RAvctp instance.
@param aBearer The bearer.
@return A fully constructed CRcpRouter.
@leave System wide error codes.
*/
CBulkRouter* CBulkRouter::NewL(RAvctp& aAvctp, MAvrcpBearer& aBearer)
	{
	LOG_STATIC_FUNC
	CBulkRouter* router = new(ELeave) CBulkRouter(aAvctp, aBearer);
	CleanupStack::PushL(router);
	router->ConstructL();
	CleanupStack::Pop(router);
	return router;
	}

CBulkRouter::~CBulkRouter()
	{
	LOG_FUNC
	iAvctp.UninstallSecondaryChannel();
	}

CBulkRouter::CBulkRouter(RAvctp& aAvctp, MAvrcpBearer& aBearer)
	: CRcpRouter(aBearer)
	, iAvctp(aAvctp)
	{
	LOG_FUNC
	}

void CBulkRouter::ConstructL()
	{
	LOG_FUNC
	LEAVEIFERRORL(iAvctp.InstallSecondaryChannel(*this, iChannel));
	}

