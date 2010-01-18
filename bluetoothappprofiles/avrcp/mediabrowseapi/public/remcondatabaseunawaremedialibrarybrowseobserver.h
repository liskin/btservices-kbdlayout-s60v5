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

#ifndef REMCONDATABASEUNAWAREMEDIALIBRARYBROWSEOBSERVER_H
#define REMCONDATABASEUNAWAREMEDIALIBRARYBROWSEOBSERVER_H

NONSHARABLE_CLASS(MRemConDatabaseUnawareMediaLibraryBrowseObserver)
	{
public:
	/**
	This requests a list of the items present in a folder.  No metadata is 
	requested, that can be retrieved separately on a per item basis.  
	
	The result should be returned via a call to
	MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderListing(). And 
	the ordering should be as shown locally, but should list folders before
	media element items.
	
	@param aScope The scope in which the folder listing is made. 
	       This indicates which listing should be returned. If the scope is 
	       EBrowseFolder then the contents of the current browsed 
	       folder should be returned. If it is the ESearchResultFolder scope 
	       then the contents of the folder populated by the last search 
	       should be returned.
	
	@param aStartItem The offset into the folder listing of the first item to 
	       retrieve. If there is no media available at this offset the error 
	       KErrMediaBrowseInvalidOffset should be returned via the result call
	       to MrcdumlbFolderListing().
	
	@param aEndItem The offset into the folder listing of the final item to
	       retrieve. If this is beyond the end of the folder listing then the 
	       items from the aStartItem offset to the end of the listing should 
	       be returned via the result call to MrcdumlbFolderListing().
	*/
	virtual void MrcdumlboGetFolderListing(TRemConFolderScope aScope,
			TUint aStartItem, 
			TUint aEndItem) = 0;
	
	/**
	Requests a specific item which may be a folder item or a media element 
	item. 
	
	If it returns KErrNone the client must return the metadata for the 
	requested item through the response function either
	MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderItemResult() 
	or 
	MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbMediaElementItemResult()
	according to the item type which is a folder item or media element item.
	If it is unable to process this call then it can return error but not
	call the response function.
	
	@param aScope The scope in which the item is present.  If the scope is 
	       EBrowseFolder then the item is present in the media library's 
	       virtual filesystem. If it is the ESearchResultFolder scope then the
	       item is present in the current set of search results.
	
	@param aItemId The UID for the requested Item.
	
	@param aIter An iterator for TMediaAttributeIds containing a list of 
	       TMediaAttributeIds requested by the controller. It is intended for
	       the media element item, so omit it if the item is a folder item.

	@return The call back result.
	      - KErrNone If success.
	      - The appropriate error code.
    */
	virtual TInt MrcdumlboGetItem(TRemConFolderScope aScope,
			const TRemConItemUid& aItemId, 
			TMediaAttributeIter& aIter) = 0;
	
	/**
	This requests that the current position within the folder heirarchy is 
	moved to the next folder up.
	
	The result of this operation must be returned via a call to 
	MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderUpResult().
	*/
	virtual void MrcdumlboFolderUp() = 0;
	
	/**
	This requests that the current position within the folder hierarchy is 
	moved to the child folder of the current folder specified by aFolder.
	 
	The result of this operation must be returned via a call to 
	MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderDownResult().
	 
	@param aFolder The child folder to change into.
	*/
	virtual void MrcdumlboFolderDown(const TRemConItemUid& aFolder) = 0;
	
	/**
	This requests the current path. 
	
	The client is responsible for appending items to the array then call 
	MrcdumlbGetPathResult() when completed. Once the item is appended in the 
	array the ownership of the item is taken so the client should keep the 
	item valid and never destroy the item. If the client can append the items 
	successfully then it must return a result via MrcdumlbGetPathResult(). 
	If the client appends some items to the array but then fails to append all
	the items then it must return an error via MrcdumlbGetPathResult().
	
	@see MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbGetPathResult()
	 
	@param aPath The current path. 
	       Each entry in the array in one level of the path.  For example the 
	       path /A/B/C would be provided as:
	       aPath[0] 'A'
	       aPath[1] 'B'
	       aPath[2] 'C'
	       All string are in UTF-8.
	*/
	virtual void MrcdumlboGetPath(RPointerArray<HBufC8>& aPath) = 0;
	
	/**
	Perform a search in the current location for items 
	matching the search string.
	
	@param aSearch The string to search on in UTF-8. The client can use it until 
	         the client has called MrcdumlbSearchResult().
	
	@see MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbSearchResult()
	*/
	virtual void MrcdumlboSearch(const TDesC8& aSearch) = 0;
	};

#endif //REMCONDATABASEUNAWAREMEDIALIBRARYBROWSEOBSERVER_H
