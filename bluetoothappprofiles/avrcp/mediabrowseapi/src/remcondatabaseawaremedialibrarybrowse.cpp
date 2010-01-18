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

#include <remcondatabaseawaremedialibrarybrowse.h>

/** 
Called by the client in response to a MrcdamlboGetFolderListing() call.

@param aFolderListing A listing of the items that are 
       in the range requested by the MrcdamlboGetFolderListing() call.

@param aMediaLibraryStateCookie The current value of the state cookie.

@param aResult The result of the MrcdamlboGetFolderListing() operation.
     - KErrNone if the operation was successful.
     - KErrMediaBrowseInvalidOffset if there is no media available at the 
       offset of the start item, i.e. the start of range provided via 
       MrcdamlboGetFolderListing() is not valid.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint16 aMediaLibraryStateCookie, 
		TUint aResult)
	{
	MrcdamlbDoFolderListing(aFolderListing, aMediaLibraryStateCookie, aResult);
	}

/** 
Called by the client in response to a MrcdamlboFolderUp call.

@param aItemCount The number of items present in this folder.
@param aResult The result of the MrcdamlboFolderUp operation.  
     - KErrNone if the operation was successful.
     - KErrInvalidMediaLibraryStateCookie if the media library state cookie 
       provided does not match the client's state cookie
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbFolderUpResult(
		TUint aItemCount, TInt aResult)
	{
	MrcdamlbDoFolderUpResult(aItemCount, aResult);
	}

/** 
Called by the client in response to a MrcdamlboFolderDown() call.

@param aItemCount The number of items present in this folder.

@param aResult The result of the MrcdamlboFolderDown() operation
     - KErrNone if the operation was successful.
     - KErrInvalidMediaLibraryStateCookie if the media library state cookie 
       provided does not match the client's state cookie
     - KErrMediaBrowseInvalidDirection if the folder provided is not a valid 
       folder e.g. path A/B/C, the current dirctory is B, and the param 
       provided with MrcdamlboFolderDown()is A, obviously you can not change 
       folder down to A, so A is invalid.
     - KErrMediaBrowseNotADirectory if the UID provided does not refer to a 
       folder item, e.g. it is a media element.
     - KErrInvalidUid if the UID provided is invalid.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbFolderDownResult(
		TUint aItemCount, TInt aResult)
	{
	MrcdamlbDoFolderDownResult(aItemCount, aResult);
	}

/** 
Called by the client in response to a MrcdamlboGetPath() call.

@param aItemCount The number of items present in this folder.

@param aMediaLibraryStateCookie The current value of the state cookie.

@param aResult The result of the MrcdamlboGetPath() operation
     - KErrNone if the operation is successful.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbGetPathResult(
		TUint aItemCount, 
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	MrcdamlbDoGetPathResult(aItemCount, aMediaLibraryStateCookie, aResult);
	}

/** 
Called by the client in response to a MrcdamlboSearch() call to indicate
that a search has completed.
 
@param aNumberItemsFound The number of items found by the search.
@param aMediaLibraryStateCookie The current of the state cookie.
@param aResult The result of the search.  
     - KErrNone if the search completed successfully
     - System wide error otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbSearchResult(
		TUint aNumberItemsFound,
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	MrcdamlbDoSearchResult(aNumberItemsFound, aMediaLibraryStateCookie, aResult);
	}

/** 
Called by the client in the case that the state of the media library has
changed.  This means that the media library state cookie provided prior to 
this call by the client may no longer be valid.  Typical causes for this 
would be items added or removed from the media library.
 
This function shall be called by any client any time it believes that media 
library state cookie has ceased to be valid, for example if the current 
directory is on removable media, and that media has been removed.  
That includes clients that are only able to offer state cookie that are 
limited to one directory.  
 
@param aMediaLibraryStateCookie The current value of the state cookie which 
       enables state mismatches between the client and the remote controller 
       device to be detected.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbMediaLibraryStateChange(
		TUint16 aMediaLibraryStateCookie)
	{
	MrcdamlbDoMediaLibraryStateChange(aMediaLibraryStateCookie);
	}

/**
Called by the client in response to MrcdamlboGetItem() call in the case the
requested item is a folder item.

@param aFolderID The folder UID.
@param aFolderName The folder name. Takes a copy of it.
@param aFolderType The folder type.
@param aPlayable Indicaties whether the folder is playable
     - KFolderNotPlayable the folder can not be played
     - KFolderPlayable the folder can be played
@param aAttributes The attributes of the folder item requested by 
       MrcdamlboGetItem()
@param aResult The result of the MrcdamlboGetItem() operation.
     - KErrNone if the operation was successful.
     - KErrInvalidMediaLibraryStateCookie if the media library state cookie 
       provided does not match the client's state cookie.
     - KErrInvalidUid if the uid of the item requested is invalid, 
       e.g. the client removed the item before the request 
       MrcdamlboGetItem() coming.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbFolderItemResult(
		const TRemConItemUid& aFolderID,
		const TDesC8& aFolderName, 
		TFolderItemType aFolderType, 
		TFolderItemPlayable aPlayable,
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	MrcdamlbDoFolderItemResult(aFolderID, aFolderName, aFolderType, aPlayable, aAttributes, aResult);
	}

/**
Called by the client in response to MrcdamlboGetItem() call in the case the
requested item is a media element item.

@param aMediaID The media element UID.
@param aMediaName The media name. Takes a copy of it.
@param aMediaType The media type.
@param aAttributes The attributes list. Takes a copy of it.
@param aResult The result of the MrcdamlboGetItem() operation.
     - KErrNone if the operation was successful.
     - KErrInvalidMediaLibraryStateCookie if the media library state cookie 
       provided does not match the client's state cookie.
     - KErrInvalidUid if the uid of the item requested is invalid, 
       e.g. the client removed the item before the request 
       MrcdamlboGetItem() coming.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareMediaLibraryBrowse::MrcdamlbMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	MrcdamlbDoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes,aResult);
	}
