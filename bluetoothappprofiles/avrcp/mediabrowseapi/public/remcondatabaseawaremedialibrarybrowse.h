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

#ifndef REMCONDATABASEAWAREMEDIALIBRARYBROWSE_H
#define REMCONDATABASEAWAREMEDIALIBRARYBROWSE_H

NONSHARABLE_CLASS(MRemConDatabaseAwareMediaLibraryBrowse)
	{
public:
	IMPORT_C void MrcdamlbFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint16 aMediaLibraryStateCookie, 
			TUint aResult);
	
	IMPORT_C void MrcdamlbFolderUpResult(TUint aItemCount, TInt aResult);
	IMPORT_C void MrcdamlbFolderDownResult(TUint aItemCount, TInt aResult);
	
	IMPORT_C void MrcdamlbGetPathResult(TUint aItemCount, 
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	IMPORT_C void MrcdamlbSearchResult(TUint aNumberItemsFound,
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	IMPORT_C void MrcdamlbMediaLibraryStateChange(TUint16 aMediaLibraryStateCookie);
	
	IMPORT_C void MrcdamlbFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	IMPORT_C void MrcdamlbMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
private:
	virtual void MrcdamlbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint16 aMediaLibraryStateCookie, 
			TUint aResult) = 0;
	
	virtual void MrcdamlbDoFolderUpResult(TUint aItemCount, TInt aResult) = 0;
	virtual void MrcdamlbDoFolderDownResult(TUint aItemCount, TInt aResult) = 0;
	
	virtual void MrcdamlbDoGetPathResult(TUint aItemCount, 
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult) = 0;
	
	virtual void MrcdamlbDoSearchResult(TUint aNumberItemsFound,
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult) = 0;
	
	virtual void MrcdamlbDoMediaLibraryStateChange(TUint16 aMediaLibraryStateCookie) = 0;
	
	virtual void MrcdamlbDoFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult) = 0;
	
	virtual void MrcdamlbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult) = 0;
	};

#endif // REMCONDATABASEAWAREMEDIALIBRARYBROWSE_H
