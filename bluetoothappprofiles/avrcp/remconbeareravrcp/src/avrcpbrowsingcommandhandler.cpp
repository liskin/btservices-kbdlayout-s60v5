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
// avrcpincomingcommandhandler.cpp
//



/**
 @file
 @internalComponent
 @released
*/
#include <e32base.h>
#include <remcon/remconbearerbulkobserver.h>
#include <remconaddress.h>

#include "browsecommand.h"
#include "avrcpbrowsingcommandhandler.h"
#include "avrcpinternalinterface.h"
#include "avrcplog.h"
#include "avrcprouter.h"
#include "avrcputils.h"
#include "browsingframe.h"
#include "bulkbearer.h"
#include "mediabrowse.h"
#include "remconcommandinterface.h"

//------------------------------------------------------------------------------------
// Construction/Destruction
//------------------------------------------------------------------------------------

/** Factory function.

@param aCommandInterface	The interface for providing commands that have been handled.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aPlayerInfoManager	The central manager for player information.
@param aAddr	The Bluetooth device address for the remote device handled by this handler.
@return A fully constructed CRcpBrowsingCommandHandler.
@leave System wide error codes.
*/
CRcpBrowsingCommandHandler* CRcpBrowsingCommandHandler::NewL(MRemConBulkCommandInterface& aCommandInterface,
	CBulkRouter& aRouter,
	CAvrcpPlayerInfoManager& aPlayerInfoManager,
	const TBTDevAddr& aAddr)
	{
	LOG_STATIC_FUNC
	CRcpBrowsingCommandHandler* handler = new(ELeave) CRcpBrowsingCommandHandler(aCommandInterface, aRouter, aPlayerInfoManager, aAddr);
	return handler;
	}

/**
@param aCommandInterface	The interface for providing commands that have been handled.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aPlayerInfoManager	The central manager for player information.
@param aAddr	The Bluetooth device address for the remote device handled by this handler.
@return A partially constructed CRcpBrowsingCommandHandler.
*/	
CRcpBrowsingCommandHandler::CRcpBrowsingCommandHandler(MRemConBulkCommandInterface& aCommandInterface,
	CBulkRouter& aRouter,
	CAvrcpPlayerInfoManager& aPlayerInfoManager,
	const TBTDevAddr& aAddr) 
	: iCommandQueue(_FOFF(CBrowseCommand, iHandlingLink))
	, iInternalCommandQueue(_FOFF(CBrowseCommand, iHandlingLink))
	, iCommandInterface(aCommandInterface)
	, iRouter(aRouter)
	, iMtu(335) // initialise to min for browse channel
	, iPlayerInfoManager(aPlayerInfoManager)
	, iAddr(aAddr)
	{
	LOG_FUNC
	}
	
CRcpBrowsingCommandHandler::~CRcpBrowsingCommandHandler()
	{
	LOG_FUNC

	while (!iCommandQueue.IsEmpty())
		{
		CBrowseCommand *command = iCommandQueue.First();
		HandledCommand(*command);
		}
	
	TRemConAddress remConAddr;
	AvrcpUtils::BTToRemConAddr(iAddr, remConAddr);
	iCommandInterface.MrcbciRemoveAddressing(remConAddr);
	}

//---------------------------------------------------------------------
// Called from the bearer
//---------------------------------------------------------------------

/** Tell the handler to gracefully shutdown.

*/
void CRcpBrowsingCommandHandler::Disconnect()
	{
	LOG_FUNC
	
	while (!iCommandQueue.IsEmpty())
		{
		CBrowseCommand* command = iCommandQueue.First();
		iRouter.RemoveFromSendQueue(*command);
		
		HandledCommand(*command);
		}
	}

//------------------------------------------------------------------------------------
// Called by router
//------------------------------------------------------------------------------------

/** Receive an incoming AVRCP browse command.

@param aMessageInformation	The command data from the AVCTP message.
@param aTransactionLabel	AVCTP transaction label for this command.
@param aAddr	The bluetooth device from which this command originated.
@leave System Wide Error code
*/
void CRcpBrowsingCommandHandler::ReceiveCommandL(const TDesC8& aMessageInformation, 
	SymbianAvctp::TTransactionLabel aTransactionLabel, 
	const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	
	// If there's nothing beyond a header this is bobs.  Dump it now.
	AvrcpBrowsing::BrowsingFrame::VerifyFrameL(aMessageInformation);
	
	TUint id = iCommandInterface.MrcciNewTransactionId();
	CBrowseCommand* command = CBrowseCommand::NewL(aMessageInformation, id, aTransactionLabel, aAddr, &iPlayerInfoManager);
	CleanupStack::PushL(command);
	
	TInt result = command->ProcessIncomingCommandL(iMtu);
	CleanupStack::Pop(command);

	command->IncrementUsers();
	
	switch(result)
		{
	case KErrAvrcpFurtherProcessingRequired: 
		{
		// The only command that we need to check out before sending on is
		// SetBrowsedPlayer.  Although it's been parsed to verify that it's 
		// a syntactically valid command we need to ensure that the selected
		// player is available before sending it on.

		__ASSERT_DEBUG(command->RemConInterfaceUid() == TUid::Uid(KRemConMediaBrowseApiUid) && command->RemConOperationId() == ESetBrowsedPlayerOperationId, AVRCP_PANIC(EFurtherProcessingRequiredForNonSetBrowsedPlayer));
		TBool valid = HandleSetBrowsedPlayer(*command);
		
		if(!valid)
			{
			Respond(*command, result);
			command->DecrementUsers();
			break;
			}
		else
			{
			result = KErrNone;
			}
		// valid case fallsthrough to be handled as normal
		}
	case KErrAvrcpHandledInternallyInformRemCon:  // this case falls through
	case KErrNone:
		{
		iCommandQueue.AddLast(*command);
		iCommandInterface.MrcciNewCommand(*command);
		
		if (result == KErrNone)
			{
			break;
			}
		// KErrAvrcpHandledInternallyInformRemCon fallsthrough here
		}
	case KErrAvrcpHandledInternallyRespondNow:
		{
		// If the command has already set payload, just sent the command
		iRouter.AddToSendQueue(*command);
		command->DecrementUsers();
		break;
		}
	case KErrAvrcpInternalCommand:
		{
		iInternalCommandQueue.AddLast(*command);
		HandleInternalCommand(*command);
		break;
		}
	default:
		{
		Respond(*command, result);
		command->DecrementUsers();
		break;
		}
		};
	}

/** Called from the router to indicate send completion.

@param aCommand The command that has been sent.
@param aSendResult KErrNone if the command was sent successfully.  System wide
				   error code otherwise.
*/
void CRcpBrowsingCommandHandler::MessageSent(CAvrcpCommand& /*aCommand*/, TInt /*aSendResult*/)
	{
	LOG_FUNC
	// We try and send the response, but if we fail there's not a lot we can do about
	// it.  Just let the remote assume the response.
	}

void CRcpBrowsingCommandHandler::MaxPacketSize(TInt aMtu)
	{
	iMtu = aMtu-AvrcpBrowsing::KHeaderLength;
	}

//------------------------------------------------------------------------------------
// Called by bearer
//------------------------------------------------------------------------------------

/** Send a response.

@param aInterfaceUid The RemCon interface this response is from.
@param aId The RemCon transaction label of the command to respond to.
@param aData The command response data.
@return KErrNotFound if the command was not found on the queue.
		System wide error codes.
*/
TInt CRcpBrowsingCommandHandler::SendRemConResponse(TUid /*aInterfaceUid*/, TUint aId, RBuf8& aData)
	{
	LOG_FUNC

	return SendResponse(iCommandQueue, aId, aData);
	}

//------------------------------------------------------------------------------------
// Internal command handling functions
//------------------------------------------------------------------------------------


/** Sends a response to the remote device.

@param aCommand The command to respond to.
@param aErr The result of handling the command.
*/
void CRcpBrowsingCommandHandler::Respond(CBrowseCommand& aCommand, TInt aErr)
	{
	LOG_FUNC
	aCommand.SetResult(aErr);
	iRouter.AddToSendQueue(aCommand);
	}

/** To be called on completion of command handling.

This aggregates the handler's tidying up of a finished
command.

@param aCommand The command to tidy up.
*/	
void CRcpBrowsingCommandHandler::HandledCommand(CBrowseCommand& aCommand)
	{
	LOG_FUNC

	aCommand.iHandlingLink.Deque();
	aCommand.DecrementUsers();
	}

void CRcpBrowsingCommandHandler::HandleInternalCommand(CBrowseCommand& aCommand)
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
	case EAvrcpInternalGetFolderItems:
		{
		err = HandleGetFolderItems(id, commandData);
		break;
		}
		};
	
	if(err)
		{
		HandledCommand(aCommand);
		}
	
	commandData.Close();
	}

TInt CRcpBrowsingCommandHandler::HandleGetFolderItems(TUint aId, RBuf8& aCommandData)
	{
	LOG_FUNC

	RBuf8 responseBuf;
	TRAPD(err, DoHandleGetFolderItemsL(aCommandData, responseBuf));
	
	if(!err)
		{
		err = SendInternalResponse(aId, responseBuf);
		}
	
	return err;
	}

void CRcpBrowsingCommandHandler::DoHandleGetFolderItemsL(RBuf8& aCommandData, RBuf8& aResponseData)
	{
	LOG_FUNC

	__ASSERT_DEBUG( &iPlayerInfoManager != NULL, AvrcpUtils::Panic(EAvrcpNotFullyConstructed));
	RAvrcpGetFolderItemsRequest request;
	CleanupClosePushL(request);
	request.ReadL(aCommandData);
	
	// Use 4 bytes even though player ids are 2 bytes becuase of
	// restrictions on RArray preventing use of non-word aligned types
	RArray<TUint> players;
	TInt err = iPlayerInfoManager.PlayerListing(request.iStartItem, request.iEndItem, players);
	CleanupStack::PopAndDestroy(&request);
	
	RRemConMediaErrorResponse errResponse;
	if(err != KErrNone)
		{
		CleanupClosePushL(players);
		errResponse.iPduId = AvrcpBrowsing::EGetFolderItems;
		errResponse.iStatus = (err == KErrArgument) ? AvrcpStatus::ERangeOutOfBounds : AvrcpStatus::EInternalError;
		aResponseData.CreateL(KBrowseResponseBaseLength);
		CleanupClosePushL(aResponseData);
		errResponse.WriteL(aResponseData);
		CleanupStack::Pop(&aResponseData);
		CleanupStack::PopAndDestroy(&players);
		return;
		}
	
	RAvrcpGetFolderItemsResponse response;
	CleanupClosePushL(response);
	CleanupClosePushL(players);
	for(TInt i = 0; i < players.Count(); i++)
		{
		RMediaPlayerItem item;
		CleanupClosePushL(item);
		iPlayerInfoManager.MediaPlayerItemL(players[i], item);
		response.iItems.AppendL(item);
		CleanupStack::Pop(&item);
		}
	
	response.iPduId = AvrcpBrowsing::EGetFolderItems;
	response.iStatus = AvrcpStatus::ESuccess;
	response.iUidCounter = KMediaPlayerListUidCounter;
	response.iNumberItems = players.Count();
	CleanupStack::PopAndDestroy(&players);
	
	//check this fits within MTU, Leave if the response size is bigger than max size 
	CleanupClosePushL(aResponseData);
	if(response.Size() > iMtu)
		{
		
		errResponse.iPduId = AvrcpBrowsing::EGetFolderItems;
		errResponse.iStatus = AvrcpStatus::EInternalError;
		aResponseData.CreateL(KBrowseResponseBaseLength);
		errResponse.WriteL(aResponseData);
		}
	else
		{
		aResponseData.CreateL(response.Size());
		response.WriteL(aResponseData);
		}
	CleanupStack::Pop(&aResponseData);
	CleanupStack::PopAndDestroy(&response);
	}

TInt CRcpBrowsingCommandHandler::SendInternalResponse(TUint aId, RBuf8& aData)
	{
	LOG_FUNC

	return SendResponse(iInternalCommandQueue, aId, aData);
	}

TInt CRcpBrowsingCommandHandler::SendResponse(TDblQue<CBrowseCommand>& aCommandQueue, TUint aId, RBuf8& aData)
	{
	LOG_FUNC
	
	TInt err = KErrNotFound;
	
	TDblQueIter<CBrowseCommand> iter(aCommandQueue);
	CBrowseCommand* command = NULL;

	while (iter)
		{
		command = iter++;
		if(command->RemConCommandId() == aId)
			{
			err = KErrNone;
			command->ProcessOutgoingResponse(aData);
			
			Respond(*command, err);
			aData.Close();
			HandledCommand(*command);
			
			break;
			}		
		}

	return err;
	}

void CRcpBrowsingCommandHandler::SendReject(TUid /*aInterfaceUid*/, TUint aTransactionId)
	{
	LOG_FUNC;

	TDblQueIter<CBrowseCommand> iter(iCommandQueue);
	CBrowseCommand* command = NULL;

	while (iter)
		{
		command = iter++;
		if(command->RemConCommandId() == aTransactionId)
			{
			Respond(*command, KErrAvrcpAirInternalError);
			HandledCommand(*command);
			}		
		}
	}

const TBTDevAddr& CRcpBrowsingCommandHandler::BtAddr() const
	{
	return iAddr;
	}

TBool CRcpBrowsingCommandHandler::HandleSetBrowsedPlayer(CBrowseCommand& aCommand)
	{
	TInt err = KErrNone;
	RRemConSetBrowsedPlayerRequest request;
	
	TRAP(err, request.ReadL(aCommand.CommandData()));
	__ASSERT_DEBUG(err == KErrNone, AvrcpUtils::Panic(ESetBrowsePlayerRequestCorruptedLocally));

	// Check if selected player exists
	TUint16 playerId = request.iPlayerId;
	TRemConClientId clientId;
	TRAP(err, clientId = iPlayerInfoManager.ClientL(playerId));

	if(err == KErrNone)
		{
		// Selected player exists, check with RemCon if we can use it
		TRemConAddress remConAddr;
		AvrcpUtils::BTToRemConAddr(iAddr, remConAddr);
		
		TInt err = iCommandInterface.MrcbciSetAddressedClient(remConAddr, clientId);
		}
	
	if(err != KErrNone)
		{
		// Either the player was incorrect or is already in use, form a RemCon
		// format response, then ask the command to process it for sending out
		// on the air.
		RBuf8 buf;
		TInt bufErr = buf.Create(KMediaBrowseOutBufMaxLength);
		
		if(bufErr == KErrNone)
			{
			RRemConMediaErrorResponse response;
			response.iPduId = 0x70;
			response.iStatus = RAvrcpIPC::SymbianErrToStatus(KErrAvrcpAirInvalidPlayerId);
			TRAP(bufErr, response.WriteL(buf));
			aCommand.ProcessOutgoingResponse(buf);
			buf.Close();
			}
		}
	// else we will continue processing this command as normal
	
	return err == KErrNone ? ETrue : EFalse;
	}
