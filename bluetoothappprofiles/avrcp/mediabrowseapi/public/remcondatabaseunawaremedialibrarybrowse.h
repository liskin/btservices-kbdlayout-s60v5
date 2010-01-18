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

#ifndef REMCONDATABASEUNAWAREMEDIALIBRARYBROWSE_H
#define REMCONDATABASEUNAWAREMEDIALIBRARYBROWSE_H

NONSHARABLE_CLASS(MRemConDatabaseUnawareMediaLibraryBrowse)
	{
public:
	IMPORT_C void MrcdumlbFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint aResult);
	
	IMPORT_C void MrcdumlbFolderUpResult(TUint aItemCount, TInt aResult);
	IMPORT_C void MrcdumlbFolderDownResult(TUint aItemCount, TInt aResult);
	
	IMPORT_C void MrcdumlbGetPathResult(TUint aItemCount, 
			TInt aResult);
	
	IMPORT_C void MrcdumlbSearchResult(TUint aNumberItemsFound,
			TInt aResult);
	
	IMPORT_C void MrcdumlbMediaLibraryStateChange();
	
	IMPORT_C void MrcdumlbFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	IMPORT_C void MrcdumlbMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
private:
	virtual void MrcdumlbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint aResult) = 0;
	
	virtual void MrcdumlbDoFolderUpResult(TUint aItemCount, TInt aResult) = 0;
	virtual void MrcdumlbDoFolderDownResult(TUint aItemCount, TInt aResult) = 0;
	
	virtual void MrcdumlbDoGetPathResult(TUint aItemCount,  
			TInt aResult) = 0;
	
	virtual void MrcdumlbDoSearchResult(TUint aNumberItemsFound,
			TInt aResult) = 0;
	
	virtual void MrcdumlbDoMediaLibraryStateChange() = 0;
	
	virtual void MrcdumlbDoFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult) = 0;
	
	virtual void MrcdumlbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult) = 0;
	};

#endif //REMCONDATABASEUNAWAREMEDIALIBRARYBROWSE_H

