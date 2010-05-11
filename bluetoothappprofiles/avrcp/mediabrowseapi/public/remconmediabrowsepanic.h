// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef REMCONMEDIABROWSEPANIC_H
#define REMCONMEDIABROWSEPANIC_H

#include <e32base.h>

_LIT(KMediaBrowsePanicName, "RcMediaBrowse");

enum TRemConMediaBrowsePanic
	{
	EFolderListingProvidedTwice = 0,
	EFolderItemResultWithoutRequest = 1,
	EInvalidFolderType = 2,
	EInvalidPlayableValue = 3,
	EInvalidMediaType = 4,
	EMediaElementItemResultWithoutRequest = 5,
	ESearchResultWithoutRequest = 6,
	/** The value of the media library state cookie must not be zero. */
	EZeroMediaLibraryStateCookie = 7,
	EUnexpectedNullMemberField = 8,
	};

class MediaBrowsePanic
	{
public:
	static void Panic(TRemConMediaBrowsePanic aPanic);
	};

#endif //REMCONMEDIABROWSEPANIC_H
