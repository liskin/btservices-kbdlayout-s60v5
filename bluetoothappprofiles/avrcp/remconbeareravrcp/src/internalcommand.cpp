// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "internalcommand.h"
#include "avrcputils.h"
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_BEARER);
#endif

/** Factory function.

@param aFrame The frame this command is to represent.
@param aRemConId The RemCon transaction label.
@param aTransLabel The AVCTP transaction label.
@param aAddr The bluetooth address to send this command to.
@return A fully constructed CBrowseCommand.
@leave System wide error codes.
*/
CInternalCommand* CInternalCommand::NewL(TUid aInterfaceUid, 
		TUint aId, 
		TUint aOperationId,
		const TDesC8& aCommandData)
	{
	CInternalCommand* command = new(ELeave)CInternalCommand(aInterfaceUid, aId, aOperationId);
	CleanupStack::PushL(command);
	command->ConstructL(aCommandData);
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
CInternalCommand::CInternalCommand(TUid aInterfaceUid, 
		TUint aId, 
		TUint aOperationId)
	: CAvrcpCommand(aId, KDummyTransactionLabel, NULL)
	{
	LOG_FUNC
	iInterfaceUid = aInterfaceUid;
	iOperationId = aOperationId;
	}

/** Destructor.
*/
CInternalCommand::~CInternalCommand()
	{
	LOG_FUNC
	__ASSERT_DEBUG(iUsers == 0, AvrcpUtils::Panic(EAvrcpCommandStillInUse));
	iCommandData.Close();
	}

void CInternalCommand::ConstructL(const TDesC8& aCommandData)
	{
	iCommandData.CreateL(aCommandData);
	}

void CInternalCommand::ResetL(TUint& aId, const TDesC8& aCommandData)
	{
	iCommandData.Close();
	iCommandData.CreateL(aCommandData);
	iRemConId = aId;
	}

SymbianAvctp::TMessageType CInternalCommand::MessageType() const
	{
	// Should never be going out AVCTPwards
	__ASSERT_DEBUG(EFalse, AvrcpUtils::Panic(EAvctpMessageTypeRequestedForInternalCommand));
			
	return SymbianAvctp::EUndefined;
	}

const TDesC8& CInternalCommand::Data() const
	{
	return iCommandData;
	}
