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

#include <bt_sock.h>
#include <e32base.h>
#include <e32msgqueue.h>
#include <remconaddress.h>
#include <remconbeareravrcp.h>
#include "avrcplog.h"
#include "avrcputils.h"

/**
@file
@internalComponent
@released
*/

/** Utility AVRCP panic function.

@param aPanic The panic number.
*/
void AvrcpUtils::Panic(TAvrcpPanic aPanic)
	{
	User::Panic(KAvrcpPanicName, aPanic);
	}
	

/** Set the command data.  This overwrites the current
contents of the data buffer.

@param aCommandData The buffer in which to set the data.
@param aOffset The offset within aCommandData to set the data.
@param aLength The length of data to replace.
@param aValue The new value for the replaced data.
*/
void AvrcpUtils::SetCommandDataFromInt(RBuf8& aCommandData, 
	TInt aOffset, TInt aLength, TInt aValue)
	{
	LOG_STATIC_FUNC
	__ASSERT_DEBUG(aLength <= 4, Panic(EAvrcpCommandDataTooLong));
	
	for(TInt i = 0; i < aLength; i++)
		{
		aCommandData[aOffset+i] = aValue >> (8*i);
		}
	}

/** Reads command data from the buffer to an int.

@param aCommandData The buffer from which to read the data.
@param aOffset The offset within aCommandData read from.
@param aLength The length of data to read.  This must not be
				more than 4.
@param aValue On return, the value of the specified data section.
*/	
void AvrcpUtils::ReadCommandDataToInt(const RBuf8& aCommandData, 
	TInt aOffset, TInt aLength, TInt& aValue)
	{
	LOG_STATIC_FUNC
	__ASSERT_DEBUG(aLength <= 4, Panic(EAvrcpCommandDataTooLong));
	
	aValue = 0;
	
	for(TInt i = 0 ; i < aLength; i++)
		{
		aValue |= aCommandData[aOffset+i]<<(8*i);
		}
	}

/** Convert from a RemCon address to a bluetooth device address.

@param aRemoteAddress The RemCon format address to convert.
@param aBTAddr On return, the bluetooth device address.
@return Whether the conversion could be performed successfully.
*/	
TInt AvrcpUtils::RemConToBTAddr(const TRemConAddress& aRemoteAddress, TBTDevAddr& aBTAddr)
	{
	LOG_STATIC_FUNC
	TInt err = KErrArgument;
	
	// Check client has provided us a valid address
	if(aRemoteAddress.Addr().Length() == KBTDevAddrSize)
		{
		aBTAddr = TBTDevAddr(aRemoteAddress.Addr());
		err = KErrNone;
		}
	else
		{
		__ASSERT_DEBUG(EFalse, AvrcpUtils::Panic(EAvrcpBadBTAddr));
		}
		
	return err;
	}

/** Convert from a bluetooth device address to a RemCon address.

We assume this cannot fail, as bluetooth addresses are generated
internally rather than by a client, so they should always be
valid, and so convertible.

@param aBTAddr The bluetooth device address to convert.
@param aRemoteAddress On return, the RemCon format address.
*/	
void AvrcpUtils::BTToRemConAddr(const TBTDevAddr& aBTAddr, TRemConAddress& aRemConAddress)
	{
	LOG_STATIC_FUNC
	aRemConAddress.Addr() = aBTAddr.Des();
	aRemConAddress.BearerUid() = TUid::Uid(KRemConBearerAvrcpImplementationUid);
	}

NONSHARABLE_CLASS(CSpecificThreadCallBackBody)
	: public CActive
	{
public:
	static CSpecificThreadCallBackBody* NewL(const TCallBack& aCallBack, TInt aPriority);
	~CSpecificThreadCallBackBody();
	
	TInt Start();
	TInt CallBack();
	void HandleCancel();
	
private:
	CSpecificThreadCallBackBody(const TCallBack& aCallBack, TInt aPriority);
	void ConstructL();
	
	TInt AsyncMessage(TInt aParam);
	
private: // from CActive
	void RunL();
	void DoCancel();
	
private:
	TCallBack		iCallBack;
	
	RThread			iLocalThread;
	
	RMsgQueue<TInt>	iInbound;
	RMsgQueue<TInt>	iOutbound;
	};

RSpecificThreadCallBack::RSpecificThreadCallBack()
	: iBody(NULL)
	{
	LOG_FUNC
	}

TInt RSpecificThreadCallBack::Create(const TCallBack& aCallBack, TInt aPriority)
	{
	TRAPD(err, iBody = CSpecificThreadCallBackBody::NewL(aCallBack, aPriority));
	return err;
	}

void RSpecificThreadCallBack::Close()
	{
	LOG_FUNC
	delete iBody;
	iBody = NULL;
	}

TInt RSpecificThreadCallBack::Start()
	{
	return iBody->Start();
	}

TInt RSpecificThreadCallBack::CallBack()
	{
	return iBody->CallBack();
	}

void RSpecificThreadCallBack::Cancel()
	{
	return iBody->HandleCancel();
	}

CSpecificThreadCallBackBody* CSpecificThreadCallBackBody::NewL(const TCallBack& aCallBack, TInt aPriority)
	{
	CSpecificThreadCallBackBody* self = new(ELeave) CSpecificThreadCallBackBody(aCallBack, aPriority);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CSpecificThreadCallBackBody::CSpecificThreadCallBackBody(const TCallBack& aCallBack, TInt aPriority)
	: CActive(aPriority)
	, iCallBack(aCallBack)
	{
	LOG_FUNC
	}

void CSpecificThreadCallBackBody::ConstructL()
	{
	User::LeaveIfError(iInbound.CreateLocal(1));
	User::LeaveIfError(iOutbound.CreateLocal(1));
	}

CSpecificThreadCallBackBody::~CSpecificThreadCallBackBody()
	{
	LOG_FUNC
	HandleCancel();
	iInbound.Close();
	iOutbound.Close();
	iLocalThread.Close();
	}

TInt CSpecificThreadCallBackBody::Start()
	{
	TInt err = KErrNone;
	if(!IsAdded())
		{
		err = iLocalThread.Duplicate(RThread());
		if(err == KErrNone)
			{
			CActiveScheduler::Add(this);
			iInbound.NotifyDataAvailable(iStatus);
			SetActive();
			}
		}
	return err;
	}

TInt CSpecificThreadCallBackBody::CallBack()
	{
	TInt err = KErrUnknown;
	if(iLocalThread.Id() == RThread().Id())
		{
		// Simple synchronous case.
		err = iCallBack.CallBack();
		}
	else
		{
		RThread thisThread;
		err = thisThread.Duplicate(RThread());
		if(err == KErrNone)
			{
			err = AsyncMessage(thisThread.Handle());
			}
		}
	return err;
	}
	
TInt CSpecificThreadCallBackBody::AsyncMessage(TInt aParam)
	{
	TInt err = KErrNone;
	TRequestStatus logonStatus;
	iLocalThread.Logon(logonStatus);
	if(logonStatus == KErrNoMemory)
		{
		// This seems kludgy, but I think it is the most reliable way.
		User::WaitForRequest(logonStatus); // Ensure the all requests are correct...
		err = KErrNoMemory;
		}
	else
		{
		iInbound.SendBlocking(aParam);
		TRequestStatus status;
		iOutbound.NotifyDataAvailable(status);
		User::WaitForRequest(status, logonStatus);
		if(status == KRequestPending)
			{
			// Remote thread is dead
			iOutbound.CancelDataAvailable();
			User::WaitForRequest(status);
			err = KErrDied;
			}
		else
			{
			// Success (the thread may have subsequently died, but we are only concerned with this call).
			iLocalThread.LogonCancel(logonStatus);
			User::WaitForRequest(logonStatus);
			err = status.Int();
			if(err == KErrNone)
				{
				iOutbound.ReceiveBlocking(err);
				}
			}
		}
	return err;
	}


void CSpecificThreadCallBackBody::RunL()
	{
	TInt threadHandle;
	iInbound.ReceiveBlocking(threadHandle);
	if(threadHandle == 0)
		{
		// 0 is a cancel message
		// therefore don't do anything
		iOutbound.SendBlocking(KErrNone);
		}
	else
		{
		RThread remoteThread;
		remoteThread.SetHandleNC(threadHandle);
		
		TInt result = iCallBack.CallBack();
		
		// There doesn't seem to be a safe way of handling when the other thread
		// dies......
		iOutbound.SendBlocking(result);
		
		remoteThread.Close();
		
		iInbound.NotifyDataAvailable(iStatus);
		SetActive();
		}
	}

void CSpecificThreadCallBackBody::DoCancel()
	{
	if(RThread().Id() == iLocalThread.Id())
		{
		iInbound.CancelDataAvailable();
		}
	else
		{
		// other thread cancelling - so just complete the
		// request
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrCancel);
		}
	}

void CSpecificThreadCallBackBody::HandleCancel()
	{
	if(IsAdded())
		{
		if(RThread().Id() == iLocalThread.Id())
			{
			Cancel(); // synchronous cancel is fine in same thread...
			}
		else
			{
			// In a different thread - this is more interesting...
			TInt err = AsyncMessage(0); // 0 is special as it means cancel.
			if(err == KErrDied && IsActive())
				{
				// Remote thread has already died so we need to tidy up the
				// active object ourselves.
				Cancel();
				}
			}
		}
	// else shouldn't be active...
	}


