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

#include <e32base.h>
#include <remconmediabrowsetypes.h>

#ifndef REMCONDATABASEUNAWARENOWPLAYINGBROWSE_H
#define REMCONDATABASEUNAWARENOWPLAYINGBROWSE_H

NONSHARABLE_CLASS(MRemConDatabaseUnawareNowPlayingBrowse)
	{
public:
	IMPORT_C void MrcdunpbFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint aResult);
			
	IMPORT_C void MrcdunpbMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
private:
	virtual void MrcdunpbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint aResult) = 0;
		
	virtual void MrcdunpbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult) = 0;
	};

#endif //REMCONDATABASEUNAWARENOWPLAYINGBROWSE_H

