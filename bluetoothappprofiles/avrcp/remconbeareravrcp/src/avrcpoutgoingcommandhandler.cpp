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

#include <remconcoreapi.h>

#include "avcpanel.h"
#include "controlcommand.h"
#include "avrcpoutgoingcommandhandler.h"
#include "avrcplog.h"
#include "avrcprouter.h"
#include "avrcptimer.h"
#include "avrcputils.h"
#include "controlbearer.h"

//---------------------------------------------------------------------
// Construction/Destruction
//---------------------------------------------------------------------

/** Factory function.

@param aBearer	The CRemConBearerAvrcp this is to handle commands for.
@param aObserver The observer of the bearer. Used to aquire converters.
@param aRouter	A CRcpRouter to use for communication with remote devices.
@param aTimer	CDeltaTimer to use for queuing timed events.
@return A fully constructed CRcpOutgoingCommandHandler.
@leave System wide error codes.
*/
CRcpOutgoingCommandHandler* CRcpOutgoingCommandHandler::NewL(CRemConBearerAvrcp& aBearer, 
	MRemConBearerObserver& aObserver,
	CRcpRouter& aRouter,
	CDeltaTimer& aTimer)
	{
	LOG_STATIC_FUNC
	CRcpOutgoingCommandHandler* handler = new(ELeave)CRcpOutgoingCommandHandler(aBearer, aObserver, aRouter, aTimer);
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
CRcpOutgoingCommandHandler::CRcpOutgoingCommandHandler(CRemConBearerAvrcp& aBearer, 
	MRemConBearerObserver& aObserver,
	CRcpRouter& aRouter,
	CDeltaTimer& aTimer) : iCommandQueue(_FOFF(CControlCommand, iHandlingLink)),
	iNotifyCommandQueue(_FOFF(CControlCommand, iHandlingLink)),
	iBearer(aBearer), iObserver(aObserver), iRouter(aRouter), iTimer(aTimer)
	{
	LOG_FUNC
	}

/** Destructor.
*/
CRcpOutgoingCommandHandler::~CRcpOutgoingCommandHandler()
	{
	LOG_FUNC

	ClearQueue(iCommandQueue);
	ClearQueue(iNotifyCommandQueue);
	}	

void CRcpOutgoingCommandHandler::ClearQueue(TDblQue<CControlCommand>& aQue)
	{
	while(!aQue.IsEmpty())
		{
		CControlCommand* command = aQue.First();
		command->CancelTimer(iTimer);
		command->iHandlingLink.Deque();
		command->DecrementUsers();
		}
	}
//---------------------------------------------------------------------
// Called from the bearer
//---------------------------------------------------------------------

/** Tell the handler to gracefully shutdown.

@param aClearQueue Whether to clear the queue without handling the things 
					on it.  If this is true the commands will be deleted.
					If this is false then pending commands will have responses
					generated to RemCon.
*/
void CRcpOutgoingCommandHandler::Disconnect(TBool aClearQueue)
	{
	LOG_FUNC	
	ProcessDisconnect(iCommandQueue, aClearQueue);
	ProcessDisconnect(iNotifyCommandQueue, aClearQueue);
	}

void CRcpOutgoingCommandHandler::ProcessDisconnect(TDblQue<CControlCommand>& aQue, TBool aClearQueue)
	{
	while(!aQue.IsEmpty())
		{
		CControlCommand* command = aQue.First();
		iRouter.RemoveFromSendQueue(*command);
		command->CancelTimer(iTimer);
		
		if(aClearQueue)
			{
			GenerateFailureResult(*command, KErrDisconnected);
			}

		command->iHandlingLink.Deque();
		command->DecrementUsers();
		}
	}
/** Sends a new command.

@param aInterfaceUid	The RemCon client interface this command is from.
@param aCommand			The operation id within aInterfaceUid.
@param aId				A unique identifier provided by RemCon.
@param aCommandData		Data associated with this command.
@param aAddr			Bluetooth address of device to send this command to.
@leave KErrNoMemory or system wide error code.
@leave Command parsing error.
*/
void CRcpOutgoingCommandHandler::SendCommandL(TUid aInterfaceUid, 
		TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	
	if(aInterfaceUid.iUid == KRemConCoreApiUid)
		{
		// Passthrough commands are stateful, so we need to examine the
		// history - we can't just blindly wham it on the queue.
		HandleCoreApiCommandL(aCommand, aId, aCommandData, aAddr);
		}
	else
		{
		SendCommandL(aInterfaceUid, aCommand, aId, aCommandData, EFalse, aAddr, ETrue, EFalse);
		}
	}

/** Sends a new notify command.

@param aInterfaceUid	The RemCon client interface this command is from.
@param aCommand			The operation id within aInterfaceUid.
@param aId				A unique identifier provided by RemCon, the transaction ID.
@param aCommandData		Data associated with this command.
@param aAddr			Bluetooth address of device to send this command to.
@leave KErrNoMemory or system wide error code.
@leave Command parsing error.
*/
void CRcpOutgoingCommandHandler::SendNotifyCommandL(TUid aInterfaceUid, 
		TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr)
	{
	LOG_FUNC
	SendCommandL(aInterfaceUid, aCommand, aId, aCommandData, EFalse, aAddr, ETrue, ETrue);
	}
	
//---------------------------------------------------------------------
// Data notifications from the router
//---------------------------------------------------------------------

/** Called by the router to provide a new response.

@param aFrame The AV/C frame for this response. Ownership is taken.
@param aTransLabel The AVCTP transaction id of this response. This is used
				to match it with its command.
@param aAddr The remote from which this response originated
*/
void CRcpOutgoingCommandHandler::ReceiveResponse(const TDesC8& aMessageInformation, 
	SymbianAvctp::TTransactionLabel aTransLabel, 
	TBool aIpidBitSet)
	{
	LOG_FUNC
	
	CAVCFrame* frame = NULL;
	TInt err = KErrNone;
	if(!aIpidBitSet)
		{
		TRAP(err, frame = CAVCFrame::NewL(aMessageInformation, AVC::EResponse));
		}
	
	if(!err)
		{
		CControlCommand* command = NULL;
		command = FindInQueue(iCommandQueue, aTransLabel);
		if ( command != NULL )
			{
			//Found, so it is a normal command response.
			ProcessReceiveResponse(frame, aIpidBitSet, command, EFalse);
			}
		else
			{
			//Try to find in the notify command queue.
			command = FindInQueue(iNotifyCommandQueue, aTransLabel);
			if( command != NULL )
			    {
			    //Found, so it is a notify command response.
			    ProcessReceiveResponse(frame, aIpidBitSet, command, ETrue);
			    }
			}
		
		delete frame;
		}
	}

CControlCommand* CRcpOutgoingCommandHandler::FindInQueue(TDblQue<CControlCommand>& aQue, 
		SymbianAvctp::TTransactionLabel aTransLabel)
	{	
	CControlCommand* command = NULL;
	TDblQueIter<CControlCommand> iter(aQue);
	while (iter)
		{
		command = iter++;
		if(command->TransactionLabel() == aTransLabel)
			{
			return command;
			}
		}
	
	return NULL;
	}

void CRcpOutgoingCommandHandler::ProcessReceiveResponse(CAVCFrame* aFrame, 
		TBool aIpidBitSet,
		CControlCommand* aCommand, 
		TBool aNotify)
	{
	aCommand->CancelTimer(iTimer);
	
	TInt err = KErrNone;
	// Inform the bearer if this is something it knows about
	// ie not a click release
	if(aCommand->KnownToBearer())
		{
		if(!aIpidBitSet)
			{
			if(aFrame->Data().Length() < KAVCFrameHeaderLength)
				{
				// Drop corrupt frames
				return;
				}

			err = aCommand->ParseIncomingResponse(iObserver, *aFrame);
			}
		else
			{
			// If aIpidBitSet is true that means AVRCP is not supported
			// by the remote end.  We handle this in the same way as not
			// supported commands, passing them up to RemCon as not 
			// supported, so just map the ctype here, rather than setting
			// up another path for ipid handling, but we need pass as the
			// frame the original because we don't get one from AVCTP if
			// ipid is set.
			aCommand->SetResponseType(KErrNotSupported);
			err = aCommand->ParseIncomingResponse(iObserver, aCommand->Frame());
			}
		
		if ( aNotify )
		    {//This is a notify command
		    iBearer.MrccciNewNotifyResponse(*aCommand);
		    }
		else
			{
			iBearer.MrccciNewResponse(*aCommand);
			}
		}

	TBool doDeque = ETrue;
	if ( (!aIpidBitSet) && (err == KErrNone) && (aNotify) && (aFrame->Type() == AVC::EInterim))
		{
		doDeque = EFalse;
		}
	
	// If this a passthrough press that hasn't yet been released, we need 
	// to wait for a release before getting rid of this, otherwise we're done.
	if(aCommand == iUnreleasedCommand)
		{
		iUnreleasedHasResponse = ETrue;
		StartReleaseTimer(*iUnreleasedCommand);
		doDeque = EFalse;
		}
	
	if ( doDeque )
		{
		aCommand->iHandlingLink.Deque();
		aCommand->DecrementUsers();
		}
	}
/** Called by the router to complete a send.

@param aCommand The command which has been sent.
@param aSendResult The result of the send. KErrNone if successful.
*/	
void CRcpOutgoingCommandHandler::MessageSent(CAvrcpCommand& aCommand, TInt aSendResult)
	{
	LOG_FUNC
	
	if(aSendResult == KErrNone)
		{
		// Set off response timer
		StartResponseTimer(static_cast<CControlCommand&>(aCommand));
		}
	else
		{
		CControlCommand* command = FindInQueue(iNotifyCommandQueue, aCommand.TransactionLabel());
		
		if(command)
			{
			command->SetNotifyVolumeChangeResult(command->Frame());
			iBearer.MrccciNewNotifyResponse(*command);
			}
		else
			{
			command = FindInQueue(iCommandQueue, aCommand.TransactionLabel());
			
			// Generate error response up to RemCon
			// if this is a core command we can set the result, 
			// otherwise we just return it as we got it.
			if(command->Frame().Opcode() == AVC::EPassThrough)
				{
				// Need to insert before setting the button action so we have
				// long enough data
				if (!command->InsertCoreResult(aSendResult))
					{
					if(command->Click())
						{
						command->SetCoreButtonAction(ERemConCoreApiButtonClick, ETrue);
						}
					}
				}
			
			iBearer.MrccciNewResponse(*command);
			}
		
		command->iHandlingLink.Deque();
		command->DecrementUsers();
		}
	}

//---------------------------------------------------------------------
// Internal Utility functions
//---------------------------------------------------------------------

void CRcpOutgoingCommandHandler::CleanupUnreleased()
	{
	iUnreleasedCommand->CancelTimer(iTimer);
	iUnreleasedCommand->iHandlingLink.Deque();
	iUnreleasedCommand->DecrementUsers();
	iUnreleasedHasResponse = EFalse;
	}
	
/** Handle a command that is part of the Core API.

@param aCommand			The operation id within aInterfaceUid.
@param aId				A unique identifier provided by RemCon.
@param aCommandData		Data associated with this command.
@param aAddr			Bluetooth address of device to send this command to.
@leave KErrNoMemory or system wide error code.
@leave Command parsing error.
*/
void CRcpOutgoingCommandHandler::HandleCoreApiCommandL(TUint aCommand, 
		TUint aId,  
		RBuf8& aCommandData, 
		const TBTDevAddr& aAddr)
	{
	if(aCommandData.Length() < KRemConCoreApiButtonDataLength)
   		{
   		User::Leave(KErrCorrupt);
   		}

	TInt buttonData;
	AvrcpUtils::ReadCommandDataToInt(aCommandData, 
   		KRemConCoreApiButtonDataOffset, KRemConCoreApiButtonDataLength, buttonData);
 
 	// First check if there's anything we need to do before sending this command,
 	// mainly releasing a previous press.  	
	if(iUnreleasedCommand)
		{
		TUint prevOpId = iUnreleasedCommand->RemConOperationId();
		
		if(aCommand == prevOpId)
			{
			// Either we've received a release, or we've refreshed the press.
			// If the unreleased press has already been responded too we can
			// dispose of it now.  
			// If the unreleased press has not been responded too we can 
			// treat it like a normal command on reception of response, so just
			// set iUnreleased to NULL.
			if(iUnreleasedHasResponse)
				{
				CleanupUnreleased();
				}

			iUnreleasedCommand = NULL;
			}
		else
			{
			// A new operation!
			if(buttonData != ERemConCoreApiButtonRelease)
				{
				// Try and generate the release for the previous command, if
				// if fails then the remote will just have to assume it.
				// There's no point leaving this to the release timer, because
				// we want to send another command now, so even if we send the
				// release later the remote should ignore it.
				TRAP_IGNORE(GenerateCommandL(*iUnreleasedCommand, ERemConCoreApiButtonRelease));
				
				if(iUnreleasedHasResponse)
					{
					CleanupUnreleased();
					}

				iUnreleasedCommand = NULL;
				}
			else
				{
				// A release for a command other than iUnreleased.  We can't 
				// send this now.
				User::Leave(KErrNotReady);
				}
			}
		}
	else if(buttonData == ERemConCoreApiButtonRelease)
		{
		// We don't have an unreleased command.  We must have already 
		// released this, either via the timer, or because we've sent
		// another command in the meantime.  We can't send this now.
		// Leaving synchronously means we don't need to worry about generating
		// a fake response, which may mislead the application.
		User::Leave(KErrNotReady);
		}
   	
   	if(buttonData == ERemConCoreApiButtonClick)
   		{	
		// aCommandData is still owned by RemCon until we return successfully.
		// If we try the operations with the new data first we won't end up 
		// in a situation where the new CControlCommand thinks that it owns 
		// aCommandData, then the release operation leaves, so RemCon also
		// thinks it owns aCommandData.
		
		RBuf8 pressBuf;
		pressBuf.CreateL(aCommandData);
		CleanupClosePushL(pressBuf);
		
		AvrcpUtils::SetCommandDataFromInt(pressBuf, 
			KRemConCoreApiButtonDataOffset, KRemConCoreApiButtonDataLength, ERemConCoreApiButtonPress);
		SendCommandL(TUid::Uid(KRemConCoreApiUid), aCommand, aId, pressBuf, ETrue, aAddr, ETrue, EFalse);
		
		// Data has been taken ownership of by SendCommandL, so can just let 
		// pressbuf go out of scope.
		CleanupStack::Pop(&pressBuf);
		
		AvrcpUtils::SetCommandDataFromInt(aCommandData, 
			KRemConCoreApiButtonDataOffset, KRemConCoreApiButtonDataLength, ERemConCoreApiButtonRelease);
		SendCommandL(TUid::Uid(KRemConCoreApiUid), aCommand, aId, aCommandData, ETrue, aAddr, EFalse, EFalse);
		}
	else if(buttonData == ERemConCoreApiButtonPress)
		{
		iUnreleasedCommand = &SendCommandL(TUid::Uid(KRemConCoreApiUid), aCommand, aId, aCommandData, EFalse, aAddr, ETrue, EFalse);
		iReleaseTimerExpiryCount = 0;
		}
	else
		{
		// Must be release
		__ASSERT_DEBUG(buttonData == ERemConCoreApiButtonRelease, AvrcpUtils::Panic(EAvrcpUnknownButtonAction));
		SendCommandL(TUid::Uid(KRemConCoreApiUid), aCommand, aId, aCommandData, EFalse, aAddr, ETrue, EFalse);
		}
	}

/** Creates a command from the RemCon data.

This is an internal utility function.

A CControlCommand will be created.  Calling ProcessOutgoingCommandL on
this creates a CAVCFrame from the provided data.  If an AV/C frame
can be created the command will be added to the outgoing queue to
wait a response from the remote.  Otherwise this function will
leave.

@param aInterfaceUid	The RemCon client interface this command is from.
@param aCommand			The operation id within aInterfaceUid.
@param aId				A unique identifier provided by RemCon.
@param aCommandData		Data associated with this command.
@param aIsClick			Whether this is a button click.
@param aAddr			Bluetooth address of device to send this command to.
@return The generated command.
@leave KErrNoMemory or system wide error code.
@leave Command parsing error.
*/
CControlCommand& CRcpOutgoingCommandHandler::SendCommandL(TUid aInterfaceUid,
	TUint aCommand, 
	TUint aId, 
	RBuf8& aCommandData, 
	TBool aIsClick, 
	const TBTDevAddr& aAddr,
	TBool aKnownToBearer,
	TBool aNotify)
	{
	LOG_FUNC
	// Create a command and wham it on our queue, so we can match it up with its response
	// CControlCommand::NewL takes ownership of the data in aCommandData then NULLs aCommandData
	// so a leave later in the function won't cause double deletion.
	CControlCommand* command = CControlCommand::NewL(aInterfaceUid, aCommand, aId, iCurrentTrans, 
			aCommandData, aIsClick, aAddr, aKnownToBearer);
	CleanupStack::PushL(command);	
	
	command->ProcessOutgoingCommandL(iObserver);
	CleanupStack::Pop(command);
	command->IncrementUsers();
	
	if ( aNotify )
		{
		iNotifyCommandQueue.AddLast(*command);
		}
	else
		{
		iCommandQueue.AddLast(*command);
		}
	
	// Increment our transaction id
	iCurrentTrans = (iCurrentTrans + 1) % SymbianAvctp::KMaxTransactionLabel;
	
	// Command stays on the queue till we've got the response
	iRouter.AddToSendQueue(*command);
	
	return *command;
	}

/** Generate a failure response to RemCon.

This sets the result for a passthrough command.
It informs the bearer of the new response.

@param aCommand The command to finish off.
@param aResult The result (only valid for passthrough)
*/
void CRcpOutgoingCommandHandler::GenerateFailureResult(CControlCommand& aCommand, TInt aResult)
	{
	// Response is only necessary if the bearer knows about this command.
	if(aCommand.KnownToBearer() && (aCommand.Frame().Opcode() == AVC::EPassThrough))
		{
		if (aCommand.InsertCoreResult(aResult) == KErrNone)
			{
			if(aCommand.Click())
				{
				aCommand.SetCoreButtonAction(ERemConCoreApiButtonClick, ETrue);
				}
			else if(aCommand.ButtonAct() == AVCPanel::EButtonPress)
				{
				aCommand.SetCoreButtonAction(ERemConCoreApiButtonPress, ETrue);
				}
			else
				{
				aCommand.SetCoreButtonAction(ERemConCoreApiButtonRelease, ETrue);
				}

			iBearer.MrccciNewResponse(aCommand);
           }
		}	
	}
	
/** Generate a command to the remote.

This is needed in situations where the application has not met the avrcp
button refresh requirements so we need to internally generate something
to stop the remote getting a bad impression of us.

@param aCommand The command to be issue again.
*/
void CRcpOutgoingCommandHandler::GenerateCommandL(CControlCommand& aCommand, TRemConCoreApiButtonAction aButtonAct)
	{
	LOG_FUNC
	
	RBuf8 commandData;
	commandData.CreateMaxL(KRemConCoreApiButtonDataLength);
	
	AvrcpUtils::SetCommandDataFromInt(commandData, 
		KRemConCoreApiButtonDataOffset, KRemConCoreApiButtonDataLength, aButtonAct);

	// This will not leave before taking ownership of commandData.
	SendCommandL(aCommand.RemConInterfaceUid(), aCommand.RemConOperationId(), aCommand.RemConCommandId(), commandData, EFalse, 
		aCommand.RemoteAddress(), EFalse, EFalse);
	}

//------------------------------------------------------------------------------------
// Timer functions
//------------------------------------------------------------------------------------

/** Starts the response timer.

AVRCP mandates a remote respond within 100ms of receiving a command.
This is the timer for that, and is started when a command has 
successfully been sent.

@param aCommand The command to start the timer for.
*/		
void CRcpOutgoingCommandHandler::StartResponseTimer(CControlCommand& aCommand)
	{
	LOG_FUNC
	// These use placement new, so cannot fail
	TAvrcpTimerExpiryInfo* timerInfo = new(aCommand.TimerExpiryInfo())TAvrcpTimerExpiryInfo(this, aCommand);

	TCallBack callback(ResponseExpiry, timerInfo);
	TDeltaTimerEntry* timerEntry = new(aCommand.TimerEntry())TDeltaTimerEntry(callback);
	
	iTimer.Queue(KRcpResponseTimeOut, *timerEntry);
	}
	
/** Callback when response timer expires.

This is a static forwarding function.

@param aExpiryInfo The information used by the real ResponseExpiry to
					deal with the timer expiry.
*/
TInt CRcpOutgoingCommandHandler::ResponseExpiry(TAny* aExpiryInfo)
	{
	LOG_STATIC_FUNC
	TAvrcpTimerExpiryInfo *timerInfo = reinterpret_cast<TAvrcpTimerExpiryInfo*>(aExpiryInfo);
	(reinterpret_cast<CRcpOutgoingCommandHandler*>(timerInfo->iHandler))->ResponseExpiry(timerInfo->iCommand);
	
	return KErrNone;
	}
	
/** Deals with response timeout.

This sends a timeout response to RemCon.

@param aCommand	The CControlCommand that has expired.
*/
void CRcpOutgoingCommandHandler::ResponseExpiry(CControlCommand& aCommand)
	{
	LOG_FUNC
	
	GenerateFailureResult(aCommand, KErrTimedOut);	
	
	// Failed to get a response to this, don't bother about trying
	// to release it.
	if(iUnreleasedCommand == &aCommand)
		{
		iUnreleasedCommand = NULL;
		__ASSERT_DEBUG(!iUnreleasedHasResponse, AvrcpUtils::Panic(EAvrcpPressHasPhantomResponse));
		}

	aCommand.iHandlingLink.Deque();
	aCommand.DecrementUsers();
	}

/** Starts the release timer.

AVRCP requires a press to be refreshed less than 2s after the first
press, if it is not to be assumed to have been released.  We pass 
this requirement on to RemCon clients as we don't know when they might
go away and we don't want to keep buttons pressed forever.  If the 
release timer expires we will assume a release, and generate it to
the remote.

@param aCommand The command to start the timer for.
*/	
void CRcpOutgoingCommandHandler::StartReleaseTimer(CControlCommand& aCommand)
	{
	LOG_FUNC

	// These cannot fail as we use placement new
	TAvrcpTimerExpiryInfo* timerInfo = new(aCommand.TimerExpiryInfo())TAvrcpTimerExpiryInfo(this, aCommand);
		
	TCallBack callback(ReleaseExpiry, timerInfo);
	TDeltaTimerEntry* timerEntry = new(aCommand.TimerEntry())TDeltaTimerEntry(callback);

	iTimer.Queue(KRcpOutgoingButtonReleaseTimeout, *timerEntry);
	}

/** Callback when release timer expires.

This is a static forwarding function.

@param aExpiryInfo The information used by the real ReleaseExpiry to
					deal with the timer expiry.
*/	
TInt CRcpOutgoingCommandHandler::ReleaseExpiry(TAny* aExpiryInfo)
	{
	LOG_STATIC_FUNC
	TAvrcpTimerExpiryInfo *timerInfo = reinterpret_cast<TAvrcpTimerExpiryInfo*>(aExpiryInfo);
	(reinterpret_cast<CRcpOutgoingCommandHandler*>(timerInfo->iHandler))->ReleaseExpiry(timerInfo->iCommand);
	
	return KErrNone;
	}

/** Deals with expiry of release timer.

1) Generate release for this command.
2) Send release to remote.

@param aCommand	The CControlCommand that has expired.
*/
void CRcpOutgoingCommandHandler::ReleaseExpiry(CControlCommand& aCommand)
	{
	LOG_FUNC
	__ASSERT_DEBUG((aCommand.ButtonAct() == AVCPanel::EButtonPress), AvrcpUtils::Panic(EAvrcpReleaseExpiryForRelease));
	__ASSERT_DEBUG(&aCommand == iUnreleasedCommand, AvrcpUtils::Panic(EAvrcpReleaseExpiryForOldCommand));
	__ASSERT_DEBUG(iUnreleasedHasResponse, AvrcpUtils::Panic(EAvrcpReleaseTimerStartedWithoutResponse));
		
	iReleaseTimerExpiryCount++;
	
	TBool commandCompleted = ETrue;
	// If the client is not yet obliged to refresh this, then send another press.  Otherwise generate
	// the release for them.
	if((iReleaseTimerExpiryCount * KRcpOutgoingButtonReleaseTimeout) < KRemConCoreApiPressRefreshInterval)
		{
		// This will try and generate a press that is identical to the original
		// aCommand, but with a new AVCTP transaction id.
		TRAPD(err, GenerateCommandL(aCommand, ERemConCoreApiButtonPress));
		
		if(!err)
			{
			// Start the timer again on the original command
			StartReleaseTimer(aCommand);
			commandCompleted = EFalse;
			}
		}
	else
		{
		// Try an generate a release, but if it fails we just have to let the 
		// remote assume it.
		TRAP_IGNORE(GenerateCommandL(aCommand, ERemConCoreApiButtonRelease));
		}
		
	// This condition may be true because
	// -  we have failed to generate a press, in which case the remote is entitled 
	//    to assume this is released, so we just give up on it.
	// or
	// -  the client has not met its press refresh obligation (whether we've 
	//    successfully generated a release or not.
	// In either case we won't do anything more with this command. 
	if(commandCompleted)
		{				
		aCommand.iHandlingLink.Deque();
		aCommand.DecrementUsers();
		
		iUnreleasedCommand = NULL;
		iUnreleasedHasResponse = EFalse;
		iReleaseTimerExpiryCount = 0;
		}
	}
