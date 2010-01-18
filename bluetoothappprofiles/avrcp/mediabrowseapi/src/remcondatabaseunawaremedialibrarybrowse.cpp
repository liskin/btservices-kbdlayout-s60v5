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

#include <remcondatabaseunawaremedialibrarybrowse.h>

/** 
Called by the client in response to a MrcdumlboGetFolderListing() call.

@param aFolderListing A listing of the items that are 
       in the range requested by the MrcdumlboGetFolderListing() call.

@param aResult The result of the MrcdumlboGetFolderListing() operation.
     - KErrNone if the operation was successful.
     - KErrMediaBrowseInvalidOffset if there is no media available at the 
       offset of the start item, i.e. the start of range provided via 
       MrcdumlboGetFolderListing() is not valid.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint aResult)
	{
	MrcdumlbDoFolderListing(aFolderListing, aResult);
	}

/** 
Called by the client in response to a MrcdumlboFolderUp call.

@param aItemCount The number of items present in this folder.
@param aResult The result of the MrcdumlboFolderUp operation.  
     - KErrNone if the operation was successful.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderUpResult(
		TUint aItemCount, TInt aResult)
	{
	MrcdumlbDoFolderUpResult(aItemCount, aResult);
	}

/** 
Called by the client in response to a MrcdumlboFolderDown() call.

@param aItemCount The number of items present in this folder.

@param aResult The result of the MrcdumlboFolderDown() operation
     - KErrNone if the operation was successful.
     - KErrMediaBrowseInvalidDirection if the folder provided is not a valid 
       folder e.g. path A/B/C, the current dirctory is B, and the param 
       provided with MrcdumlboFolderDown()is A, obviously you can not change 
       folder down to A, so A is invalid.
     - KErrMediaBrowseNotADirectory if the UID provided does not refer to a 
       folder item, e.g. it is a media element.
     - KErrInvalidUid if the UID provided is invalid.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderDownResult(
		TUint aItemCount, TInt aResult)
	{
	MrcdumlbDoFolderDownResult(aItemCount, aResult);
	}

/** 
Called by the client in response to a MrcdumlboGetPath() call.

@param aItemCount The number of items present in this folder.

@param aResult The result of the MrcdumlboGetPath() operation
     - KErrNone if the operation is successful.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbGetPathResult(
		TUint aItemCount, 
		TInt aResult)
	{
	MrcdumlbDoGetPathResult(aItemCount, aResult);
	}

/** 
Called by the client in response to a MrcdumlboSearch() call to indicate
that a search has completed.
 
@param aNumberItemsFound The number of items found by the search.
@param aResult The result of the search.  
     - KErrNone if the search completed successfully
     - System wide error otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbSearchResult(
		TUint aNumberItemsFound,
		TInt aResult)
	{
	MrcdumlbDoSearchResult(aNumberItemsFound, aResult);
	}

/** 
Called by the client to indicate changes to the media database in the client.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbMediaLibraryStateChange()
	{
	MrcdumlbDoMediaLibraryStateChange();
	}

/**
Called by the client in response to MrcdumlboGetItem() call in the case the
requested item is a folder item.

@param aFolderID The folder UID.
@param aFolderName The folder name. Takes a copy of it.
@param aFolderType The folder type.
@param aPlayable Indicaties whether the folder is playable
     - KFolderNotPlayable the folder can not be played
     - KFolderPlayable the folder can be played
@param aAttributes The attributes of the folder item requested by 
       MrcdumlboGetItem()
@param aResult The result of the MrcdumlboGetItem() operation.
     - KErrNone if the operation was successful.
     - KErrInvalidUid if the uid of the item requested is invalid, 
       e.g. the client removed the item before the request 
       MrcdumlboGetItem() coming.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbFolderItemResult(
		const TRemConItemUid& aFolderID,
		const TDesC8& aFolderName, 
		TFolderItemType aFolderType, 
		TFolderItemPlayable aPlayable,
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	MrcdumlbDoFolderItemResult(aFolderID, aFolderName, aFolderType, aPlayable, aAttributes, aResult);
	}

/**
Called by the client in response to MrcdumlboGetItem() call in the case the
requested item is a media element item.

@param aMediaID The media element UID.
@param aMediaName The media name. Takes a copy of it.
@param aMediaType The media type.
@param aAttributes The attributes list. Takes a copy of it.
@param aResult The result of the MrcdumlboGetItem() operation.
     - KErrNone if the operation was successful.
     - KErrInvalidUid if the uid of the item requested is invalid, 
       e.g. the client removed the item before the request 
       MrcdumlboGetItem() coming.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareMediaLibraryBrowse::MrcdumlbMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	MrcdumlbDoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}

