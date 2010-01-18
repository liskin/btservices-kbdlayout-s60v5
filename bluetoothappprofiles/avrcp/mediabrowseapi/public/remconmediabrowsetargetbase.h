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

#ifndef REMCONMEDIABROWSETARGETBASE_H
#define REMCONMEDIABROWSETARGETBASE_H

#include <remcondatabaseawaremedialibrarybrowseobserver.h>
#include <remcondatabaseawarenowplayingbrowseobserver.h>
#include <remcondatabaseunawaremedialibrarybrowseobserver.h>
#include <remcondatabaseunawarenowplayingbrowseobserver.h>
#include <remconmediabrowsetypes.h>
#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>

class RRemConGetFolderItemsResponse;
class RRemConGetItemAttributesResponse;
class RRemConGetPathResponse;
class TRemConMessageQueue;
/**
This interface enables the client to present a heirarchical view
of the local media library for browsing by a remote entity.

The view is a virtual filesystem and does not need to correspond
directly to any structure on local storage.  It could for example
map onto a database of media content.

Each item in the media library is identified by a UID.  At a 
minimum this must uniquely identify an element within a folder.
For clients that support it the UID can be persistant accross
the entire virtual filesystem.  In this case a media library
state cookie is used to ensure that that state is consistent
between the client and the remote device. 

This interface does not have NewL()/NewLC, so the client should 
use the object CRemConDatabaseAwareMediaBrowseTarget or 
CRemConDatabaseUnawareMediaBrowseTarget to use its exported functions.

This class is only to be used by classes currently derived from it.

@see CRemConDatabaseAwareMediaBrowseTarget
@see CRemConDatabaseUnawareMediaBrowseTarget
*/
NONSHARABLE_CLASS(CRemConMediaBrowseTargetBase) : public CRemConInterfaceBase, public MRemConInterfaceIf
	{
protected:
	CRemConMediaBrowseTargetBase(CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConDatabaseAwareMediaLibraryBrowseObserver& aMlObserver, 
			MRemConDatabaseAwareNowPlayingBrowseObserver& aNpObserver,
			TUint16 aMediaLibraryStateCookie);

	CRemConMediaBrowseTargetBase(CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConDatabaseUnawareMediaLibraryBrowseObserver& aMlObserver, 
			MRemConDatabaseUnawareNowPlayingBrowseObserver& aNpObserver);
	
	virtual ~CRemConMediaBrowseTargetBase();
	
	virtual void BaseConstructL(TBool aSearchSupported);
	
	void DoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	void DoFolderUpResult(TUint aItemCount, TInt aResult);
	void DoFolderDownResult(TUint aItemCount, TInt aResult);
	
	void DoGetPathResult(TUint aItemCount, 
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	void DoSearchResult(TUint aNumberItemsFound,
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	void DoMediaLibraryStateChange(TUint16 aMediaLibraryStateCookie);
	
	void DoFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	void DoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
private: // From CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);

private: // From MRemConInterfaceIf
	void MrcibNewMessage(TUint aOperationId,
			const TDesC8& aData);
	
private: // Utility
	void ProcessGetFolderItems(const TDesC8& aData);
	void ProcessChangePath(const TDesC8& aData);
	void ProcessGetItemAttributes(const TDesC8& aData);
	void ProcessSearch(const TDesC8& aData);
	void ProcessGetPath(const TDesC8& aData);
	void ProcessMediaLibraryStateCookieUpdate(const TDesC8& aData);
	
	void SendGetItemAttributesResponse(TInt aResult, const TDesC8& aData);
	void SendGetFolderItemsResponse(TInt aResult, const TDesC8& aData);
	void SendChangePathResponse(TUint aItemCount, TInt aResult);
	void SendInternalNotificationResponse(TRemConMessageSubType aMsgSubType);
	void SendNotificationResponse(TRemConMessageSubType aMsgSubType);
	void SendSearchResponse(TInt aResult, TUint aNumberItemsFound, TUint16 aMediaLibraryStateCookie);
	void SendMediaLibraryStateCookieUpdateResponse(TUint16 aMediaLibraryStateCookie);
	
	static TInt NextMessageCb(TAny* aThis);
	void DoNextMessage();
	
	static TInt NextItemCallBack(TAny* aThis);
	void RequestNextItem();
	void DoItemComplete(TInt aResult);
	
	void SendError(TUint8 aPduId, 
			TUint aOperationId, 
			TInt aError);
	
	// Used for Gia
	TInt ItemAttributesResult(const TArray<TMediaElementAttribute>& aAttributes);
	TInt DoAttributeValue(TMediaAttributeId aAttributeId, const TDesC8& aAttributeData);
	void DoAllAttributesCompleted(TInt aResult);
	
	void AddToOperationQueue(TUint8 aPduId, 
			TInt aOperationId, 
			const TDesC8& aData);
	TInt ParseGetItemAttributesRequest(const TDesC8& aData,
			TRemConFolderScope& aScope,
			TRemConItemUid& aItemUid,
			TUint16& aMediaLibraryStateCookie);
	void ProcessMediaElementItemResult(const TRemConItemUid& aMediaID,
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	void ProcessFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			TInt aResult);
	void ProcessGetItemAttributesResult(
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);

	inline TBool DatabaseAware() const;

private:
	// Unowned
	CRemConInterfaceSelector& 							iInterfaceSelector;
	
	MRemConDatabaseAwareMediaLibraryBrowseObserver*		iRcdamlbo;
	MRemConDatabaseUnawareMediaLibraryBrowseObserver*	iRcdumlbo;
	
	MRemConDatabaseAwareNowPlayingBrowseObserver*		iRcdanpbo;
	MRemConDatabaseUnawareNowPlayingBrowseObserver*		iRcdunpbo;
	
	// Owned
	CAsyncCallBack*										iNextMessageCallBack;
	CAsyncCallBack*										iNextItemCallBack;
	
	RBuf8												iOutBuf;
	
	TUint16												iMediaLibraryStateCookie;
	
	TBool												iInProgress; // used internally to see if any operation is in progress
	TRemConMessageQueue*                     			iMsgQueue; // used to queue messages if another operation is in progress
	
	RArray<TMediaAttributeId>							iMediaAttributeIds; // used for Gfl and Gia to store requested attributes
	TMediaAttributeIter									iAttributeIterator; // provided to the client on requesting item attributes for Gfl and Gia
	
	RArray<TMediaAttributeId>							iNullArray; 
	TMediaAttributeIter									iNullIterator; // Used for getting a folder item to perform a folder listing
	
	TBool												iGetFolderListing; // used to check Gfl is in progress 
	RRemConGetFolderItemsResponse*						iGflResponse; // used to build up Gfl response from client
	TRemConFolderScope									iScope; // stored for use in async callbacks to get folder items

	TBool												iGetItemAttributes;	// used to check Gia is in progress
	TBool												iSetAttributeValue; // used to check that client has set response if it returns KErrNone
	RRemConGetItemAttributesResponse*					iGiaResponse; // used to build up Gia response from client
	
	TBool												iSearchSupported;
	TBool												iSearchInProgress; // used to check Search is in progress
	RBuf8												iSearchString; // used to store search string until request is complete
	
	RRemConGetPathResponse*      						iGetPathResponse; // used to build up get path response 
	
	TBool												iMlscUpdatePending;
	TUint16												iLastMlscUpdate;
	};

#endif // REMCONMEDIABROWSETARGETBASE_H

