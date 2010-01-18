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



/**
 @file
 @internalTechnology
 @released
*/

#include "remconqueuemessage.h"
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_BEARER);
#endif

EXPORT_C CRemConQueuedMessage* CRemConQueuedMessage::NewL(TUid aInterfaceUid
		,const TDesC8& aData
		,TInt aOperationId
		)
	{
	LOG_STATIC_FUNC
	CRemConQueuedMessage* self = new(ELeave) CRemConQueuedMessage(
			aInterfaceUid
			,aOperationId
			);
	
	CleanupStack::PushL(self);
	self->ConstructL(aData);
	CleanupStack::Pop(self);
	return self;
	}

CRemConQueuedMessage::CRemConQueuedMessage(TUid aInterfaceUid
		,TInt aOperationId
		)
: iInterfaceUid(aInterfaceUid), iOperationId(aOperationId)
	{
	LOG_FUNC	
	}

void CRemConQueuedMessage::ConstructL(const TDesC8& aData)
	{
	iData.CreateL(aData);
	}

EXPORT_C CRemConQueuedMessage::~CRemConQueuedMessage()
	{
	LOG_FUNC
	iData.Close();
	}

EXPORT_C const TDesC8& CRemConQueuedMessage::Data()
	{
	return iData;
	}

EXPORT_C const CRemConQueuedMessage* TRemConMessageQueue::Find(
		TUid aInterfaceUid
		,TInt aOperationId
		)
	{
	CRemConQueuedMessage* request = NULL;
	while (iIter)
		{
		request = iIter++;
		if(request->iInterfaceUid == aInterfaceUid
		   && request->iOperationId == aOperationId
		   )
			{
			return request;
			}
		}
				
	return NULL;
	}

EXPORT_C TRemConMessageQueue::TRemConMessageQueue()
    :iQueue(_FOFF(CRemConQueuedMessage, iLink))
    , iIter(iQueue)
	{
	
	}

EXPORT_C void TRemConMessageQueue::Reset()
	{
	CRemConQueuedMessage* msg;
	iIter.SetToFirst();
	while (NULL != (msg = iIter++))
		{
		iQueue.Remove(*msg);
		delete msg;
		}
	}

EXPORT_C void TRemConMessageQueue::AddLast(CRemConQueuedMessage& aMsg)
	{
	iQueue.AddLast(aMsg);
	}

EXPORT_C TBool TRemConMessageQueue::IsEmpty()
	{
	return iQueue.IsEmpty();
	}

EXPORT_C CRemConQueuedMessage* TRemConMessageQueue::First() const
    {
    return iQueue.First();
    }

EXPORT_C void TRemConMessageQueue::Remove(CRemConQueuedMessage& aMsg)
	{
	iQueue.Remove(aMsg);
	}
