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

#ifndef REMCONDATABASEAWAREMEDIABROWSETARGET_H
#define REMCONDATABASEAWAREMEDIABROWSETARGET_H

#include <e32base.h>
#include <remcondatabaseawaremedialibrarybrowse.h>
#include <remcondatabaseawaremedialibrarybrowseobserver.h>
#include <remcondatabaseawarenowplayingbrowse.h>
#include <remcondatabaseawarenowplayingbrowseobserver.h>
#include <remcon/remconmediabrowsetargetbase.h>

NONSHARABLE_CLASS(CRemConDatabaseAwareMediaBrowseTarget) : public CRemConMediaBrowseTargetBase,
	private MRemConDatabaseAwareMediaLibraryBrowse, private MRemConDatabaseAwareNowPlayingBrowse
	{
public:
	IMPORT_C static CRemConDatabaseAwareMediaBrowseTarget* NewL(
			CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseAwareMediaLibraryBrowseObserver& aMlObserver, 
			MRemConDatabaseAwareNowPlayingBrowseObserver& aNpObserver,
			TBool aSearchSupported, 
			MRemConDatabaseAwareMediaLibraryBrowse*& aMlInterface, 
			MRemConDatabaseAwareNowPlayingBrowse*& aNpInterface,
			TUint16 aMediaLibraryStateCookie);
	virtual ~CRemConDatabaseAwareMediaBrowseTarget();
	
private:
	// from MRemConDatabaseAwareMediaLibraryBrowse
	virtual void MrcdamlbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint16 aMediaLibraryStateCookie, 
			TUint aResult);
	
	virtual void MrcdamlbDoFolderUpResult(TUint aItemCount, TInt aResult);
	virtual void MrcdamlbDoFolderDownResult(TUint aItemCount, TInt aResult);
	
	virtual void MrcdamlbDoGetPathResult(TUint aItemCount, 
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	virtual void MrcdamlbDoSearchResult(TUint aNumberItemsFound,
			TUint16 aMediaLibraryStateCookie, 
			TInt aResult);
	
	virtual void MrcdamlbDoMediaLibraryStateChange(TUint16 aMediaLibraryStateCookie);
	
	virtual void MrcdamlbDoFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	virtual void MrcdamlbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	// from MRemConDatabaseAwareNowPlayingBrowse
	virtual void MrcdanpbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint16 aMediaLibraryStateCookie, 
			TUint aResult);
		
	virtual void MrcdanpbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
private:
	CRemConDatabaseAwareMediaBrowseTarget(
			CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConDatabaseAwareMediaLibraryBrowseObserver& aMlObserver, 
			MRemConDatabaseAwareNowPlayingBrowseObserver& aNpObserver, 
			MRemConDatabaseAwareMediaLibraryBrowse*& aMlInterface, 
			MRemConDatabaseAwareNowPlayingBrowse*& aNpInterface,
			TUint16 aMediaLibraryStateCookie);
	void ConstructL(TBool aSearchSupported);
	};

#endif //REMCONDATABASEAWAREMEDIABROWSETARGET_H
