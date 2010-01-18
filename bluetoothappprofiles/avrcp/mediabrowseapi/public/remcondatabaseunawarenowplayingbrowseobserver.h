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

#ifndef REMCONDATABASEUNAWARENOWPLAYINGBROWSEOBSERVER_H
#define REMCONDATABASEUNAWARENOWPLAYINGBROWSEOBSERVER_H

NONSHARABLE_CLASS(MRemConDatabaseUnawareNowPlayingBrowseObserver)
	{
public:
	/**
	Requests a listing of the items present in the Now Playing folder. If the 
	media player does not natively support a Now Plyaing folder the client
	shall return a listing of one item which is the currently playing media 
	element.
	
	No metadata is requested, that can be retrieved separately on a per item 
	basis. Trying this out for now as it fits in with the existing metadata 
	attribute collection, and means that we would also separate out the 
	listing from the metadata. This means that we can retrieve a NowPlaying 
	list and then request the metadata from the observer.
	
	The result should be returned via a call to 
	MRemConDatabaseUnawareNowPlayingBrowse::MrcdunpbFolderListing().
	
	@param aStartItem The offset into the folder listing of the first item to 
	       retrieve. If there is no media available at this offset the error 
	       KErrMediaBrowseInvalidOffset should be returned via the result call
	       to MrcdunpbFolderListing().
	
	@param aEndItem The offset into the folder listing of the final item to 
	       retrieve. If this is beyond the end of the folder listing then the 
	       items from the aStartItem offset to the end of the listing should 
	       be returned via the result call to MrcdunpbFolderListing().
	*/
	virtual void MrcdunpboGetFolderListing(TUint aStartItem, 
			TUint aEndItem) = 0;
	
	/**
	Requests a set of metadata corresponding to a specific media element item. 
	 
	The client must return the metadata for the requested item via using 
	MRemConDatabaseUnawareNowPlayingBrowse::MrcdunpbMediaElementItemResult()
	if it returns KErrNone, if it is unable to process this call then it can
	return error but not call the response function 
	MRemConDatabaseUnawareNowPlayingBrowse::MrcdunpbMediaElementItemResult(). 
	
	@param aItemId The UID for the requested media element item.
	
	@param aIter An iterator for TMediaAttributeIds containing a list of 
	       TMediaAttributeIds requested by the controller.
			    
	@return The call back result.
	      - KErrNone If success.
	      - The appropriate error code.
	*/
	virtual TInt MrcdunpboGetItem(const TRemConItemUid& aItemId, 
			TMediaAttributeIter& aIter) = 0;
	};

#endif //REMCONDATABASEUNAWARENOWPLAYINGBROWSEOBSERVER_H
