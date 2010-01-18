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

#ifndef REMCONMEDIABROWSETYPES_H
#define REMCONMEDIABROWSETYPES_H

#include <e32base.h>
#include <remconmediaattributeid.h>

/**
Defines a type representing a UID which is 8 octet and uniquely identifys 
a folder or media element in the media library.
*/
typedef TUint64 TRemConItemUid;

/**
Defines item types to indicate the item is a folder item or media item.
*/
enum TRemConItemType
	{
	ERemConFolderItem = 2,
	ERemConMediaItem = 3,
	};

class TRemConItem
	{
public:
	TRemConItemUid iUid;
	TRemConItemType iType;
	};

/**
Defines a type which indicates whether a folder item is playable.
*/
typedef TUint8 TFolderItemPlayable;

/**
Defines a type to indicate a media item type.
*/
typedef TUint8 TMediaItemType;

const TFolderItemPlayable KFolderNotPlayable = 0;
const TFolderItemPlayable KFolderPlayable = 1;

const TMediaItemType KMediaAudio = 0;
const TMediaItemType KMediaVideo = 1;

enum TFolderItemType
	{
	EFolderMixed = 0,
	EFolderTitles = 1,
	EFolderAlbums = 2,
	EFolderArtists = 3,
	EFolderGenres = 4,
	EFolderPlaylists = 5,
	EFolderYears = 6,
	};

/**
Defines folder scopes in which media content navigaition  may take place.
*/
enum TRemConFolderScope
	{
	/**
	Applicable in browsed player.
	Valid browseable items are foler item and media element item.
	*/
	EBrowseFolder         = 0x1,
	
	/**
	Applicable in browsed player.
	Valid browseable items are media element items.
	*/
	ESearchResultFolder   = 0x2,
	
	/**
	Applicable in addressed player.
	Valid browseable items are media element items.
	*/
	ENowPlayingFolder     = 0x3
	};

/**
A data container of a media element attribute.

*/
class TMediaElementAttribute
	{
public:
	TMediaAttributeId     iAttributeId;
	
	/**
	The value of the attribute in UTF-8.
	*/
	HBufC8*               iString;
	};

#endif //REMCONMEDIABROWSETYPES_H
