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

#include "avrcpcommand.h"
#include "avrcplog.h"

CAvrcpCommand::CAvrcpCommand(TUint aRemConId,
	SymbianAvctp::TTransactionLabel aTransactionLabel, 
	const TBTDevAddr& aAddr)
	: iRemConId(aRemConId)
	, iTransactionLabel(aTransactionLabel)
	, iRemoteAddr(aAddr)
	{
	LOG_FUNC
	}

//------------------------------------------------------------------------------------
// Called by handlers, router and bearer
//------------------------------------------------------------------------------------

/** Decrement the users of this command.

This must be called by users of the command when they
no longer require the command to remain in existence.
When the number of users reaches zero, the command
is deleted.
*/
void CAvrcpCommand::DecrementUsers()
	{
	LOG_FUNC
	if(--iUsers < 1)
		{
		iHandlingLink.Deque();
		delete this;
		}
	}

/** Increment the users of this command.

This must be called by users of the command to signify
they require it to remain in existence.
*/
void CAvrcpCommand::IncrementUsers()
	{
	LOG_FUNC
	iUsers++;
	}

//------------------------------------------------------------------------------------
// Called by bearer
//------------------------------------------------------------------------------------

/** Get info needed by RemCon for this command.

@param aInterfaceUid On return, the RemCon interface of this command.
@param aId On return, the RemCon transaction id of this command.
@param aOperationId On return, the RemCon operation id of this command.
@param aCommandData On return, the command data for this operation. 
					Ownership is returned.
@param aAddr On return, the originating device for this command.
*/
void CAvrcpCommand::GetCommandInfo(TUid& aInterfaceUid, 
		TUint& aId, 
		TUint& aOperationId, 
		RBuf8& aCommandData, 
		TBTDevAddr& aAddr)
	{
	LOG_FUNC
	aInterfaceUid = iInterfaceUid;
	aId = iRemConId;
	aOperationId = iOperationId;
	
	// RemCon will take ownership of command data
	aCommandData.Assign(iCommandData);
	iCommandData.Assign(NULL);
	
	aAddr = iRemoteAddr;
	}

//------------------------------------------------------------------------------------
// Called by handlers
//------------------------------------------------------------------------------------

/** Gets the remote associated with this command.
This may be the source or the destination, depending
on if the originated locally or remotely.

@return The BT address of the remote device.
*/
const TBTDevAddr& CAvrcpCommand::RemoteAddress() const
	{
	LOG_FUNC
	return iRemoteAddr;
	}	

/** Gets the AVCTP transaction label for this command.
This is used to match commands with responses.

@return The AVCTP transaction label for this command.
*/
SymbianAvctp::TTransactionLabel CAvrcpCommand::TransactionLabel() const
	{
	LOG_FUNC
	return iTransactionLabel;
	}

/** Gets the RemCon id of this command.
This is used to uniquely identify a transaction to RemCon.

@return The RemCon command id.
*/	
TUint CAvrcpCommand::RemConCommandId() const
	{
	LOG_FUNC
	return iRemConId;
	}

/** Gets the RemCon interface uid of this command.

This is used with the operation id to uniquely identify an operation to RemCon.

@param aOperation
@return The RemCon operation id.
*/	
const TUid& CAvrcpCommand::RemConInterfaceUid() const
	{
	LOG_FUNC
	return iInterfaceUid;
	}

/** Gets the RemCon operation id of this command.

This is used with the interface uid to uniquely identify an operation to RemCon.

@param aOperation
@return The RemCon operation id.
*/	
TUint CAvrcpCommand::RemConOperationId() const
	{
	LOG_FUNC
	return iOperationId;
	}


/**
Whether the Bearer is aware of this command.  If not we shouldn't
inform the bearer of responses.
*/
TBool CAvrcpCommand::KnownToBearer() const
	{
	return iKnownToBearer;
	}
