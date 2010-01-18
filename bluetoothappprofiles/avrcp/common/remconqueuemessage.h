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

#ifndef REMCONQUEUEMESSAGE_H_
#define REMCONQUEUEMESSAGE_H_

#include <e32base.h>

class CRemConQueuedMessage;

/**
A queue for RemCon request command messages. 
*/
NONSHARABLE_CLASS(TRemConMessageQueue)
	{
public:
	IMPORT_C TRemConMessageQueue();
	IMPORT_C void Reset();
	IMPORT_C void AddLast(CRemConQueuedMessage& aMsg);
	IMPORT_C TBool IsEmpty();
	IMPORT_C CRemConQueuedMessage* First() const;
	IMPORT_C void Remove(CRemConQueuedMessage& aMsg);
	IMPORT_C const CRemConQueuedMessage* Find(TUid aInterfaceUid
				,TInt aOperationId
				);
private:
	TSglQue<CRemConQueuedMessage> iQueue;
	TSglQueIter<CRemConQueuedMessage> iIter;
	};

/**
A data encapsulation class for a request.

The object contains the data of the request and the type of the request, which
will be put in a queue.
*/
NONSHARABLE_CLASS(CRemConQueuedMessage) : public CBase
	{
public:
	IMPORT_C static CRemConQueuedMessage* NewL(TUid aInterfaceUid
			,const TDesC8& aData
			,TInt aOperationId
			);
	
	IMPORT_C ~CRemConQueuedMessage();	
	IMPORT_C const TDesC8& Data();
	
private:
	CRemConQueuedMessage(TUid aInterfaceUid, TInt aOperationId);
	void ConstructL(const TDesC8& aData);
	
public:
	TSglQueLink                       iLink;
	TUid                              iInterfaceUid;
	TInt                              iOperationId;
	
private:
	RBuf8                             iData;
	};


#endif /*REMCONQUEUEMESSAGE_H_*/
