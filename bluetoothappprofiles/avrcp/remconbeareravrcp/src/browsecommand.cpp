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
// avrcpcommands.cpp
//



/**
 @file
 @internalComponent
 @released
*/

#include <e32base.h>
#include <remcon/remconbearerobserver.h>
#include <remcon/remconconverterplugin.h>
#include <remcon/messagetype.h>
#include <remconbeareravrcp.h>

#include "browsecommand.h"
#include "avrcpinternalinterface.h"
#include "avrcplog.h"
#include "avrcputils.h"
#include "browsingframe.h"
#include "mediabrowse.h"
#include "nowplaying.h"
							

//---------------------------------------------------------------------
// Incoming command construction
//---------------------------------------------------------------------

/** Factory function.

@param aFrame The frame this command is to represent.
@param aRemConId The RemCon transaction label.
@param aTransLabel The AVCTP transaction label.
@param aAddr The bluetooth address to send this command to.
@return A fully constructed CBrowseCommand.
@leave System wide error codes.
*/
CBrowseCommand* CBrowseCommand::NewL(const TDesC8& aMessageInformation, 
	TUint aRemConId, 
	SymbianAvctp::TTransactionLabel aTransLabel, 
	const TBTDevAddr& aAddr,
	CAvrcpPlayerInfoManager* aPlayerInfoManager)
	{
	LOG_STATIC_FUNC
	CBrowseCommand* command = new(ELeave)CBrowseCommand(aRemConId, aTransLabel, aAddr, aPlayerInfoManager);
	CleanupStack::PushL(command);
	command->ConstructL(aMessageInformation);
	CleanupStack::Pop(command);
	return command;
	}

/** Constructor.

@param aFrame The AV/C frame this command is to represent.
@param aRemConId The RemCon transaction label.
@param aTransLabel The AVCTP transaction label.
@param aAddr The bluetooth address to send this command to.
@return A partially constructed CBrowseCommand.
@leave System wide error codes.
*/	
CBrowseCommand::CBrowseCommand(TUint aRemConId, 
	SymbianAvctp::TTransactionLabel aTransLabel, 
	const TBTDevAddr& aAddr,
	CAvrcpPlayerInfoManager* aPlayerInfoManager)
	: CAvrcpCommand(aRemConId, aTransLabel, aAddr)
	{
	LOG_FUNC
	iPlayerInfoManager = aPlayerInfoManager;
	}

/** Destructor.
*/
CBrowseCommand::~CBrowseCommand()
	{
	LOG_FUNC
	__ASSERT_DEBUG(iUsers == 0, AvrcpUtils::Panic(EAvrcpCommandStillInUse));
	iFrame.Close();
	iCommandData.Close();
	}

/** Second phase construction.
*/
void CBrowseCommand::ConstructL(const TDesC8& aMessageInformation)
	{
	iFrame.CreateL(aMessageInformation);
	}

//------------------------------------------------------------------------------------
// Called by router
//------------------------------------------------------------------------------------

SymbianAvctp::TMessageType CBrowseCommand::MessageType() const
	{
	return SymbianAvctp::EResponse;
	}
	
//------------------------------------------------------------------------------------
// Called by handlers
//------------------------------------------------------------------------------------
	
/** Fills in command info from iFrame.

This must be called by the command handler before processing this
command.

This functions sets iInterfaceUid, iOperationId and iCommandData
to the correct values according to iFrame.  The format of iCommandData
is defined by RemCon and is dependent on iInterfaceUid and iOperationId.

@return KErrNone				If the frame has been parsed successfully.
@return KErrNotSupported		This frame represents a command for which a
								RemCon converter or client side interface
								cannot be found.
@return KErrAvrcpInvalidCType	The CType specified in this frame is invalid.
@return KErrCorrupt	        	If the frame is corrupted(e.g invalid Operation Id).
@return	System wide error code.
*/
TInt CBrowseCommand::ProcessIncomingCommandL(TInt aMaxResponse)
	{
	LOG_FUNC
	TInt result = KErrNotSupported;
	iMaxResponse = aMaxResponse-5;

	// This command has already been verified as containing at least a header
	// and an accurate length field
	switch(AvrcpBrowsing::BrowsingFrame::PduId(iFrame)) // PDU ID
		{
	case AvrcpBrowsing::ESetBrowsedPlayer:
		{
		result = HandleSetBrowsedPlayer();
		break;
		}
	case AvrcpBrowsing::EGetFolderItems:
		{
		result = HandleGetFolderItems();
		break;
		}
	case AvrcpBrowsing::EChangePath:
		{
		result = HandleChangePath();
		break;
		}
	case AvrcpBrowsing::EGetItemAttributes:
		{
		result = HandleGetItemAttributes();
		break;
		}
	case AvrcpBrowsing::ESearch:
		{
		result = HandleSearch();
		break;
		}
	case AvrcpBrowsing::EGeneralReject:
		// We only support the target role for browsing, so we should
		// never receive a General Reject
		// fallthrough
	default:
		{
		result = HandleUnknownPdu();
		}
		};
	return result;
	}
	

/** Processes an outgoing response.

This should only be called for vendor dependent commands as
we respond to passthrough commands internally.

@param aFrame The command data for the response.
*/
void CBrowseCommand::ProcessOutgoingResponse(RBuf8& aCommandData)
	{
	__ASSERT_DEBUG(( (iInterfaceUid == TUid::Uid(KRemConMediaBrowseApiUid)) 
				|| (iInterfaceUid == TUid::Uid(KRemConNowPlayingApiUid))
				|| (iInterfaceUid == TUid::Uid(KUidAvrcpInternalInterface))),
				AvrcpUtils::Panic(EAvrcpResponseToUnknownCommand));
	
	iFrame.Close();
	iFrame.Assign(aCommandData);
	TInt length = aCommandData.Length() - 3;
	iFrame[1] = length >> 8;
	iFrame[2] = length;
	aCommandData.Assign(NULL);
	}

/** Set the response type in the AV/C frame.

@param aErr The result of processing the operation. KErrNone if 
			successful. KErrNotsupported if this operation is not
			implemented, eg because no converter was found.
*/
void CBrowseCommand::SetResult(TInt aErr)
	{
	LOG_FUNC
	switch(aErr)
		{
		case KErrNone:
			break;
		case KErrCorrupt:
		case EAvrcpResponseToUnknownCommand:
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
		case KErrAvrcpInvalidScope:
			{
			// If this fails we can't send the error response - just give up
			TRAPD(err, GenerateRejectPayloadL(aErr));
			err = err; // Avoid warning about not using it.
			break;
			}
		}
	}

/** Gets this command's frame.
@return the browse frame for this command
*/	
const TDesC8& CBrowseCommand::Data() const
	{
	LOG_FUNC
	return iFrame;
	}

const TDesC8& CBrowseCommand::CommandData() const
	{
	LOG_FUNC
	return iCommandData;
	}

void CBrowseCommand::GenerateRejectPayloadL(TInt aErr)
	{
	LOG_FUNC;
	
	TUint8 pduId = AvrcpBrowsing::BrowsingFrame::PduId(iFrame);
	RRemConMediaErrorResponse errResponse;		
	errResponse.iPduId = pduId;
	errResponse.iStatus = RAvrcpIPC::SymbianErrToStatus(aErr);
	
	RBuf8 frame;
	frame.CreateL(KBrowseResponseBaseLength);
	CleanupClosePushL(frame);
	errResponse.WriteL(frame);
	CleanupStack::Pop(&frame);
	
	iFrame.Close();
	iFrame.Assign(frame);
	}


