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

#include <remcondatabaseunawarenowplayingbrowse.h>

/** 
Called by the client in response to a MrcdunpboGetFolderListing() call.

@param aFolderListing A listing of the items that are 
       in the range requested by the MrcdunpboGetFolderListing() call. If the
       media player does not natively support a Now Playing folder the listing
       will only contain one item which is the currently playing media element

@param aResult The result of the MrcdunpboGetFolderListing() operation.
     - KErrNone if the operation was successful.
     - KErrMediaBrowseInvalidOffset if there is no media available at the 
       offset of the start item, that is the start of range provided via 
       MrcnptoGetFolderListing() is not valid.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareNowPlayingBrowse::MrcdunpbFolderListing(
		const TArray<TRemConItem>& aFolderListing,  
		TUint aResult)
	{
	MrcdunpbDoFolderListing(aFolderListing, aResult);
	}

/**
Called by the client in response to MrcdunpboGetItem() call 

@param aMediaID The media element UID.
@param aMediaName The media name. Takes a copy of it.
@param aMediaType The media type.
@param aAttributes The attributes list. Takes a copy of it.
@param aResult The result of the MrcdunpboGetItem() operation.
     - KErrNone if the operation was successful.
     - KErrInvalidUid if the uid of the item requested is invalid, 
       e.g. the client removed the item before the request 
       MrcdunpboGetItem() coming.
     - System wide error code otherwise.
*/
EXPORT_C void MRemConDatabaseUnawareNowPlayingBrowse::MrcdunpbMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	MrcdunpbDoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}
