// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef AVRCP_H
#define AVRCP_H

/**
@file
@internalComponent
@released
*/

namespace AvrcpStatus
	{
	enum TCode
		{
		EInvalidCommand = 0,
		EInvalidParameter = 1,
		EParameterNotFound = 2,
		EInternalError = 3,
		ESuccess = 4,
		EUidChanged = 5,
		EReserved = 6,
		EInvalidDirection = 7,
		ENotADirectory = 8,
		EDoesNotExist = 9,
		EInvalidScope = 0xa,
		ERangeOutOfBounds = 0xb,
		EUidIsADirectory = 0xc,
		EMediaInUse = 0xd,
		ENowPlayingListFull = 0xe,
		ESearchNotSupported = 0xf,
		ESearchInProgress = 0x10,
		EInvalidPlayerId = 0x11,
		EPlayerNotBrowesable = 0x12,
		EPlayerNotAddressed = 0x13,
		ENoValidSearchResults = 0x14,
		ENoAvailablePlayers = 0x15,
		EAddressedPlayerChanged = 0x16,
		};
	}


#endif // AVRCP_H
