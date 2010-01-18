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
 @internalComponent
 @released
*/

#ifndef REMCONNOWPLAYINGFAULT_H_
#define REMCONNOWPLAYINGFAULT_H_

#include <e32base.h>

_LIT(KNowPlayingFaultName, "RcNpFault");

enum TRemConNowPlayingFault
	{
	EInvalidEventId = 0,
	};

class NowPlayingFault
	{
public:
	static void Fault(TRemConNowPlayingFault aFault);
	};

#endif /*REMCONNOWPLAYINGFAULT_H_*/
