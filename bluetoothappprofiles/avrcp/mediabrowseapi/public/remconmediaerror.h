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

#ifndef REMCONMEDIAERROR_H_
#define REMCONMEDIAERROR_H_

#include <e32base.h>

const TInt KErrRemConMediaBase                     = -6780;

/**
Invalid media library state cookie.

This error only occurs in the database aware clients.
*/
const TInt KErrInvalidMediaLibraryStateCookie     = KErrRemConMediaBase - 0x5;

/**
The direction parameter is invalid

@see MrcdamlbFolderDownResult()
@see MrcdumlbFolderDownResult()
*/
const TInt KErrMediaBrowseInvalidDirection        = KErrRemConMediaBase - 0x7;

/**
The UID provided does not refer to a folder item

@see MrcdamlbFolderDownResult()
@see MrcdumlbFolderDownResult()
*/
const TInt KErrMediaBrowseNotADirectory           = KErrRemConMediaBase - 0x8;

/**
The UID provided does not refer to any currently valid item

@see MrcdamlbFolderDownResult()
@see MrcdumlbFolderDownResult()
@see MrcdamlbFolderItemResult()
@see MrcdumlbFolderItemResult()
@see MrcdamlbMediaElementItemResult()
@see MrcdumlbMediaElementItemResult()
@see MrcdanpbMediaElementItemResult()
@see MrcdunpbMediaElementItemResult()
*/
const TInt KErrInvalidUid                        = KErrRemConMediaBase - 0x9;

/**
The start of range provided is not valid

@see MrcdamlbFolderListing()
@see MrcdumlbFolderListing()
@see MrcdanpbFolderListing()
@see MrcdunpbFolderListing()
*/
const TInt KErrMediaBrowseInvalidOffset           = KErrRemConMediaBase - 0xb;

/**
The UID provided refers to a directory 
which cannot be handled by this media player.

*/
const TInt KErrNowPlayingUidIsADirectory          = KErrRemConMediaBase - 0xc;

/**
The media is not able to be used for this operation at this time

*/
const TInt KErrNowPlayingMediaInUse               = KErrRemConMediaBase - 0xd;

/**
No more items can be added to the now playing list because it is full

*/
const TInt KErrNowPlayingListFull                 = KErrRemConMediaBase - 0xe;

#endif /*REMCONMEDIAERROR_H_*/
