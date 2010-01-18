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
 @internalTechnology
 @released
*/

#ifndef NOWPLAYING_H
#define NOWPLAYING_H

#include <e32base.h>
#include <remconmediabrowsetypes.h>
#include "avrcpipc.h"

//The UID identifying the RemCon NowPlaying interface.
const TInt KRemConNowPlayingApiUid = 0x10285bbc;

/**
This is the maximum length of a request that can be received
Play Item(0x74) and AddToNowPlaying(0x90) have the largest possible request
which is Scope (1 byte) + UID (8 bytes) + UID Counter (2 bytes) 
the total of which is 11
*/
const TInt KMaxLengthNowPlayingMsg = 11;


class RRemConNowPlayingRequest : public RAvrcpIPC
	{
public:
	TRemConFolderScope iScope;
	TUint64 iElement;
	TUint16 iUidCounter;
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

class RRemConNowPlayingResponse : public RAvrcpIPC
	{
public:
	TUint8 iStatus;
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

#endif //NOWPLAYING_H
