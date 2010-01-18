// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef PLAYEREVENTSUTILS_H
#define PLAYEREVENTSUTILS_H

_LIT(KPlayerEventsPanicName, "PlayerEvents");

enum TPlayerEventsPanic
	{
	ETwoGetPlayStatusUpdatesQueued = 0,
	EBadlyFormattedInternalData = 1,
	};

class PlayerEventsUtils
	{
public:
	static void Panic(TPlayerEventsPanic aPanic); 
	};

/** Utility PlayerEvents panic function.

@param aPanic The panic number.
*/
void PlayerEventsUtils::Panic(TPlayerEventsPanic aPanic)
	{
	User::Panic(KPlayerEventsPanicName, aPanic);
	}

#endif // PLAYEREVENTSUTILS_H
