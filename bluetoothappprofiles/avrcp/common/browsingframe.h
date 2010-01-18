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
// browisngframe.h
//

#ifndef BROWSINGFRAME_H
#define BROWSINGFRAME_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>

const TInt KUidCounter = 0;

namespace AvrcpBrowsing
	{
const TInt KHeaderLength = 3;
typedef TUint8 TPduId;	
typedef TUint8 TScope;
typedef TUint8 TDirection;
typedef TUint8 TPlayable;
typedef TUint8 TMediaType;

const TScope KMediaPlayerScope = 0x0;
const TScope KVirtualFilesystemScope = 0x1;
const TScope KSearchScope = 0x2;
const TScope KNowPlayingScope = 0x3;

const TDirection KUp = 0x00;
const TDirection KDown = 0x01;

const TPlayable KPlayable = 1;
const TPlayable KNotPlayable = 0;
const TPlayable KPlayableReserved = 2;

const TMediaType KAudio = 0;
const TMediaType KVideo = 1;
const TMediaType KMediaTypeReserved = 2;

enum TPdu
	{
	ESetBrowsedPlayer = 0x70,
	EGetFolderItems = 0x71,
	EChangePath = 0x72,
	EGetItemAttributes = 0x73, 
	ESearch = 0x80,
	EGeneralReject = 0xa0,
	};

enum TItemType
	{
	EMediaPlayer = 0x01,
	EFolderItem = 0x02,
	EMediaElement = 0x03,
	};

enum TFolderType
	{
	EMixed = 0,
	ETitles = 1,
	EAlbums = 2,
	EArtists = 3,
	EGenres = 4,
	EPlaylists = 5,
	EYears = 6,
	EFolderTypeReserved = 7,
	};

class BrowsingFrame
	{
public:
	static void VerifyFrameL(const TDesC8& aFrame);
	static TPduId PduId(const TDesC8& aFrame);
	static TInt ParamLength(const TDesC8& aFrame);
	static void Payload(const TDesC8& aFrame, TPtrC8& aPayload);
	};

	} // end namespace BrowsingFrame

#endif //BROWSINGFRAME_H
