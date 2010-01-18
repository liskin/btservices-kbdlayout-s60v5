// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedAll
 @released
*/

#ifndef REMCONMEDIAINFORMATIONTARGET_H
#define REMCONMEDIAINFORMATIONTARGET_H

#include <e32base.h>
#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>
#include <remconmediainformationtargetobserver.h>

class CRemConInterfaceSelector;
class RRemConGetElementAttributesResponse;

NONSHARABLE_CLASS(CRemConMediaInformationQueuedMessage): public CBase
	{
public:
	static CRemConMediaInformationQueuedMessage* NewL(const TDesC8& aData);
	~CRemConMediaInformationQueuedMessage();
	
	const TDesC8& Data();
private:
	CRemConMediaInformationQueuedMessage();
	void ConstructL(const TDesC8& aData);
public:
	TSglQueLink iLink;
private:
	RBuf8 iData;
	};

/**
Client-instantiable type supporting sending media information.
*/
NONSHARABLE_CLASS(CRemConMediaInformationTarget) : public CRemConInterfaceBase, 
								          		public MRemConInterfaceIf
	{
public:

	/**
	Factory function.
	@param aInterfaceSelector The interface selector. The client must have 
	created one of these first.
	@param aObserver The observer of this interface.
	@return A new CRemConMediaInformationTarget, owned by the interface selector.
	*/
	IMPORT_C static CRemConMediaInformationTarget* NewL(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConMediaInformationTargetObserver& aObserver);
	
	/** Destructor */
	IMPORT_C ~CRemConMediaInformationTarget();

public:
	/** Called by the client in response to a MrcmitoGetCurrentlyPlayingMetadata() call.
	Each call provides one of the attribute values requested.
	This is repeatedly called until all attributes requested have been supplied and then followed by Completed().
	@param aAttribute The ID of the attribute supplied.
	@param aValue A reference to a descriptor containing the attribute data.  
	This must be valid until Completed() has been called. 
	@see MRemConMediaInformationTargetObserver::MrcmitoGetCurrentlyPlayingMetadata()
	@see CRemConMediaInformationTarget::Completed()
	*/
	IMPORT_C TInt AttributeValue( TMediaAttributeId aAttributeId, TDesC8& aAttributeData);
	
	/** Signal that all attributes requested by MrcmitoGetCurrentlyPlayingMetadata() have been supplied
	using the AttributeValue() method.
	@see MRemConMediaInformationTargetObserver::MrcmitoGetCurrentlyPlayingMetadata()
	@see CRemConMediaInformationTarget::AttributeValue()
	*/
	IMPORT_C void Completed();
	
private:
	/** 
	Constructor.
	@param aInterfaceSelector The interface selector.
	@param aObserver The observer of this interface.
	*/
	CRemConMediaInformationTarget(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConMediaInformationTargetObserver& aObserver);

	void ConstructL();

private: // from CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);

private: // from MRemConInterfaceIf
	void MrcibNewMessage(TUint aOperationId, const TDesC8& aData);

private: // utility
	void SendError(TInt aError);
	void ProcessMessage(const TDesC8& aData);
	
	static int NextMessageCb(TAny* aThis);
	void DoNextMessage();
private: // unowned
	MRemConMediaInformationTargetObserver& iObserver;

private: // owned
	RRemConGetElementAttributesResponse* iResponse;

	RArray<TMediaAttributeId>  	iMediaAttributeIDs;
	TMediaAttributeIter 		iAttributeIterator;
	TBool						iInProgress;
	TSglQue<CRemConMediaInformationQueuedMessage> iMsgQueue;
	
	CAsyncCallBack* iNextMessageCallBack;
	};

#endif // REMCONMEDIAINFORMATIONTARGET_H
