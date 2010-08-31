// Copyright (c) 2004-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <remcon/avrcpspec.h>
#include <remcon/remconbearerobserver.h>
#include <remconcoreapi.h>

#include "controlcommand.h"
#include "avrcpcommandframer.h"
#include "avrcpincomingcommandhandler.h"
#include "avrcpinternalinterface.h"
#include "avrcplog.h"
#include "avrcprouter.h"
#include "avrcptimer.h"
#include "avrcputils.h"
#include "controlbearer.h"
#include "passthroughhelper.h"
#include "mediainformation.h"
#include "nowplaying.h"
#include "playerinformation.h"
#include "remconbattery.h"
#include "remcongroupnavigation.h"

//------------------------------------------------------------------------------------
// Construction/Destruction
//------------------------------------------------------------------------------------

/** Factory function.

@param aBearer	The CRemConBearerAvrcp this is to handle commands for.
@param aObserver The observer of the bearer. Used to acquire converters.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aTimer	CDeltaTimer to use for queuing timed events.
@return A fully constructed CRcpIncomingCommandHandler.
@leave System wide error codes.
*/
CRcpIncomingCommandHandler* CRcpIncomingCommandHandler::NewL(MRemConControlCommandInterface& aCommandInterface,
	MRemConBearerObserver& aObserver,
	CRcpRouter& aRouter,
	CDeltaTimer& aTimer,
	CAvrcpPlayerInfoManager& aPlayerInfoManager,
	TBTDevAddr& aDevice) 
	{
	LOG_STATIC_FUNC
	CRcpIncomingCommandHandler* handler = new(ELeave)CRcpIncomingCommandHandler(aCommandInterface, aObserver, aRouter, aTimer, aPlayerInfoManager, aDevice);
	CleanupStack::PushL(handler);
	handler->ConstructL();
	CleanupStack::Pop(handler);
	return handler;
	}

/** Constructor.

@param aBearer	The CRemConBearerAvrcp this is to handle commands for.
@param aObserver The observer of the bearer. Used to aquire converters.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aTimer	CDeltaTimer to use for queuing timed events.
@return A partially constructed CRcpIncomingCommandHandler.
@leave System wide error codes.
*/	
CRcpIncomingCommandHandler::CRcpIncomingCommandHandler(MRemConControlCommandInterface& aCommandInterface, 
	MRemConBearerObserver& aObserver,
	CRcpRouter& aRouter,
	CDeltaTimer& aTimer,
	CAvrcpPlayerInfoManager& aPlayerInfoManager,
	TBTDevAddr& aDevice) 
	: iCommandQueue(_FOFF(CControlCommand, iHandlingLink))
	, iInternalCommandQueue(_FOFF(CControlCommand, iHandlingLink))
	, iFragmenter(NULL)
	, iCommandInterface(aCommandInterface)
	, iObserver(aObserver)
	, iRouter(aRouter)
	, iTimer(aTimer)
	, iAddressedMode(EFalse)
	, iPlayerInfoManager(aPlayerInfoManager)
	, iDevice(aDevice)
	{
	LOG_FUNC
	}

void CRcpIncomingCommandHandler::ConstructL()
	{
	LOG_FUNC
	
	iFragmenter = CAVRCPFragmenter::NewL();
	iPlayerInfoManager.AddObserverL(*this);

	RArray<TUint> players;
	iPlayerInfoManager.PlayerListing(players);
	CleanupClosePushL(players);
	
	if(players.Count())
		{
		// Start out with the first player as default.  TSP will need to update
		// us if it wants to set a different one.  Note that this wont be used 
		// unless we enter addressed mode anyway.
		iClientId = iPlayerInfoManager.ClientL(0);
		}
	// if no player have yet been registered we'll set the default when the
	// first one is
	
	CleanupStack::PopAndDestroy(&players);
	
	iPassthroughHelper = CPassthroughHelper::NewL(iRouter, iCommandInterface, iTimer);
	}

/** Destructor.
*/	
CRcpIncomingCommandHandler::~CRcpIncomingCommandHandler()
	{
	LOG_FUNC

	iCommandInterface.MrccciUnregisterForLocalAddressedClientUpdates();
	iPlayerInfoManager.RemoveObserver( *this );
	delete iFragmenter;
	delete iPassthroughHelper;

	// All commands should have been handled by the time we reach here.
	__ASSERT_ALWAYS(iCommandQueue.IsEmpty(), AvrcpUtils::Panic(EAvrcpIncomingCommandsNotHandled));
	__ASSERT_ALWAYS(iInternalCommandQueue.IsEmpty(), AvrcpUtils::Panic(EAvrcpIncomingCommandsNotHandled));
	}

//---------------------------------------------------------------------
// Called from the bearer
//---------------------------------------------------------------------

/** Tell the handler to gracefully shutdown.

*/
void CRcpIncomingCommandHandler::Disconnect()
	{
	LOG_FUNC
	
	iPassthroughHelper->Disconnect();

	if(!iCommandQueue.IsEmpty())
		{
		// Cleanup remaining commands
		while(!iCommandQueue.IsEmpty())
			{
			CControlCommand* command = iCommandQueue.First();
			// Tell RemCon the command is finished with
			iObserver.CommandExpired(command->RemConCommandId());
			// And now remove the command from the bearer
			iRouter.RemoveFromSendQueue(*command);
			HandledCommand(*command);
			}
		}
	if(!iInternalCommandQueue.IsEmpty())
		{
				
		// Cleanup remaining commands
		while(!iInternalCommandQueue.IsEmpty())
			{
			CControlCommand* command = iInternalCommandQueue.First();
			// Tell RemCon the command is finished with
			iObserver.CommandExpired(command->RemConCommandId());
			// And now remove the command from the bearer
			iRouter.RemoveFromSendQueue(*command);
			HandledCommand(*command);
			}
		}
	}

//------------------------------------------------------------------------------------
// Called by router
//------------------------------------------------------------------------------------

void CRcpIncomingCommandHandler::MaxPacketSize(TInt /*aMtu*/)
	{
	// ignore - we don't care if we use AVCTP fragmentation on the
	// control channel
	}

/** Receive an incoming AVRCP command.

The command is parsed from a CAVCFrame into a CControlCommand owned by the
command handler.

@param aFrame	The AVC frame contained within the AVCTP message.
@param aTransactionLabel	AVCTP transaction label for this command.
@param aAddr	The bluetooth device from which this command originated.
@leave System Wide Error code
*/
void CRcpIncomingCommandHandler::ReceiveCommandL(const TDesC8& aMessageInformation, 
	SymbianAvctp::TTransactionLabel aTransactionLabel, 
	const TBTDevAddr& aAddr)
	{
	LOG_FUNC
		
	TUint id = iCommandInterface.MrcciNewTransactionId();
	
	// If there's nothing beyond a header this is bobs.  Dump it now.
	if(aMessageInformation.Length() <= KAVCFrameHeaderLength)
		{
		User::Leave(KErrCorrupt);
		}

	// Parse it out into an AV/C frame
	CAVCFrame* frame = CAVCFrame::NewL(aMessageInformation, AVC::ECommand);
	
	CControlCommand* command = CControlCommand::NewL(frame, id, aTransactionLabel, aAddr, iAddressedMode ? iClientId : KNullClientId, &AvrcpPlayerInfoManager());
	CleanupStack::PushL(command);

	TInt result = command->ParseIncomingCommandL(iObserver, *iFragmenter);
	CleanupStack::Pop(command);

	command->IncrementUsers();
	
	switch(result)
		{
		case KErrAvrcpHandledInternallyInformRemCon:
			{
			HandleRemConCommand(*command);
			iRouter.AddToSendQueue(*command);
			command->DecrementUsers();
			break;
			}
		case KErrNone:
			{
			if(! command->IsPassthrough())
				{
				// add to iCommandQueue for non-passthrough commands
				iCommandQueue.AddLast(*command);
				}
			HandleRemConCommand(*command);
			break;
			}
		case KErrAvrcpInternalCommand:
			{
			iInternalCommandQueue.AddLast(*command);
			HandleInternalCommand(*command);
			break;
			}
		case KErrAvrcpInvalidCType:
			{
			// We should ignore commands with invalid ctype (AV/C v4.0 8.3.1).
			command->DecrementUsers();
			break;
			}
		default:
			{
			Respond(*command, result);
			command->DecrementUsers();
			break;
			}
		}
	}

/** Called from the router to indicate send completion.

@param aCommand The command that has been sent.
@param aSendResult KErrNone if the command was sent successfully.  System wide
				   error code otherwise.
*/
void CRcpIncomingCommandHandler::MessageSent(CAvrcpCommand& /*aCommand*/, TInt /*aSendResult*/)
	{
	LOG_FUNC
	// We try and send the response, but if we fail there's not a lot we can do about
	// it.  Just let the remote assume the response.
	}

//------------------------------------------------------------------------------------
// Called by bearer
//------------------------------------------------------------------------------------

/** Send a response.

@param aInterfaceUid The RemCon interface this response is from.
@param aId The RemCon transaction label of the command to respond to.
@param aData The command response data.
@return KErrNone.
*/
TInt CRcpIncomingCommandHandler::SendRemConResponse(TUid /*aInterfaceUid*/, TUint aId, RBuf8& aData)
	{
	LOG_FUNC

	// We always take ownership of the response data in SendResponse, so we 
	// always return KErrNone.
	SendResponse(iCommandQueue, aId, aData);
	return KErrNone;
	}

void CRcpIncomingCommandHandler::SendReject(TUid aInterfaceUid, TUint aId)
	{
	LOG_FUNC

	TDblQueIter<CControlCommand> iter(iCommandQueue);
	CControlCommand* command = NULL;

	while (iter)
		{
		command = iter++;
		if(command->RemConCommandId() == aId)
			{
			TInt err = KErrGeneral;
			if (aInterfaceUid.iUid == KRemConMediaInformationApiUid || aInterfaceUid.iUid == KRemConPlayerInformationUid)
				{
				err = KErrAvrcpMetadataInternalError;
				}
			
			Respond(*command, err);
			
			HandledCommand(*command);
			}		
		}
	}

//------------------------------------------------------------------------------------
// MPlayerChangeObserver functions
//------------------------------------------------------------------------------------
void CRcpIncomingCommandHandler::MpcoAvailablePlayersChanged()
	{
	LOG_FUNC 
	
	TDblQueIter<CControlCommand> internalIter(iInternalCommandQueue);
	CControlCommand* command = NULL;
	TInt err = KErrNone;
	
	while(internalIter)
		{
		command = internalIter++;
		if((command->RemConInterfaceUid() == TUid::Uid(KUidAvrcpInternalInterface)) && (command->RemConOperationId() == EAvrcpInternalAvailablePlayersNotification))
			{
			// If this fails we cann't send the notification - just give up
			err = HandleRegisterAvailablePlayersNotification(*command);
			static_cast<void>(err = err); // ignore this error (i.e. give up).
			}
		}
	//register internal notification
	//still need to optimize for avoid re-registering notification
	RArray<TUint> players;
	err = iPlayerInfoManager.PlayerListing(players);
	
	if(err == KErrNone)
		{
		TBool currentClientAvailable = EFalse;

		for(TInt i = 0 ; i < players.Count() ; i++)
			{
			TRemConClientId clientId = 0;
			clientId = iPlayerInfoManager.Client(players[i]);
			if(clientId == iClientId)
				{
				currentClientAvailable = ETrue;
				}
			}
		
		if((iClientId == KNullClientId || !currentClientAvailable) && players.Count())
			{
			// If this is the first target client we set our default client id
			// to this
			iClientId = iPlayerInfoManager.Client(players[0]);
			}
		}

	players.Close();
	}

void CRcpIncomingCommandHandler::MpcoAddressedPlayerChangedLocally(TRemConClientId aClientId)
	{
	LOG_FUNC
	
	TRAP_IGNORE(AddressedPlayerChangedL(aClientId));
	}

void CRcpIncomingCommandHandler::MpcoUidCounterChanged(TRemConClientId aClientId)
	{
	LOG_FUNC
	
	CControlCommand* command;
	if(aClientId == iClientId)
		{
		command = FindNotify(iInternalCommandQueue, TUid::Uid(KUidAvrcpInternalInterface),EAvrcpInternalUidChangedNotification);
		if(command)
			{
			// if we fail to send an update it is effectively the same condition
			// as when the notification arrives after the controller uses the
			// old UID counter
			TInt err = HandleUidChangedNotification(*command);
			
			if(err != KErrNone)
				{
				Respond(*command, KErrAvrcpAirInternalError);
				HandledCommand(*command);
				}
			}
		}
	}

//------------------------------------------------------------------------------------
// Internal command handling functions
//------------------------------------------------------------------------------------


/** Sends a response to the remote device.

Because of the 100ms timeout for responses we send a response to
passthrough commands as soon as we receive them.  This means the real
response from RemCon is currently ignored.  A real response is
only sent if this is a vendor dependent command.

@param aCommand The command to respond to.
@param aErr The result of handling the command.  KErrNone if successful.
			KErrNotSupported if this operation is not supported.
*/		
void CRcpIncomingCommandHandler::Respond(CControlCommand& aCommand, TInt aErr)
	{
	LOG_FUNC
	aCommand.SetResponseType(aErr);
	iRouter.AddToSendQueue(aCommand);
	}

/** To be called on completion of command handling.

This aggregates the handler's tidying up of a finished
command.

@param aCommand The command to tidy up.
*/	
void CRcpIncomingCommandHandler::HandledCommand(CControlCommand& aCommand)
	{
	LOG_FUNC
	aCommand.CancelTimer(iTimer);
	aCommand.iHandlingLink.Deque();
	aCommand.DecrementUsers();
	}

void CRcpIncomingCommandHandler::HandleInternalCommand(CControlCommand& aCommand)
	{
	LOG_FUNC
	
	TUid interfaceUid;
	TUint id;
	TUint operationId;
	RBuf8 commandData;
	TBTDevAddr addr;
	
	aCommand.GetCommandInfo(interfaceUid, id, operationId, commandData, addr);
	
	__ASSERT_DEBUG(interfaceUid == TUid::Uid(KUidAvrcpInternalInterface), AvrcpUtils::Panic(EAvrcpInternalHandlingRequestedOnWrongInterface));
	
	TInt err = KErrNone;
	switch(operationId)
		{
	case EAvrcpInternalSetAddressedPlayer:
		{
		err = HandleSetAddressedPlayer(id, commandData);
		break;
		}
	case EAvrcpInternalAvailablePlayersNotification:
		{
		err = HandleRegisterAvailablePlayersNotification(aCommand);
		break;
		}
	case EAvrcpInternalAddressedPlayerNotification:
		{
		err = HandleRegisterAddressedPlayerNotification(aCommand);
		break;
		}
	case EAvrcpInternalUidChangedNotification:
		{
		err = HandleUidChangedNotification(aCommand);
		break;
		}
		};

	if(err != KErrNone)
		{
		Respond(aCommand, KErrAvrcpAirInternalError);
		HandledCommand(aCommand);
		}

	commandData.Close();
	}

void CRcpIncomingCommandHandler::HandleRemConCommand(CControlCommand& aCommand)
	{
	LOG_FUNC
	
	if(aCommand.IsPassthrough())
		{
		// This deals with button press/release stuff,
		// queues the command and responds
		iPassthroughHelper->HandlePassthrough(aCommand);
		}
	else 
		{
		// can go directly to client (unlike passthrough which may need to map 2 commands to 1 click
		if (aCommand.Frame().Type() == AVC::ENotify)
			{
			iCommandInterface.MrccciNewNotifyCommand(aCommand, aCommand.ClientId());
			}
		else
			{
			iCommandInterface.MrcciNewCommand(aCommand, aCommand.ClientId());
			}
		}
	}

TInt CRcpIncomingCommandHandler::HandleSetAddressedPlayer(TUint aId, RBuf8& aCommandData)
	{
	LOG_FUNC
	
	// Once we respond to this we've told the remote that we're using a particular player
	EnterAddressedMode();
	
	// SetAddressedPlayer involves not just responding to this command but
	// also rejecting a bunch of notifies and completing the addressed player
	// changed notify.  We try as hard as we can to ensure our state remains
	// consistent with the view of the remote.
	
	// Allocate the response buffer for the SetAddressedPlayer command first
	RBuf8 responseBuf;
	TInt err = responseBuf.Create(KSetAddressedPlayerResponseSize);
	if(err == KErrNone)
		{
		// Now we know we can at least try and send a response to the remote 
		// do the other gubbins, which we can cope with failing
		TRAPD(setResult, DoHandleSetAddressedPlayerL(aCommandData));
	
		switch(setResult)
			{
		case KErrNotFound:
			{
			RAvrcpIPCError errorResponse;
			errorResponse.iError = KErrAvrcpAirInvalidPlayerId;
	
			// Can ignore this as we know we have allocated a big enough buffer
			TRAP_IGNORE(errorResponse.WriteL(responseBuf));
			break;
			}
		case KErrNone:
		default:
			// For any result other than KErrNotFound we managed to set the 
			// addressed client.  Other errors indicate a failure in sending
			// a changed response to the AddressedPlayerChanged notify.  Even
			// if we failed sending a changed response we should have been
			// able to reject the outstanding notify so we are in a consistent
			// state
			{
			RAvrcpSetAddressedPlayerResponse response;
			response.iStatus = AvrcpStatus::ESuccess;
			
			// Can ignore this as we know we have allocated a big enough buffer
			TRAP_IGNORE(response.WriteL(responseBuf));

			iCommandInterface.MrccciSetAddressedClient(iClientId);
			break;
			}
			};
	
		SendInternalResponse(aId, responseBuf);
		responseBuf.Close();
		}

	return err;
	}

void CRcpIncomingCommandHandler::DoHandleSetAddressedPlayerL(RBuf8& aCommandData)
	{
	LOG_FUNC

	RAvrcpSetAddressedPlayerRequest request;
	request.ReadL(aCommandData);
	
	TRemConClientId clientId = iPlayerInfoManager.ClientL(request.iPlayerId);
	AddressedPlayerChangedL(clientId);
	}

void CRcpIncomingCommandHandler::AddressedPlayerChangedL(TRemConClientId aClientId)
	{
	LOG_FUNC

	if(aClientId == iClientId)
		{
		return;
		}

	iClientId = aClientId;
	
	TDblQueIter<CControlCommand> iter(iCommandQueue);
	CControlCommand* command = NULL;
	
	// Complete player specific notifications
	while(iter)
		{
		command = iter++;
		if(command->PlayerSpecificNotify())
			{
			iObserver.CommandExpired(command->RemConCommandId());

			Respond(*command, KErrAvrcpAirAddressedPlayerChanged);
			
			HandledCommand(*command);
			}
		}

	command = FindNotify(iInternalCommandQueue, TUid::Uid(KUidAvrcpInternalInterface),EAvrcpInternalAddressedPlayerNotification);
	if(command)
		{
		User::LeaveIfError(HandleRegisterAddressedPlayerNotification(*command));
		}
	}

TInt CRcpIncomingCommandHandler::HandleRegisterAvailablePlayersNotification(CControlCommand& aCommand)
	{
	LOG_FUNC

	RBuf8 responseBuf;
	TRAPD(err, DoHandleRegisterAvailablePlayersNotificationL(responseBuf, aCommand));
	
	if(err == KErrNone)
		{
		SendInternalResponse(aCommand.RemConCommandId(), responseBuf);
		}

	responseBuf.Close();
	return err;
	}

void CRcpIncomingCommandHandler::DoHandleRegisterAvailablePlayersNotificationL(RBuf8& aResponseData, CControlCommand& aCommand)
	{
	LOG_FUNC

	RAvrcpIPCError response;
	response.iError = KErrNone;

	if(DuplicateNotify(iInternalCommandQueue, aCommand))
		{
		response.iError = KErrAvrcpAirInvalidCommand;
		}
	
	aResponseData.CreateL(KRegisterNotificationEmptyResponseSize);
	CleanupClosePushL(aResponseData);
	
	response.WriteL(aResponseData);
	CleanupStack::Pop();
	}

TInt CRcpIncomingCommandHandler::HandleRegisterAddressedPlayerNotification(CControlCommand& aCommand)
	{
	LOG_FUNC

	// Once we respond to this we've told the remote that we're using a particular player
	EnterAddressedMode();
	
	RBuf8 responseBuf;
	TRAPD(err, DoHandleRegisterAddressedPlayerNotificationL(responseBuf, aCommand));
	
	if(!err)
		{
		SendInternalResponse(aCommand.RemConCommandId(), responseBuf);
		}

	responseBuf.Close();
	return err;
	}

void CRcpIncomingCommandHandler::DoHandleRegisterAddressedPlayerNotificationL(RBuf8& aResponseData, CControlCommand& aCommand)
	{
	LOG_FUNC

	RAvrcpAddressedPlayerNotificationResponse response;
	RAvrcpIPCError rejectResponse;
	
	if(DuplicateNotify(iInternalCommandQueue, aCommand))
		{
		aResponseData.CreateL(KRegisterNotificationEmptyResponseSize);
		rejectResponse.iError = KErrAvrcpAirInvalidCommand;
		CleanupClosePushL(aResponseData);
		rejectResponse.WriteL(aResponseData);
		}
	else
		{
		// Tricky situation thinking.Reject if at this moment the client just be shut down
		TRAPD(err, response.iPlayerId = iPlayerInfoManager.PlayerL(iClientId));
		if(err != KErrNone)
			{
			aResponseData.CreateL(KRegisterNotificationEmptyResponseSize);
			rejectResponse.iError = KErrAvrcpAirInvalidCommand;
			CleanupClosePushL(aResponseData);
			rejectResponse.WriteL(aResponseData);
			}
		else
			{
			// This line will never leave once the previous line pass 
			response.iUidCounter = iPlayerInfoManager.UidCounterL(iClientId);
			aResponseData.CreateL(KRegisterNotificationAddressedPlayerResponseSize);
			CleanupClosePushL(aResponseData);
			response.WriteL(aResponseData);
			}
		}
	
	CleanupStack::Pop();
	}

TInt CRcpIncomingCommandHandler::HandleUidChangedNotification(CControlCommand& aCommand)
	{
	LOG_FUNC

	// Although we haven't strictly told the remote which player we're using this is 
	// a 1.4 command, and implies use of a specific player so switch into addressed mode
	EnterAddressedMode();

	RBuf8 responseBuf;
	TUint16 uidCounter = 0;
	TRAPD(err, uidCounter = iPlayerInfoManager.UidCounterL(iClientId));
	
	if(err == KErrNone && !DuplicateNotify(iInternalCommandQueue, aCommand))
		{
		TRAP(err, DoHandleUidChangedNotificationL(responseBuf, uidCounter));
		}

	if(err == KErrNone)
		{
		SendInternalResponse(aCommand.RemConCommandId(), responseBuf);
		}

	responseBuf.Close();
	return err;
	}

void CRcpIncomingCommandHandler::DoHandleUidChangedNotificationL(RBuf8& aResponseData, TUint16 aUidCounter)
	{
	LOG_FUNC

	RAvrcpUidCounterNotificationResponse response;
	response.iUidCounter = aUidCounter;
	aResponseData.CreateL(KRegisterNotificationUidChangedResponseSize);
	CleanupClosePushL(aResponseData);
	response.WriteL(aResponseData);
	CleanupStack::Pop();
	}

void CRcpIncomingCommandHandler::SendInternalResponse(TUint aId, RBuf8& aData)
	{
	LOG_FUNC 

	SendResponse(iInternalCommandQueue, aId, aData);
	}

/**
This function always takes responsibility for the response.
*/
void CRcpIncomingCommandHandler::SendResponse(TDblQue<CControlCommand>& aCommandQueue, TUint aId, RBuf8& aData)
	{
	LOG_FUNC
	
	TDblQueIter<CControlCommand> iter(aCommandQueue);
	CControlCommand* command = NULL;

	while (iter)
		{
		command = iter++;
		if(command->RemConCommandId() == aId)
			{
			TInt err = command->ProcessOutgoingResponse(iObserver, aData, *iFragmenter);
			
			if(command->Frame().Type() == AVC::ENotify)
				{
				// If the interim response was successful then create a new command 
				// to contain the final response when it arrives.  The same command
				// cannot be re-used as we may not have finished sending this before
				// we get the final response.  We won't have both the commands on 
				// the handling queue at one time though, so there is no ambiguity
				// about which is which.  We finish handling the interim response here.
				if(err == KErrNone)
					{
					// Try creating the CControlCommand which will be used for the
					// final response to the Notify.  If it fails then we will just
					// reject the notify straight away.
					//
					// To start with we set its AV/C frame to be an Interim response, 
					// since this will match the AV/C frame of the original CControlCommand
					// once we send the interim response later in this function.
					CControlCommand* finalResponse = NULL;
					TRAP(err, finalResponse = command->InterimResponseL());
				
					if(err == KErrNone)
						{
						finalResponse->IncrementUsers();
						aCommandQueue.AddLast(*finalResponse);
						}
					}
				
				if(err != KErrNone && command->NormalCommand())
					{
					// If we had an unsuucessful interim response, we need to remove the command from remcon
					iObserver.CommandExpired(aId);
					}
				}

			Respond(*command, err);
			HandledCommand(*command);
			break;  // Exit while (iter) loop
			}
		}
	
	// Either we have created a response which took ownership of aData, or
	// we didn't match a command, so this was a response to something we've
	// already removed from our queue.  We're telling RemCon that we dealt ok
	// with this so we have resonsibility for tidying up the data.
	aData.Close();
	}

void CRcpIncomingCommandHandler::EnterAddressedMode()
	{
	iAddressedMode = ETrue;
	iCommandInterface.MrccciRegisterForLocalAddressedClientUpdates();
	}

TBool CRcpIncomingCommandHandler::DuplicateNotify(TDblQue<CControlCommand>& aCommandQueue, const CControlCommand& aCommand) const
	{
	TUid interfaceUid = aCommand.RemConInterfaceUid();
	TUint operationId = aCommand.RemConOperationId();
	
	CControlCommand* command = NULL;
	TDblQueIter<CControlCommand> iter(aCommandQueue);
	TInt count = 0;
	TBool duplicate = EFalse;
	
	while(iter)
		{
		command = iter++;
		if((interfaceUid == command->RemConInterfaceUid())&&(operationId == command->RemConOperationId()))
			{
			count++;
			// this should be a reject if we've already got a notification outstanding
			if(count > 1)
				{
				duplicate = ETrue;
				break;
				}
			}
		}
	
	return duplicate;
	}

CControlCommand* CRcpIncomingCommandHandler::FindNotify(TDblQue<CControlCommand>& aCommandQueue, TUid aInterfaceUid, TUint aOperationId)
	{
	CControlCommand* command = NULL;
	TDblQueIter<CControlCommand> iter(aCommandQueue);
	TBool found = EFalse;
	
	while(iter)
		{
		command = iter++;

		if((command->RemConInterfaceUid() == aInterfaceUid)&&(command->RemConOperationId() == aOperationId))
			{
			found = ETrue;
			break;
			}
		}
	
	return found ? command : NULL;
	}
