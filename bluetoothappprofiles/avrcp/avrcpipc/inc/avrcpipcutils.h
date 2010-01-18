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

#ifndef AVRCPIPCUTILS_H
#define AVRCPIPCUTILS_H

_LIT(KAvrcpIpcPanicName, "AVRCP IPC");

enum TAvrcpIpcPanic
	{
	EAvrcpIpcCommandDataTooLong = 0,
	};

class AvrcpIpcUtils
	{
public:
	static void Panic(TAvrcpIpcPanic aPanic); // The macro is probably more useful so this can be removed
	};

/** Utility AVRCP panic function.

@param aPanic The panic number.
*/
void AvrcpIpcUtils::Panic(TAvrcpIpcPanic aPanic)
	{
	User::Panic(KAvrcpIpcPanicName, aPanic);
	}

#endif //AVRCPIPCUTILS_H
