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
 @publishedAll
 @released
*/

#ifndef REMCONNOWPLAYINGTARGETBASE_H
#define REMCONNOWPLAYINGTARGETBASE_H

#include <e32base.h>
#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>
#include <remconmediabrowsetypes.h>

class CRemConInterfaceSelector;
class MRemConDatabaseAwareNowPlayingTargetObserver;
class MRemConDatabaseUnawareNowPlayingTargetObserver;
class TRemConMessageQueue;

/**
A parent class for CRemConDatabaseAwareNowPlayingTarget and 
CRemConDatabaseUnawareNowPlayingTarget.
 
This class is not intended for instantiation. Clients should instead use the
appropriate derived class - either CRemConDatabaseAwareNowPlayingTarget or
CRemConDatabaseUnawareNowPlayingTarget.

This class is only to be used by classes currently derived from it.

@see CRemConDatabaseAwareNowPlayingTarget
@see CRemConDatabaseUnawareNowPlayingTarget
*/
NONSHARABLE_CLASS(CRemConNowPlayingTargetBase) : public CRemConInterfaceBase,
	                                             public MRemConInterfaceIf2
	{
public:
	IMPORT_C void PlayItemResponse(TInt aErr);
	IMPORT_C void AddToNowPlayingResponse(TInt aErr);
	IMPORT_C void NowPlayingContentChanged();
	
protected:
	IMPORT_C virtual ~CRemConNowPlayingTargetBase();
	
	CRemConNowPlayingTargetBase(CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseAwareNowPlayingTargetObserver& aObserver);
	
	CRemConNowPlayingTargetBase(CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseUnawareNowPlayingTargetObserver& aObserver);

	virtual void BaseConstructL();

private: //From CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);

private: //From MRemConInterfaceIf2
	void MrcibNewMessage(TUint aOperationId,
			const TDesC8& aData,
			TRemConMessageSubType aMsgSubType);

private: //Utility functions
	void SendResponse(TUint8 aStatus, TUint aOperationId);
	void SendError(TInt aError, TUint aOperationId);	
	void SendNotificationResponse(TRemConMessageSubType aMsgSubType);
	
	void ProcessGetStatus();
	void ProcessGetStatusAndBeginObserving();	
	void ProcessPlayItem(const TDesC8& aData);
	void ProcessAddToNowPlaying(const TDesC8& aData);
	
	TBool ProcessNowPlaying(const TDesC8& aData,
			TUint aOperationId,
			TUint64& aItem, 
			TRemConFolderScope& aScope, 
			TUint16& aMediaLibraryStateCookie);

	static TInt NextMessageCb(TAny* aThis);
	void DoNextMessage();
	void AddToOperationQueue(TInt aOperationId, const TDesC8& aData);
		
private: //Unowned
	MRemConDatabaseAwareNowPlayingTargetObserver*	iDaObserver;
	MRemConDatabaseUnawareNowPlayingTargetObserver*	iDuObserver;
	CRemConInterfaceSelector&                   	iInterfaceSelector;

private: //Owned
	TBool                                       iNotificationPending;
	RBuf8                                       iOutBuf;

    TBool                                       iInProgress;
    TRemConMessageQueue*                         iMsgQueue;
    CAsyncCallBack*                             iNextMessageCallBack;
	};

#endif //REMCONNOWPLAYINGTARGETBASE_H
