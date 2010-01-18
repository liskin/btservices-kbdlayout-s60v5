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

#include <remcondatabaseawarenowplayingbrowse.h>

/** 
Called by the client in response to a MrcdanpboGetFolderListing() call.

@param aFolderListing A listing of the items that are 
       in the range requested by the MrcdanpboGetFolderListing() call. If the
       media player does not natively support a Now Playing folder the listing
       will only contain one item which is the currently playing media element
       
@param aMediaLibraryStateCookie The current value of the state cookie.
@param aResult The result of the MrcdanpboGetFolderListing() operation.
     - KErrNone if the operation was successful.
     - KErrMediaBrowseInvalidOffset if there is no media available at the 
       offset of the start item, i.e. the start of range provided via 
       MrcdanpboGetFolderListing() is not valid. 
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareNowPlayingBrowse::MrcdanpbFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint16 aMediaLibraryStateCookie, 
		TUint aResult)
	{
	MrcdanpbDoFolderListing(aFolderListing, aMediaLibraryStateCookie, aResult);
	}

/**
Called by the client in response to MrcdanpboGetItem() call 

@param aMediaID The media element UID.
@param aMediaName The media name. Takes a copy of it.
@param aMediaType The media type.
@param aAttributes The attributes list. Takes a copy of it.
@param aResult The result of the MrcdanpboGetItem() operation.
     - KErrNone if the operation was successful.
     - KErrInvalidMediaLibraryStateCookie if the media library state cookie 
       does not match the client's state cookie.
     - KErrInvalidUid if the uid of the item requested is invalid, 
       e.g. the client removed the item before the request 
       MrcdanpboGetItem() coming.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseAwareNowPlayingBrowse::MrcdanpbMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	MrcdanpbDoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}
