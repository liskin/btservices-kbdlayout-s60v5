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

#ifndef REMCONMEDIAATTRIBUTEID_H_
#define REMCONMEDIAATTRIBUTEID_H_

#include <e32base.h>

const TInt KMediaAttributeNum = 7;
enum TMediaAttributeId
	{
	ETitleOfMedia 			=0x1,
	ENameOfArtist			=0x2,
	ENameOfAlbum			=0x3,
	ETrackNumber			=0x4,		/* for example the CD track number */
	ENumberOfTracks			=0x5,		/* for example the total track number of the CD */
	EGenre					=0x6,
	EPlayingTime			=0x7
	};

/**
 * Provide an iterator wrapper class 
 */
class TMediaAttributeIter
	{
public:
	/** 
	@internalTechnology
	*/
	IMPORT_C TMediaAttributeIter(RArray<TMediaAttributeId>& aMediaAttributeIDs);
	IMPORT_C void Start();
	IMPORT_C TBool Next(TMediaAttributeId& aId);
	
private:
	RArray<TMediaAttributeId>& iMediaAttributeIDs;
	TInt iterator;
	};

#endif /*REMCONMEDIAATTRIBUTEID_H_*/
