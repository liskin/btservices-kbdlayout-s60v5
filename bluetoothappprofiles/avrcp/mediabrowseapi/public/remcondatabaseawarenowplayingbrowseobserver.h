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

#ifndef REMCONDATABASEAWARENOWPLAYINGBROWSEOBSERVER_H
#define REMCONDATABASEAWARENOWPLAYINGBROWSEOBSERVER_H

NONSHARABLE_CLASS(MRemConDatabaseAwareNowPlayingBrowseObserver)
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
	
	The result shall be returned via a call to 
	MRemConDatabaseAwareNowPlayingBrowse::MrcdanpbFolderListing() 
	
	@param aStartItem The offset into the folder listing of the first item to 
	       retrieve. If there is no media available at this offset the error 
	       KErrMediaBrowseInvalidOffset shall be returned via the result call
	       to MrcdanpbFolderListing().
	
	@param aEndItem The offset into the folder listing of the final item to 
	       retrieve. If this is beyond the end of the folder listing then the 
	       items from the aStartItem offset to the end of the listing shall 
	       be returned via the result call to MrcdanpbFolderListing().
	*/
	virtual void MrcdanpboGetFolderListing(TUint aStartItem, 
			TUint aEndItem) = 0;
	
	/**
	Requests a set of metadata corresponding to a specific media element item. 
	 
	The client must return the metadata for the requested item via using 
	MRemConDatabaseAwareNowPlayingBrowse::MrcdanpbMediaElementItemResult() if
	it returns KErrNone, if it is unable to to process this call then it can
	return error but not call the response function
	MRemConDatabaseAwareNowPlayingBrowse::MrcdanpbMediaElementItemResult().
	
	@param aItemId The UID for the requested media element item.
	
	@param aIter An iterator for TMediaAttributeIds containing a list of 
	       TMediaAttributeIds requested by the controller.
			    
	@param aMediaLibraryStateCookie The cookie which the remote device 
	       believes refers to the current state of the media library. If this 
	       does not match the current state according to the client then the 
	       client should call MrcdanpbMediaElementItemResult() with the error 
	       KErrInvalidMediaLibraryStateCookie.
	       
	@return The call back result.
	      - KErrNone If success.
	      - The appropriate error code.
	*/
	virtual TInt MrcdanpboGetItem(const TRemConItemUid& aItemId, 
			TMediaAttributeIter& aIter, 
			TUint16 aMediaLibraryStateCookie) = 0;
	};

#endif //REMCONDATABASEAWARENOWPLAYINGBROWSEOBSERVER_H
