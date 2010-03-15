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
#include <avcframe.h>
#include <e32base.h>
#include <remcon/remconbearerobserver.h>
#include <remcon/remconconverterplugin.h>
#include <remcon/messagetype.h>
#include <remconbeareravrcp.h>

#include <absolutevolumeapi.h>
#include <absolutevolumeutils.h>
#include <remcon/avrcpspec.h>
#include "controlcommand.h"
#include "avrcpcommandframer.h"
#include "avrcpfragmenter.h"
#include "avrcpinternalinterface.h"
#include "avrcpipc.h"
#include "avrcplog.h"
#include "avrcputils.h"
#include "avrcpincomingcommandhandler.h"
#include "avrcp.h"
#include "mediabrowse.h"
#include "mediainformation.h"
#include "nowplaying.h"
#include "playerinformation.h"
#include "remconbattery.h"
#include "remcongroupnavigation.h"

//---------------------------------------------------------------------
// Outgoing command construction
//---------------------------------------------------------------------

/** Factory function.

@param aInterfaceUid The RemCon interface uid of this command.
@param aCommand The operation id of this command within the interface defined
				by aInterface Uid.
@param aRemConId The RemCon transaction label.
@param aTransactionLabel The AVCTP transaction label.
@param aCommandData The RemCon command data associated with this command.
@param aIsClick Whether this command is a button click (ie RemCon believes
				that this and the other part of the click constitute one
				command.
@param aAddr The bluetooth address to send this command to.
@return A fully constructed CControlCommand.
@leave System wide error codes.
*/
CControlCommand* CControlCommand::NewL(TUid aInterfaceUid,
	TUint aCommand, 
	TUint aRemConId, 
	SymbianAvctp::TTransactionLabel aTransactionLabel, 
	RBuf8& aCommandData, 
	TBool aIsClick, 
	const TBTDevAddr& aAddr,
	TBool aKnownToBearer)
	{
	LOG_STATIC_FUNC
	CControlCommand* command = new (ELeave) CControlCommand(aInterfaceUid, aCommand, 
			aRemConId, aTransactionLabel, aCommandData, aIsClick, aAddr, aKnownToBearer);
	CleanupStack::PushL(command);
	command->ConstructL();
	CleanupStack::Pop(command);
	return command;
	}

/** Constructor.

@param aInterfaceUid The RemCon interface uid of this command.
@param aCommand The operation id of this command within the interface defined
				by aInterface Uid.
@param aRemConId The RemCon transaction label.
@param aTransactionLabel The AVCTP transaction label.
@param aCommandData The RemCon command data associated with this command.
@param aIsClick Whether this command is a button click (ie RemCon believes
				that this and the other part of the click constitute one
				command.
@param aAddr The bluetooth address to send this command to.
@return A constructed CControlCommand.
@leave System wide error codes.
*/
CControlCommand::CControlCommand(TUid aInterfaceUid,
	TUint aCommand,
	TUint aRemConId, 
	SymbianAvctp::TTransactionLabel aTransactionLabel, 
	RBuf8& aCommandData,
	TBool aIsClick,
	const TBTDevAddr& aAddr,
	TBool aKnownToBearer) 
	: CAvrcpCommand(aRemConId, aTransactionLabel, aAddr)
	{
	LOG_FUNC
	
	iIsClick = aIsClick;
	iInterfaceUid = aInterfaceUid;
	iOperationId = aCommand;
	iKnownToBearer = aKnownToBearer;
	
	iCommandData.Assign(aCommandData);
	aCommandData.Assign(NULL);
	iPlayerInfoManager = NULL;
	}
	
//---------------------------------------------------------------------
// Incoming command construction
//---------------------------------------------------------------------

/** Factory function.

@param aMessageInformation A buffer containing AV/C frame this command is to represent.
@param aRemConId The RemCon transaction label.
@param aTransLabel The AVCTP transaction label.
@param aAddr The bluetooth address to send the response to.
@param aClientId The RemCon client that should receive this command
@return A fully constructed CControlCommand.
@leave System wide error codes.
*/
CControlCommand* CControlCommand::NewL(CAVCFrame* aFrame, 
	TUint aRemConId, 
	SymbianAvctp::TTransactionLabel aTransLabel, 
	const TBTDevAddr& aAddr,
	const TRemConClientId& aClientId,
	CAvrcpPlayerInfoManager* aPlayerInfoManager)
	{
	LOG_STATIC_FUNC
	CControlCommand* command = new(ELeave)CControlCommand(aFrame, aRemConId, aTransLabel, aAddr, aClientId, aPlayerInfoManager);
	CleanupStack::PushL(command);
	command->ConstructL();
	CleanupStack::Pop(command);
	return command;
	}

/** Constructor.

@param aRemConId The RemCon transaction label.
@param aTransLabel The AVCTP transaction label.
@param aAddr The bluetooth address to send the response to.
@param aClientId The RemCon client that should receive this command
@return A partially constructed CControlCommand.
@leave System wide error codes.
*/	
CControlCommand::CControlCommand(CAVCFrame* aFrame, 
	TUint aRemConId, 
	SymbianAvctp::TTransactionLabel aTransLabel, 
	const TBTDevAddr& aAddr,
	const TRemConClientId& aClientId,
	CAvrcpPlayerInfoManager* aPlayerInfoManager) 
	: CAvrcpCommand(aRemConId, aTransLabel, aAddr)
	, iFrame(aFrame)
	, iClientId(aClientId)
	{
	LOG_FUNC

	iIsClick = ETrue; // Assume click until we know otherwise
	iPlayerInfoManager = aPlayerInfoManager;
	}

//---------------------------------------------------------------------
// Generic construction/destruction
//---------------------------------------------------------------------

/** Destructor.
*/
CControlCommand::~CControlCommand()
	{
	LOG_FUNC
	__ASSERT_DEBUG(iUsers == 0, AvrcpUtils::Panic(EAvrcpCommandStillInUse));
	__ASSERT_ALWAYS(!iHandlingLink.IsQueued(), AvrcpUtils::Panic(EAvrcpCommandStillQueuedForHandling));
	__ASSERT_ALWAYS(!iReadyLink.IsQueued(), AvrcpUtils::Panic(EAvrcpCommandStillQueuedAsReady));
	__ASSERT_ALWAYS(!iSendLink.IsQueued(), AvrcpUtils::Panic(EAvrcpCommandStillQueuedForSending));
	delete iFrame;
	iCommandData.Close();
	delete iTimerEntry;
	delete iTimerExpiryInfo;
	}
	
/** Second phase construction.

@leave System wide error codes.
*/
void CControlCommand::ConstructL()
	{
	LOG_FUNC

	// Allocate these now so we know we have the memory.  Info is
	// irrelevant as we won't add to the timer's queue without 
	// setting the true info.
	TCallBack callback(DummyCallback, NULL);
	iTimerEntry = new(ELeave)TDeltaTimerEntry(callback);
	iTimerExpiryInfo = new(ELeave)TAvrcpTimerExpiryInfo(NULL, *this);
	}
	
//------------------------------------------------------------------------------------
// From MRcpTimerNotify
//------------------------------------------------------------------------------------	

/** Get the timer entry.

@return Timer entry.
*/
TDeltaTimerEntry* CControlCommand::TimerEntry()
	{
	return iTimerEntry;
	}
	
/** Get the timer expiry info.

@return Timer expiry info.
*/
TAvrcpTimerExpiryInfo* CControlCommand::TimerExpiryInfo()
	{
	return iTimerExpiryInfo;
	}

/** Remove this command's timer entry from the queue.

@param aTimer The timer queue to remove this from.
*/	
void CControlCommand::CancelTimer(CDeltaTimer& aTimer)
	{
	LOG_FUNC

	aTimer.Remove(*iTimerEntry);
	}
	
//------------------------------------------------------------------------------------
// Called by bearer
//------------------------------------------------------------------------------------

const TRemConClientId& CControlCommand::ClientId() const
	{
	return iClientId;
	}
//------------------------------------------------------------------------------------
// Called by handlers
//------------------------------------------------------------------------------------

/** Creates iFrame.

This function must be called between creating this command and using it.

@param aInterfaceUid	The RemCon interface this command came from.
@param aCommand			The command id within the interface identified by aInterfaceUid.
@param aCommandData		Data supplied with this command by RemCon.  The format of this
						data is defined by RemCon and is dependent on aInterfaceUid and 
						aCommand.
@leave System wide error code if parsing could not complete.
*/
void CControlCommand::ProcessOutgoingCommandL(MRemConBearerObserver& aObserver)
	{
	LOG_FUNC
			
	switch(iInterfaceUid.iUid)
		{
		//Process the absolute volume controller api
		case KRemConAbsoluteVolumeControllerApiUid:
			{
			switch (iOperationId)
			    {
			    //Registers absolute volume changed
			    case KRemConAbsoluteVolumeNotification:
			        {
					iFrame = AvrcpCommandFramer::NotifyVolumeChangeCommandL();
			        break;
			        }
			    //Sets absolute volume.
			    case KRemConSetAbsoluteVolume:
			    	{
			    	//Gets the absolute volume to be set.
			    	RRemConAbsoluteVolumeRequest setAbsVol;
			    	CleanupClosePushL(setAbsVol);
					setAbsVol.ReadL(iCommandData);
					
					__ASSERT_ALWAYS(setAbsVol.iVolume <= setAbsVol.iMaxVolume, 
							AvrcpUtils::Panic(EAvrcpVolumeBeyondMaxVolume));
					
					TUint8 absVol = KAvrcpMaxAbsoluteVolume * setAbsVol.iVolume / setAbsVol.iMaxVolume;
					iFrame = AvrcpCommandFramer::SetAbsoluteVolumeCommandL(absVol);	
					CleanupStack::PopAndDestroy(&setAbsVol);					
			    	break;
			    	}
			    default:
			    	{
			    	User::Leave(KErrNotSupported);
			    	}
			    
			    }			
			break;
			}
		case KRemConCoreApiUid:
			{
			// Default interface - all commands are passthrough
			AVCPanel::TOperationId avrcpOp;
			
			if((RemConToAvrcpOperation(iOperationId, avrcpOp) != KErrNone) ||
			   (iCommandData.Length() < KRemConCoreApiButtonDataLength))
				{
				User::Leave(KErrCorrupt);
				}
			else
				{
				TInt remConButtonAct;
				AvrcpUtils::ReadCommandDataToInt(iCommandData, 
					KRemConCoreApiCommandDataOffset + KRemConCoreApiButtonDataOffset,
					KRemConCoreApiButtonDataLength, remConButtonAct);
		
				AVCPanel::TButtonAction buttonAct = (remConButtonAct == ERemConCoreApiButtonPress) ? 
					AVCPanel::EButtonPress : AVCPanel::EButtonRelease;

				iFrame = AvrcpCommandFramer::PassthroughL(avrcpOp, buttonAct);
				if(iIsClick)
					{
					// restore our mangled command data
					AvrcpUtils::SetCommandDataFromInt(iCommandData, 
						KRemConCoreApiCommandDataOffset + KRemConCoreApiButtonDataOffset,
						KRemConCoreApiButtonDataLength, ERemConCoreApiButtonClick);
					}
				}
			break;
			}	
		default:
			{
			RBuf8 buf;
			buf.CreateMaxL(KAVCFrameMaxLength);
			User::LeaveIfError(aObserver.InterfaceToBearer(TUid::Uid(KRemConBearerAvrcpImplementationUid), 
				iInterfaceUid, iOperationId, iCommandData, ERemConCommand, buf));

			CleanupClosePushL(buf);
			iFrame = CAVCFrame::NewL(buf, AVC::ECommand);
			CleanupStack::PopAndDestroy(&buf);
			break;
			}
		};
	}
	
/** Fills in command info from iFrame.

This must be called by the command handler before handling this
command.

This functions sets iInterfaceUid, iOperationId and iCommandData
to the correct values according to iFrame.  The format of iCommandData
is defined by RemCon and is dependent on iInterfaceUid and iOperationId.

@return KErrNone				If the frame has been parsed successfully.
@return KErrNotSupported		This frame represents a command for which a
								RemCon converter or client side interface
								cannot be found.
@return KErrAvrcpInvalidCType	The CType specified in this frame is invalid.
@return KErrAvrcpMetadataInvalidCommand		The AVRCP command is invalid.
@return KErrAvrcpMetadataInvalidParameter	The AVRCP parameter is invalid.
@return KErrAvrcpMetadataParameterNotFound	The AVRCP parameter was not found.
@return KErrAvrcpMetadataInternalError		An AVRCP internal error occurred (such as out-of-memory,
											or an inter-process communication error)
@return KErrCorrupt	        	If the frame is corrupted(e.g invalid Operation Id).
@return	System wide error code.
*/
TInt CControlCommand::ParseIncomingCommandL(MRemConBearerObserver& aObserver, CAVRCPFragmenter& aFragmenter)
	{
	LOG_FUNC
	TInt err = KErrNotSupported;
	
	switch(iFrame->Type())
		{
		// check it isn't a reponse
		case AVC::ENotImplemented:
		case AVC::EAccepted:
		case AVC::ERejected:
		case AVC::EInTransition:
		case AVC::EImplemented:
		case AVC::EChanged:
		case AVC::EInterim:
		case 0x0E:	// not given a enum for SC reasons; reserved response code in spec
			{
			// We were told this was a command, can't go using response
			// CTypes here matey
			err = KErrAvrcpInvalidCType;
			break;
			}
		case AVC::EGeneralEnquiry:
		case AVC::ESpecificEnquiry:
			{
			err = KErrNotSupported;
			break;
			}
		default:
			if (iFrame->Opcode() == AVC::EVendorDependent)
				{
				err = ParseIncomingVendorCommandL(aObserver, aFragmenter);
				}
			
			else
				{
				// give off to the regular processor
				err = ParseIncomingKnownOpcodeL(aObserver);
				}
			break;
		};


	return err;
	}
	
/** Processes an incoming response.

This function may not fail.  We always need to generate something
to RemCon.

@param aObserver Observer to use for retrieving converter.
@param aFrame The AV/C frame containing the response.
*/
TInt CControlCommand::ParseIncomingResponse(MRemConBearerObserver& aObserver, const CAVCFrame& aFrame)
	{
	LOG_FUNC	
	TInt error = KErrNone;
	
	// Compare Opcode with that of the sent frame rather than the
	// received one because we trust that more.  Should be the same
	// as this is matched by AVCTP transaction label, but who knows
	// what those illicit remote devices could be up to.
	if(iFrame->Opcode() == AVC::EPassThrough)
		{
		switch(aFrame.Type())
			{		
			case AVC::EAccepted:
				{
				InsertCoreResult(KErrNone);
				break;
				}
			case AVC::ENotImplemented:
				{
				InsertCoreResult(KErrNotSupported);
				break;
				}
			default:
				{
				InsertCoreResult(KErrGeneral);
				break;
				}
			}
		}	
	else if (iFrame->Opcode() == AVC::EVendorDependent)
		{
		TPtrC8 payloadData;
		AVC::TAVCVendorId vID;
		//Get the PDU ID with that of the sent frame rather than the received one,
		//the reason is the same to above comments.
		payloadData.Set(CAVCVendorDependentCommand::GetPayloadAndVID(*iFrame, vID));
		if (vID == KBluetoothSIGVendorId)
			{
			TMetadataTransferPDUID metadataPDUID = MetadataTransferParser::GetPDUID(payloadData);
			switch ( metadataPDUID )
			    {
			    case ESetAbsoluteVolume://Response for setting absolute volume.
			    	{
			    	error = SetSetAbsoluteVolumeResult(aFrame);
			    	break;
			    	}
			    case ERegisterNotification:
			    	{
			    	//Get notify event ID with the sent frame rather than the received one
			    	//because there is a big possibility that the received one is an error response, e.g. rejected,notimplemented.
			    	//In order to make sure this is an absolute volume response even if the response is an error response, 
			    	//we have to use the sent frame, and then we can process absolute volume specifically.
			    	TMetadataTransferNotifyEventID eventID = MetadataTransferParser::GetNotifyEventID(payloadData);
			    	
			    	__ASSERT_ALWAYS(eventID == ERegisterNotificationVolumeChanged,
			    			AvrcpUtils::Panic(EAvrcpInvalidEventId));
			    	
			    	if (eventID == ERegisterNotificationVolumeChanged)
			    		{
			    		error = SetNotifyVolumeChangeResult(aFrame);
			    		}
			    	break;
			    	}
			    default:
			    	{
			    	// Should never hit here
			    	AvrcpUtils::Panic(EAvrcpResponseToUnknownCommand);
			        break;
			    	}
			    }
			}
		else
			{
			ParseIncomingUnknownResponse(aObserver, aFrame);
			}
		}
	else
		{
		ParseIncomingUnknownResponse(aObserver, aFrame);
		}
	
	return error;
	}

/** Processes an outgoing response.

This should only be called for vendor dependent commands as
we respond to passthrough commands internally.

@param aObserver Observer to use for retrieving converter.
@param aFrame The command data for the response.
*/
TInt CControlCommand::ProcessOutgoingResponse(MRemConBearerObserver& aObserver,
//											TRemConMessageType aMessageType,
											RBuf8& aResponseData,
											CAVRCPFragmenter& aFragmenter)
	{
	TRAPD(err, DoProcessOutgoingResponseL(aObserver,aResponseData, aFragmenter));
	return err;
	}

void CControlCommand::DoProcessOutgoingResponseL(MRemConBearerObserver& aObserver,
											 RBuf8& aResponseData,
											 CAVRCPFragmenter& aFragmenter)
	{
	LOG_FUNC
	
	// Payload size may be increased in GenerateMetadataResponsePayload
	// if there's a very large response which needs fragmenting
	RBuf8 payload;
	payload.CreateL(KAVCFrameMaxLength);
	CleanupClosePushL(payload);

	if(( iInterfaceUid.iUid == KRemConMediaInformationApiUid ) 
		|| ( iInterfaceUid.iUid == KRemConPlayerInformationUid )
		|| ( iInterfaceUid.iUid == KRemConAbsoluteVolumeTargetApiUid )
		|| ( iInterfaceUid.iUid == KRemConNowPlayingApiUid )
		|| ( iInterfaceUid.iUid == KUidAvrcpInternalInterface))
		{
		// metadata
		// "this" is the command for which the response lurks in aCommandData
		// GenerateMetadataResponsePayload() MUST set PDU id, fragmentation stauts
		// and paramlen (4 bytes total) - check this in ASSERT_DEBUG
		User::LeaveIfError(GenerateMetadataResponsePayload(aObserver, payload, aResponseData));
		__ASSERT_DEBUG(payload.Length() >= KAVRCPMinVendorDependentResponseLen, AvrcpUtils::Panic(EAvrcpFunnyLengthData));
		aResponseData.Close();
		
		if (payload.Length() > KAVCMaxVendorDependentPayload)
			{
			// Fragment response (in payload) and queue fragments ready
			// for sending when CT sends a CONTINUE request. If any other
			// request is received (other than pass-through) then throw
			// away our fragmented packet, as the CT has aborted.
			aFragmenter.AssignPayload(payload);
			payload.Assign(NULL);
			payload.Close();
			
			// Re-allocate this back to a sensible size
			// from the much larger size, which has now been
			// assigned to fragmenter (avoids copying payload)
			payload.CreateL(KAVCFrameMaxLength);
			payload.Append(aFragmenter.GetNextFragmentHeader());
			payload.Append(aFragmenter.GetNextFragment());
			}
			
		CAVCFrame* frame = CAVCVendorDependentResponse::NewL(KBluetoothSIGVendorId);
		frame->Append(payload);
		frame->SetType(iFrame->Type());
		delete iFrame;
		iFrame = frame;
		}
	else
		{
		User::LeaveIfError(aObserver.InterfaceToBearer(TUid::Uid(KRemConBearerAvrcpImplementationUid),
							iInterfaceUid, iOperationId,
							aResponseData, /*ERemConCommand*/ERemConResponse, payload));
		aResponseData.Close();
		CAVCFrame* frame = CAVCFrame::NewL(payload, AVC::EResponse);
		delete iFrame;
		iFrame = frame;
		}
		
	CleanupStack::PopAndDestroy(&payload);
	}

/** Set the response type in the AV/C frame.

@param aErr The result of processing the operation. KErrNone if 
			successful. KErrNotsupported if this operation is not
			implemented, eg because no converter was found.
*/
void CControlCommand::SetResponseType(TInt aErr)
	{
	LOG_FUNC
	AVC::TCType cType = iFrame->Type();
	switch(aErr)
		{
		case KErrNone:
		case KErrCompletion:
		case KErrAvrcpHandledInternallyRespondNow:
			if (cType == AVC::EControl)
				{
				iFrame->SetType(AVC::EAccepted);
				}
			else if (cType == AVC::ENotify)
				{
				iFrame->SetType(AVC::EInterim);
				}
			else if (cType == AVC::EInterim)
				{
				iFrame->SetType(AVC::EChanged);
				}
			else if (cType == AVC::EStatus)
				{
				iFrame->SetType(AVC::EStable);
				}
			else 
				{
				iFrame->SetType(AVC::EImplemented);
				}
			break;
		case KErrAvrcpMetadataInvalidCommand:
		case KErrAvrcpMetadataInvalidParameter:
		case KErrAvrcpMetadataParameterNotFound:
		case KErrAvrcpMetadataInternalError:
		case KErrAvrcpAirInvalidCommand:
		case KErrAvrcpAirInvalidParameter:
		case KErrAvrcpAirParameterNotFound:
		case KErrAvrcpAirInternalError:
		case KErrAvrcpAirSuccess:
		case KErrAvrcpAirUidChanged:
		case KErrAvrcpAirReserved:
		case KErrAvrcpAirInvalidDirection:
		case KErrAvrcpAirNotADirectory:
		case KErrAvrcpAirDoesNotExist:
		case KErrAvrcpAirInvalidScope:
		case KErrAvrcpAirRangeOutOfBounds:
		case KErrAvrcpAirUidIsADirectory:
		case KErrAvrcpAirMediaInUse:
		case KErrAvrcpAirNowPlayingListFull:
		case KErrAvrcpAirSearchNotSupported:
		case KErrAvrcpAirSearchInProgress:
		case KErrAvrcpAirInvalidPlayerId:
		case KErrAvrcpAirPlayerNotBrowesable:
		case KErrAvrcpAirPlayerNotAddressed:
		case KErrAvrcpAirNoValidSearchResults:
		case KErrAvrcpAirNoAvailablePlayers:
		case KErrAvrcpAirAddressedPlayerChanged:
			{
			// If this fails, we're OOM (it only contains a NewL)
			// so we can't send the error response - just give up
			TRAPD(err, GenerateMetadataRejectPayloadL(aErr));
			err = err;  // avoid warning about not using this
			break;
			}
		default:
			iFrame->SetType(AVC::ENotImplemented);
		}
	iFrame->SetFrameType(AVC::EResponse);	
	}

/** Gets this command's AV/C frame.
@return the AV/C frame for this command
*/	
const CAVCFrame& CControlCommand::Frame() const
	{
	LOG_FUNC
	return *iFrame;
	}

const TDesC8& CControlCommand::Data() const
	{
	LOG_FUNC
	return iFrame->Data();
	}

SymbianAvctp::TMessageType CControlCommand::MessageType() const
	{
	LOG_FUNC
	return (iFrame->FrameType() == AVC::ECommand) ? SymbianAvctp::ECommand : SymbianAvctp::EResponse;
	}

/** Gets the button action from this command's AV/C frame.
This is only valid on passthrough commands.

@return The button action.
*/
AVCPanel::TButtonAction CControlCommand::ButtonAct() const
	{
	LOG_FUNC
	AVCPanel::TButtonAction act;
	iFrame->ButtonAct(act);
	return act;
	}

/** Gets whether this command is currently assumed to be a click.

This is used to support the click facility offered by RemCon, which
is not offered by AVRCP.  As such AVRCP internally simulates outgoing
clicks by generating a press and release for one RemCon click.  When
responses are received we know that if a command is a click we should
send only one response up to RemCon.

Incoming passthrough press commands are assumed to be click until
the hold timer expires.  When a matching release is received we can
then tell whether we need to send a single click up to RemCon, or 
a release to match the press that was sent when the hold timer expired.

@return ETrue is this is a click. EFalse if not.
*/
TBool CControlCommand::Click() const
	{
	LOG_FUNC
	return iIsClick;
	}

/** Sets whether this command is currently assumed to be a click
or not.

@see CRcpcommand::Click()
@param aIsClick ETrue to set this as click. EFalse to set this as 
				not click.
*/	
void CControlCommand::SetClick(TBool aIsClick)
	{
	LOG_FUNC
	iIsClick = aIsClick;
	}

/** Sets the RemCon data to indicate what button action this 
command is.  This function is only valid for commands in the
core api.

@param aButtonAct The RemCon button action for this command.
@param aCommand Whether this is a command. This is needed
				because the command data is at a different offset for
				commands and responses.
*/
void CControlCommand::SetCoreButtonAction(TRemConCoreApiButtonAction aButtonAct, TBool aCommand)
	{
	LOG_FUNC
	
	TInt offset = aCommand ? KRemConCoreApiButtonDataOffset + KRemConCoreApiCommandDataOffset
						   : KRemConCoreApiButtonDataOffset + KRemConCoreApiResponseDataOffset;
						   
	AvrcpUtils::SetCommandDataFromInt(iCommandData, offset, 
						  KRemConCoreApiButtonDataLength, aButtonAct);
	}
/** ReSets the RemCon data to indicate what button action this 
command is. This function is called when we the command is being re-used to generate a 
new command to remconServ.

@param aButtonAct The RemCon button action for this command.
@param aCommand Whether this is a command. This is needed
				because the command data is at a different offset for
				commands and responses.
*/
void CControlCommand::ReSetCoreButtonActionL(TRemConCoreApiButtonAction aButtonAct, TBool aCommand)
	{
	LOG_FUNC
	
	if (iCommandData.MaxLength() < KRemConCoreApiButtonDataLength)
		{
		iCommandData.Close();
		iCommandData.CreateMaxL(KRemConCoreApiButtonDataLength);
		}
	
	SetCoreButtonAction(aButtonAct, aCommand);
	}

/** Inserts the results at the beginning of this command's data.
If the data buffer is not large enough it will be ReAlloced to 
allow the insertion.

@return The result to pass to RemCon. KErrNone for an AV/C accepted.
		KErrNotSupported for an AV/C not implemented. KErrGeneral
		for an AV/C rejected. 
*/
TInt CControlCommand::InsertCoreResult(TInt aResult)
	{
	LOG_FUNC
	TInt err = KErrNone;
	TInt requiredLength = KRemConCoreApiResultDataLength + iCommandData.Length();
	
	if(iCommandData.Length() >= requiredLength)
		{
		// Insert data to write result into
		iCommandData.Insert(0, KRemConCoreApiResultPad);
		}
	else
		{
		// need longer buffer
		err = iCommandData.ReAlloc(requiredLength);
		if(!err)
			{
			iCommandData.Insert(0, KRemConCoreApiResultPad);
			}
		else
			{
			return err;
			}
		}

	AvrcpUtils::SetCommandDataFromInt(iCommandData, 0, 
						  KRemConCoreApiResultDataLength, aResult);
	return err;
	}

/**
Sets the result of set absolute volume response into this command's data
*/
TInt CControlCommand::SetSetAbsoluteVolumeResult(const CAVCFrame& aFrame)
	{
	TInt err = KErrNone;
    TRAP(err, DoSetAbsoluteVolumeResultL(aFrame));
    if (err != KErrNone)
    	{
    	// Ensure the client can receive an error in case of 
    	// DoSetAbsoluteVolumeResultL leaves out.
    	iCommandData.Zero();
    	TPckgBuf<TInt> errBuf(err);
    	iCommandData.Append(errBuf);
    	iCommandData.SetLength(iCommandData.MaxLength());
    	}
    return err;
	}

void CControlCommand::DoSetAbsoluteVolumeResultL(const CAVCFrame& aFrame)
	{
	RRemConAbsoluteVolumeResponse absVol;	
	absVol.iError = KErrGeneral;
	absVol.iMaxVolume = KAvrcpMaxAbsoluteVolume;
	
	CleanupClosePushL(absVol);	
	
	switch(aFrame.Type())
        {
        case AVC::EAccepted:
	        {
	        if (aFrame.Data().Length() == KLengthSetAbsoluteVolumeResponse)
		        {
		        absVol.iError = KErrNone;
		        TUint volumeOffset = KLengthSetAbsoluteVolumeResponse - 1;
		        absVol.iVolume = KAbsoluteVolumeMask & aFrame.Data()[volumeOffset];
		        }
		    break;
		    }
        case AVC::ERejected: // fall through
        case AVC::ENotImplemented:
        	break;
	    default:
	    	break;
        }
	
	absVol.WriteL(iCommandData);
	CleanupStack::PopAndDestroy(&absVol);
	}
/**
Sets the result of volume changed notification response into this command's 
data.
*/
TInt CControlCommand::SetNotifyVolumeChangeResult(const CAVCFrame& aFrame)
	{
	TInt err = KErrNone;
	TRAP(err, DoSetNotifyVolumeChangeResultL(aFrame));
    if (err == KErrNone)
    	{
    	// Through AVC::TCType the RemCon sever can know whether the response
    	// is an Interim or Changed or any other responses, so the RemCon 
    	// server can decide to remove the notify command from its 
    	// outgoingsent queue or not.
    	iFrame->SetType(aFrame.Type());
    	}
    else
    	{
    	// Ensure the client can receive an error in case of 
    	// DoSetNotifyVolumeChangeResultL leaves out.
    	iCommandData.Zero();
    	TPckgBuf<TInt> errBuf(KErrGeneral);
    	iCommandData.Append(errBuf);
    	iCommandData.SetLength(iCommandData.MaxLength());
    	
    	// Setting AVC::TCType to ERejected is intended to let the RemCon
    	// server to remove the notify command from its outgoingsent queue
    	// in case of DoSetNotifyVolumeChangeResultL leaves out.
    	iFrame->SetType(AVC::ERejected);
    	}
    
    return err;
	}

void CControlCommand::DoSetNotifyVolumeChangeResultL(const CAVCFrame& aFrame)
	{
	if (iCommandData.MaxLength() < KAbsoluteVolumeResponseDataSize)
		{
		iCommandData.Close();
		iCommandData.CreateL(KAbsoluteVolumeResponseDataSize);
		}
	
	RRemConAbsoluteVolumeResponse absVol;
	absVol.iError = KErrGeneral;
	absVol.iMaxVolume = KAvrcpMaxAbsoluteVolume;
	
	CleanupClosePushL(absVol);	
	
	switch(aFrame.Type())
        {
	    case AVC::EInterim:
	    case AVC::EChanged:
		    {
		    if (aFrame.Data().Length() == KLengthNotifyVolumeChangeResponse)
		    	{
		    	absVol.iError = KErrNone;
		    	TUint volumeOffset = KLengthNotifyVolumeChangeResponse - 1;
		    	absVol.iVolume = KAbsoluteVolumeMask & aFrame.Data()[volumeOffset];			    	
		    	}		    
		    break;
		    }
	    case AVC::ERejected: // fall through
	    case AVC::ENotImplemented:
	    	break;
	    default:
	    	break;
        }
	absVol.WriteL(iCommandData);
	CleanupStack::PopAndDestroy(&absVol);
	}
//------------------------------------------------------------------------------------
// Internal utility functions
//------------------------------------------------------------------------------------

/** Fills in command info from an AVC Control.

This functions sets iInterfaceUid, iOperationId and iCommandData
to the correct values according to iFrame.  The format of iCommandData
is defined by RemCon and is dependent on iInterfaceUid and iOperationId.

@return KErrNone				If the frame has been parsed successfully.
@return KErrNotSupported		This frame represents a command for which a
								RemCon converter or client side interface
								cannot be found.
@return	System wide error code.
*/
TInt CControlCommand::ParseIncomingKnownOpcodeL(MRemConBearerObserver& aObserver)
	{
	LOG_FUNC
	TInt err = KErrNotSupported;
	
	AVC::TCType cType = iFrame->Type();
	
	switch(iFrame->Opcode())
		{
		case AVC::EPassThrough:
			{
			if(iFrame->Data().Length() < KAVCPassthroughFrameLength)
				{
				LEAVEL(KErrCorrupt);
				}
			if (iFrame->SubunitType() != AVC::EPanel)
				{
				LEAVEL(KErrNotSupported);
				}
	
			TUint8 avrcpOp;			
			if (cType != AVC::EGeneralEnquiry && cType == AVC::EControl)
				{
				iCommandData.CreateMaxL(KRemConCoreApiButtonDataLength);
				err = iFrame->OperationId(avrcpOp);
				if (err == KErrNone)
					{
					if (avrcpOp!=AVCPanel::EVendorUnique)
						{
						err = AvrcpToRemConOperation(avrcpOp, iOperationId, iInterfaceUid);
						}
					else
						{
						err = ParseVendorUniquePassthroughCommand(aObserver);
						}
					}
				
				if (err!=KErrNone)
					{
					err = KErrAvrcpInvalidOperationId;
					}
				}
			else
				{
				iCommandData.Close();
				iCommandData.CreateL(KAVCFrameMaxLength);
				TRemConMessageType message = ERemConCommand;
				err = aObserver.BearerToInterface(TUid::Uid(KRemConBearerAvrcpImplementationUid),
													iFrame->Data(),
													iFrame->Data(),
													iInterfaceUid,
													iOperationId,
													message,
													iCommandData);
				}
			break;
			}
		case AVC::EUnitInfo:
			{
			if (iFrame->Type() == AVC::EStatus)
				{
				CAVCFrame* resp = AvrcpCommandFramer::UnitInfoResponseL();
				delete iFrame;
				iFrame = resp;
				err = KErrCompletion; // since bearer has done its job without client needed
				}
			else
				{
				err = KErrAvrcpInvalidCType;
				}
			break;
			}
		case AVC::ESubunitInfo:
			{
			if (iFrame->Type() == AVC::EStatus)
				{
				CAVCFrame* resp = AvrcpCommandFramer::SubunitInfoResponseL();
				delete iFrame;
				iFrame = resp;
				err = KErrCompletion; // since bearer has done its job without client needed
				}
			else
				{
				err = KErrAvrcpInvalidCType;
				}
			break;
			}

		default:
			{
			iCommandData.Close();
			iCommandData.CreateL(KAVCFrameMaxLength);
			TRemConMessageType message = ERemConCommand;
			err = aObserver.BearerToInterface(TUid::Uid(KRemConBearerAvrcpImplementationUid),
												iFrame->Data(),
												iFrame->Data(),
												iInterfaceUid,
												iOperationId,
												message,
												iCommandData);
			break;
			}
		}
		
	return err;
	}
	

/** Fills in command info from an AVC Vendor Dependent message.

This functions sets iInterfaceUid, iOperationId and iCommandData
to the correct values according to iFrame.  The format of iCommandData
is defined by RemCon and is dependent on iInterfaceUid and iOperationId.
The AVC frame's length is checked that it at least contains the vendor id.

@param aObserver An observer to be used to obtain a converter.
@return KErrNone				If the frame has been parsed successfully.
@return KErrNotSupported		This frame represents a command for which a
								RemCon converter or client side interface
								cannot be found.
@return KErrAvrcpInvalidCType	The CType specified in this frame is invalid.
@return KErrAvrcpMetadataInvalidCommand		The AVRCP command is invalid.
@return KErrAvrcpMetadataInvalidParameter	The AVRCP parameter is invalid.
@return KErrAvrcpMetadataParameterNotFound	The AVRCP parameter was not found.
@return KErrAvrcpMetadataInternalError		An AVRCP internal error occurred (such as out-of-memory,
											or an inter-process communication error)
@return	System wide error code.
*/	
TInt CControlCommand::ParseIncomingVendorCommandL(MRemConBearerObserver& aObserver, CAVRCPFragmenter& aFragmenter)
	{
	LOG_FUNC
	TInt err = KErrNone;

	SetVendorInfoL(EFalse); // set id and payload; leaves if not enough space available
	
	if (iVendorId!=KBluetoothSIGVendorId)
		{
		iCommandData.Close();
		iCommandData.CreateL(KAVCFrameMaxLength);
	
		TRemConMessageType message = ERemConCommand;
		
		err = aObserver.BearerToInterface(TUid::Uid(KRemConBearerAvrcpImplementationUid),
											iFrame->Data(),
											iFrame->Data(),
											iInterfaceUid,
											iOperationId,
											message,
											iCommandData);
		}
	else
		{
		// process v>1.0 version of AVRCP
		// which use vendor dependent frames to extend v1.0 of AVRCP
		// the vendor code has the value for the BT SIG
		if (iFrame->SubunitType() != AVC::EPanel)
			{
			// this is for Control not Metadata
			return KErrNotSupported;
			}
		
		err = ParseMetadataTransferVendorCommand(aFragmenter);
		if (err == KErrNone)
			{
			// Check that the interface UID is non-zero
			__ASSERT_DEBUG(iInterfaceUid != TUid::Uid(0), AvrcpUtils::Panic(EAvrcpInterfaceUidNotSet));
			}
		}
	return err;
	}


/** Creates RemCon command information from iFrame.

This functions sets iInterfaceUid, iOperationId and iCommandData
to the correct values according to iFrame.  The format of iCommandData
is defined by the interface, iInterfaceUid and is dependent on 
iOperationId.  A converter should be able to be found as this response
is a result of an outgoing command on this interface.

@param aObserver An observer used to get a converter.
@param aFrame The AV/C frame for this command.
*/
void CControlCommand::ParseIncomingUnknownResponse(MRemConBearerObserver& aObserver, 
	const CAVCFrame& aFrame)
	{
	LOG_FUNC
	// We need to pass a response up to RemCon even if we can't get a
	// converter to generate a decent response so we don't 

	iCommandData.Close();
	TInt err = iCommandData.Create(KAVCFrameMaxLength);
	if(!err)
		{
		TRemConMessageType type = ERemConResponse; // output param
		err = aObserver.BearerToInterface(TUid::Uid(KRemConBearerAvrcpImplementationUid),
			aFrame.Data(), aFrame.Data(), iInterfaceUid, iOperationId, type, iCommandData);
		}
	}

/** Translates from an AVC operation id to RemCon's core interface.

@param aAvrcpOp		The AVC passthrough operation id.
@param aRemConOp	On return the RemCon operation id within the core interface.
@return KErrNone	If the operation has been translated successfully.
@return KErrNotSupported	If the operation does not correspond to one
							in the RemCon core interface.
*/
TInt CControlCommand::AvrcpToRemConOperation(TUint aAvrcpOp, TUint& aRemConOp, TUid& aRemConIf)
	{
	LOG_STATIC_FUNC
	TInt err = KErrNone;

	//TBH setting here as most are for the Core API
	//some cases will override
	aRemConIf = TUid::Uid(KRemConCoreApiUid);

	switch(aAvrcpOp)	
		{
		case AVCPanel::ESelect:
			aRemConOp = ERemConCoreApiSelect;
			break;
		case AVCPanel::EUp:
			aRemConOp = ERemConCoreApiUp;
			break;
		case AVCPanel::EDown:
			aRemConOp = ERemConCoreApiDown;
			break;
		case AVCPanel::ELeft:
			aRemConOp = ERemConCoreApiLeft;
			break;
		case AVCPanel::ERight:
			aRemConOp = ERemConCoreApiRight;
			break;
		case AVCPanel::ERightUp:
			aRemConOp = ERemConCoreApiRightUp;
			break;
		case AVCPanel::ERightDown:
			aRemConOp = ERemConCoreApiRightDown;
			break;
		case AVCPanel::ELeftUp:
			aRemConOp = ERemConCoreApiLeftUp;
			break;
		case AVCPanel::ELeftDown:
			aRemConOp = ERemConCoreApiLeftDown;
			break;
		case AVCPanel::ERootMenu:
			aRemConOp = ERemConCoreApiRootMenu;
			break;
		case AVCPanel::ESetupMenu:
			aRemConOp = ERemConCoreApiSetupMenu;
			break;
		case AVCPanel::EContentsMenu:
			aRemConOp = ERemConCoreApiContentsMenu;
			break;
		case AVCPanel::EFavoriteMenu:
			aRemConOp = ERemConCoreApiFavoriteMenu;
			break;
		case AVCPanel::EExit:
			aRemConOp = ERemConCoreApiExit;
			break;
		case AVCPanel::E0:
			aRemConOp = ERemConCoreApi0;
			break;
		case AVCPanel::E1:
			aRemConOp = ERemConCoreApi1;
			break;
		case AVCPanel::E2:
			aRemConOp = ERemConCoreApi2;
			break;
		case AVCPanel::E3:
			aRemConOp = ERemConCoreApi3;
			break;
		case AVCPanel::E4:
			aRemConOp = ERemConCoreApi4;
			break;
		case AVCPanel::E5:
			aRemConOp = ERemConCoreApi5;
			break;
		case AVCPanel::E6:
			aRemConOp = ERemConCoreApi6;
			break;
		case AVCPanel::E7:
			aRemConOp = ERemConCoreApi7;
			break;
		case AVCPanel::E8:
			aRemConOp = ERemConCoreApi8;
			break;
		case AVCPanel::E9:
			aRemConOp = ERemConCoreApi9;
			break;
		case AVCPanel::EDot:
			aRemConOp = ERemConCoreApiDot;
			break;
		case AVCPanel::EEnter:
			aRemConOp = ERemConCoreApiEnter;
			break;
		case AVCPanel::EClear:
			aRemConOp = ERemConCoreApiClear;
			break;
		case AVCPanel::EChannelUp:
			aRemConOp = ERemConCoreApiChannelUp;
			break;
		case AVCPanel::EChannelDown:
			aRemConOp = ERemConCoreApiChannelDown;
			break;
		case AVCPanel::EPreviousChannel:
			aRemConOp = ERemConCoreApiPreviousChannel;
			break;
		case AVCPanel::ESoundSelect:
			aRemConOp = ERemConCoreApiSoundSelect;
			break;
		case AVCPanel::EInputSelect:
			aRemConOp = ERemConCoreApiInputSelect;
			break;
		case AVCPanel::EDisplayInformation:
			aRemConOp = ERemConCoreApiDisplayInformation;
			break;
		case AVCPanel::EHelp:
			aRemConOp = ERemConCoreApiHelp;
			break;
		case AVCPanel::EPageUp:
			aRemConOp = ERemConCoreApiPageUp;
			break;
		case AVCPanel::EPageDown:
			aRemConOp = ERemConCoreApiPageDown;
			break;
		case AVCPanel::EPower:
			aRemConOp = ERemConCoreApiPower;
			break;
		case AVCPanel::EVolumeUp:
			aRemConOp = ERemConCoreApiVolumeUp;
			break;
		case AVCPanel::EVolumeDown:
			aRemConOp = ERemConCoreApiVolumeDown;
			break;
		case AVCPanel::EMute:
			aRemConOp = ERemConCoreApiMute;
			break;
		case AVCPanel::EPlay:
			aRemConOp = ERemConCoreApiPlay;
			break;
		case AVCPanel::EStop:
			aRemConOp = ERemConCoreApiStop;
			break;
		case AVCPanel::EPause:
			aRemConOp = ERemConCoreApiPause;
			break;
		case AVCPanel::ERecord:
			aRemConOp = ERemConCoreApiRecord;
			break;
		case AVCPanel::ERewind:
			aRemConOp = ERemConCoreApiRewind;
			break;
		case AVCPanel::EFastForward:
			aRemConOp = ERemConCoreApiFastForward;
			break;
		case AVCPanel::EEject:
			aRemConOp = ERemConCoreApiEject;
			break;
		case AVCPanel::EForward:
			aRemConOp = ERemConCoreApiForward;
			break;
		case AVCPanel::EBackward:
			aRemConOp = ERemConCoreApiBackward;
			break;
		case AVCPanel::EAngle:
			aRemConOp = ERemConCoreApiAngle;
			break;
		case AVCPanel::ESubpicture:
			aRemConOp = ERemConCoreApiSubpicture;
			break;
		case AVCPanel::EF1:
			aRemConOp = ERemConCoreApiF1;
			break;
		case AVCPanel::EF2:
			aRemConOp = ERemConCoreApiF2;
			break;
		case AVCPanel::EF3:
			aRemConOp = ERemConCoreApiF3;
			break;
		case AVCPanel::EF4:
			aRemConOp = ERemConCoreApiF4;
			break;
		case AVCPanel::EF5:
			aRemConOp = ERemConCoreApiF5;
			break;
		case AVCPanel::EVendorUnique:
		default:
			err = KErrNotSupported;
		}
		
	return err;
	}
	

TInt CControlCommand::ParseVendorUniquePassthroughCommand(MRemConBearerObserver& aObserver)
	{
	TInt err = KErrNone;
	TRAP(err, SetVendorInfoL(ETrue)); // set id and payload; leaves if not enough space available
	
	if (err == KErrNone && iVendorId == KBluetoothSIGVendorId)
		{
		// it's one of the v1.3 (or later!) MT commands
		err = ParseMetadataTransferPassthroughCommand();
		}
	else
		{
		iCommandData.Close();
		TRAP(err, iCommandData.CreateL(KAVCFrameMaxLength));
		if(err == KErrNone)
			{
			TRemConMessageType message = ERemConCommand;
			err = aObserver.BearerToInterface(TUid::Uid(KRemConBearerAvrcpImplementationUid),
											  iFrame->Data(),
											  iFrame->Data(),
											  iInterfaceUid,
											  iOperationId,
											  message,
											  iCommandData);
			}
		}

	return err;
	}
	
/** Translates from RemCon's core interface to an AVC operation id.

@param aRemConOp	The RemCon operation id within the core interface.
@param aAvrcpOp		On return the AVC passthrough operation id.
@return KErrNone	If the operation has been translated successfully.
@return KErrNotSupported	If the operation does not correspond to one
							provided by AVRCP.
*/
TInt CControlCommand::RemConToAvrcpOperation(TUint aRemConOp, AVCPanel::TOperationId& aAvrcpOp)
	{
	LOG_STATIC_FUNC
	TInt err = KErrNone;
	switch(aRemConOp)	
		{
		case ERemConCoreApiSelect:
			aAvrcpOp = AVCPanel::ESelect;
			break;
		case ERemConCoreApiUp:
			aAvrcpOp = AVCPanel::EUp;
			break;
		case ERemConCoreApiDown:
			aAvrcpOp = AVCPanel::EDown;
			break;
		case ERemConCoreApiLeft:
			aAvrcpOp = AVCPanel::ELeft;
			break;
		case ERemConCoreApiRight:
			aAvrcpOp = AVCPanel::ERight;
			break;
		case ERemConCoreApiRightUp:
			aAvrcpOp = AVCPanel::ERightUp;
			break;
		case ERemConCoreApiRightDown:
			aAvrcpOp = AVCPanel::ERightDown;
			break;
		case ERemConCoreApiLeftUp:
			aAvrcpOp = AVCPanel::ELeftUp;
			break;
		case ERemConCoreApiLeftDown:
			aAvrcpOp = AVCPanel::ELeftDown;
			break;
		case ERemConCoreApiRootMenu:
			aAvrcpOp = AVCPanel::ERootMenu;
			break;
		case ERemConCoreApiSetupMenu:
			aAvrcpOp = AVCPanel::ESetupMenu;
			break;
		case ERemConCoreApiContentsMenu:
			aAvrcpOp = AVCPanel::EContentsMenu;
			break;
		case ERemConCoreApiFavoriteMenu:
			aAvrcpOp = AVCPanel::EFavoriteMenu;
			break;
		case ERemConCoreApiExit:
			aAvrcpOp = AVCPanel::EExit;
			break;
		case ERemConCoreApi0:
			aAvrcpOp = AVCPanel::E0;
			break;
		case ERemConCoreApi1:
			aAvrcpOp = AVCPanel::E1;
			break;
		case ERemConCoreApi2:
			aAvrcpOp = AVCPanel::E2;
			break;
		case ERemConCoreApi3:
			aAvrcpOp = AVCPanel::E3;
			break;
		case ERemConCoreApi4:
			aAvrcpOp = AVCPanel::E4;
			break;
		case ERemConCoreApi5:
			aAvrcpOp = AVCPanel::E5;
			break;
		case ERemConCoreApi6:
			aAvrcpOp = AVCPanel::E6;
			break;
		case ERemConCoreApi7:
			aAvrcpOp = AVCPanel::E7;
			break;
		case ERemConCoreApi8:
			aAvrcpOp = AVCPanel::E8;
			break;
		case ERemConCoreApi9:
			aAvrcpOp = AVCPanel::E9;
			break;
		case ERemConCoreApiDot:
			aAvrcpOp = AVCPanel::EDot;
			break;
		case ERemConCoreApiEnter:
			aAvrcpOp = AVCPanel::EEnter;
			break;
		case ERemConCoreApiClear:
			aAvrcpOp = AVCPanel::EClear;
			break;
		case ERemConCoreApiChannelUp:
			aAvrcpOp = AVCPanel::EChannelUp;
			break;
		case ERemConCoreApiChannelDown:
			aAvrcpOp = AVCPanel::EChannelDown;
			break;
		case ERemConCoreApiPreviousChannel:
			aAvrcpOp = AVCPanel::EPreviousChannel;
			break;
		case ERemConCoreApiSoundSelect:
			aAvrcpOp = AVCPanel::ESoundSelect;
			break;
		case ERemConCoreApiInputSelect:
			aAvrcpOp = AVCPanel::EInputSelect;
			break;
		case ERemConCoreApiDisplayInformation:
			aAvrcpOp = AVCPanel::EDisplayInformation;
			break;
		case ERemConCoreApiHelp:
			aAvrcpOp = AVCPanel::EHelp;
			break;
		case ERemConCoreApiPageUp:
			aAvrcpOp = AVCPanel::EPageUp;
			break;
		case ERemConCoreApiPageDown:
			aAvrcpOp = AVCPanel::EPageDown;
			break;
		case ERemConCoreApiPower:
			aAvrcpOp = AVCPanel::EPower;
			break;
		case ERemConCoreApiVolumeUp:
			aAvrcpOp = AVCPanel::EVolumeUp;
			break;
		case ERemConCoreApiVolumeDown:
			aAvrcpOp = AVCPanel::EVolumeDown;
			break;
		case ERemConCoreApiMute:
			aAvrcpOp = AVCPanel::EMute;
			break;
		case ERemConCoreApiPlay:
			aAvrcpOp = AVCPanel::EPlay;
			break;
		case ERemConCoreApiStop:
			aAvrcpOp = AVCPanel::EStop;
			break;
		case ERemConCoreApiPause:
			aAvrcpOp = AVCPanel::EPause;
			break;
		case ERemConCoreApiRecord:
			aAvrcpOp = AVCPanel::ERecord;
			break;
		case ERemConCoreApiRewind:
			aAvrcpOp = AVCPanel::ERewind;
			break;
		case ERemConCoreApiFastForward:
			aAvrcpOp = AVCPanel::EFastForward;
			break;
		case ERemConCoreApiEject:
			aAvrcpOp = AVCPanel::EEject;
			break;
		case ERemConCoreApiForward:
			aAvrcpOp = AVCPanel::EForward;
			break;
		case ERemConCoreApiBackward:
			aAvrcpOp = AVCPanel::EBackward;
			break;
		case ERemConCoreApiAngle:
			aAvrcpOp = AVCPanel::EAngle;
			break;
		case ERemConCoreApiSubpicture:
			aAvrcpOp = AVCPanel::ESubpicture;
			break;
		case ERemConCoreApiF1:
			aAvrcpOp = AVCPanel::EF1;
			break;
		case ERemConCoreApiF2:
			aAvrcpOp = AVCPanel::EF2;
			break;
		case ERemConCoreApiF3:
			aAvrcpOp = AVCPanel::EF3;
			break;
		case ERemConCoreApiF4:
			aAvrcpOp = AVCPanel::EF4;
			break;
		case ERemConCoreApiF5:
			aAvrcpOp = AVCPanel::EF5;
			break;
		default:
			err = KErrNotSupported;
		}
	return err;
	}

TUint16 CControlCommand::Get16(const TPtrC8& aPtr)
	{
	return (aPtr[0]<<8) | aPtr[1];
	}

TInt CControlCommand::DummyCallback(TAny*)
	{
	// Should never be called- should be overwritten by a non-dummy callback 
	// before it's ever requested let alone called.
	AvrcpUtils::Panic(EAvrcpDummyCallbackCalled);
	return KErrNone;
	}

void CControlCommand::SetVendorInfoL(TBool aIsPassthrough)
	{
	if (aIsPassthrough)
		{
		if (iFrame->DataLength() < KAVCVendorUniquePassthroughHeader)
			{
			User::Leave(KErrCorrupt);
			}
		iVendorPayloadData.Set(CAVCVendorUniquePassthroughCommand::GetPayloadAndVID(*iFrame, iVendorId));
		}
	else
		{
		if (iFrame->DataLength() < KAVCVendorIdLength)
			{
			User::Leave(KErrCorrupt);
			}
		iVendorPayloadData.Set(CAVCVendorDependentCommand::GetPayloadAndVID(*iFrame, iVendorId));
		}
	}

TBool CControlCommand::IsAvrcpPassthrough() const
	{
	TBool isAvrcpPassthrough = EFalse;
	
	if(iInterfaceUid.iUid == KRemConCoreApiUid || iInterfaceUid.iUid == KRemConGroupNavigationApiUid)
		{
		isAvrcpPassthrough = ETrue;
		}
	
	return isAvrcpPassthrough;
	}

TBool CControlCommand::IsPassthrough() const
	{
	return ((iFrame->Opcode() == AVC::EPassThrough) && (iFrame->SubunitType() == AVC::EPanel));
	}

TBool CControlCommand::PlayerSpecificNotify() const
	{
	TRegisterNotificationEvent eventId = RAvrcpIPC::GetEventIdFromIPCOperationId(iOperationId);
	TMetadataTransferPDU pduId = RAvrcpIPC::GetPDUIdFromIPCOperationId(iOperationId);

	if(pduId != ERegisterNotification)
		{
		return EFalse;
		}
	
	if(iInterfaceUid == TUid::Uid(KRemConPlayerInformationUid))
		{
		if((eventId == ERegisterNotificationPlaybackStatusChanged) ||
		  (eventId == ERegisterNotificationTrackChanged) ||
		  (eventId == ERegisterNotificationTrackReachedEnd) ||
		  (eventId == ERegisterNotificationTrackReachedStart) ||
		  (eventId == ERegisterNotificationPlaybackPosChanged) ||
		  (eventId == ERegisterNotificationPlayerApplicationSettingChanged))
			{
			return ETrue;
			}
		}
	else if(iInterfaceUid == TUid::Uid(KRemConNowPlayingApiUid))
		{
		if (eventId == ERegisterNotificationNowPlayingContentChanged)
			{
			return ETrue;
			}
		}
	else 
		{
		return EFalse;
		}
	return EFalse;
	}

TBool CControlCommand::NormalCommand()
	{
	TBool ret = ETrue;
	TRegisterNotificationEvent eventId = RAvrcpIPC::GetEventIdFromIPCOperationId(iOperationId);
	TMetadataTransferPDU pduId = RAvrcpIPC::GetPDUIdFromIPCOperationId(iOperationId);
	
	if((eventId == ERegisterNotificationAvailablePlayersChanged) ||
	    (eventId == ERegisterNotificationAddressedPlayerChanged))
		{
		ret = EFalse;
		}
	return ret;
	}
/**
@return Ownership of a CControlCommand representing an interim response to this command
 */
CControlCommand* CControlCommand::InterimResponseL()
	{
	CAVCFrame* frame = CAVCFrame::NewL(iFrame->Data(), AVC::EResponse);
	CleanupStack::PushL(frame);
	frame->SetType(AVC::EInterim);
	
	CControlCommand* finalResponse = CControlCommand::NewL(frame, iRemConId,
			iTransactionLabel, iRemoteAddr, iClientId, iPlayerInfoManager);
	CleanupStack::Pop(frame);
	
	finalResponse->iInterfaceUid = iInterfaceUid;
	finalResponse->iOperationId = iOperationId;
	
	return finalResponse;
	}

