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

#ifndef REMCONDATABASEUNAWAREMEDIABROWSETARGET_H
#define REMCONDATABASEUNAWAREMEDIABROWSETARGET_H

#include <e32base.h>
#include <remcondatabaseunawaremedialibrarybrowse.h>
#include <remcondatabaseunawaremedialibrarybrowseobserver.h>
#include <remcondatabaseunawarenowplayingbrowse.h>
#include <remcondatabaseunawarenowplayingbrowseobserver.h>
#include <remcon/remconmediabrowsetargetbase.h>

NONSHARABLE_CLASS(CRemConDatabaseUnawareMediaBrowseTarget) : public CRemConMediaBrowseTargetBase,
private MRemConDatabaseUnawareMediaLibraryBrowse, private MRemConDatabaseUnawareNowPlayingBrowse
	{
public:
	IMPORT_C static CRemConDatabaseUnawareMediaBrowseTarget* NewL(
			CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConDatabaseUnawareMediaLibraryBrowseObserver& aMlObserver, 
			MRemConDatabaseUnawareNowPlayingBrowseObserver& aNpObserver,
			TBool aSearchSupported, 
			MRemConDatabaseUnawareMediaLibraryBrowse*& aMlInterface, 
			MRemConDatabaseUnawareNowPlayingBrowse*& aNpInterface);
	
	virtual ~CRemConDatabaseUnawareMediaBrowseTarget();

private:
	// from MRemConDatabaseUnawareMediaLibraryBrowse
	virtual void MrcdumlbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint aResult);
	
	virtual void MrcdumlbDoFolderUpResult(TUint aItemCount, TInt aResult);
	virtual void MrcdumlbDoFolderDownResult(TUint aItemCount, TInt aResult);
	
	virtual void MrcdumlbDoGetPathResult(TUint aItemCount,  
			TInt aResult);
	
	virtual void MrcdumlbDoSearchResult(TUint aNumberItemsFound,
			TInt aResult);
	
	virtual void MrcdumlbDoMediaLibraryStateChange();
	
	virtual void MrcdumlbDoFolderItemResult(const TRemConItemUid& aFolderID,
			const TDesC8& aFolderName, 
			TFolderItemType aFolderType, 
			TFolderItemPlayable aPlayable,
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	virtual void MrcdumlbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
	// from MRemConDatabaseUnawareNowPlayingBrowse
	virtual void MrcdunpbDoFolderListing(const TArray<TRemConItem>& aFolderListing, 
			TUint aResult);
		
	virtual void MrcdunpbDoMediaElementItemResult(const TRemConItemUid& aMediaID, 
			const TDesC8& aMediaName, 
			TMediaItemType aMediaType, 
			const TArray<TMediaElementAttribute>& aAttributes,
			TInt aResult);
	
private:
	CRemConDatabaseUnawareMediaBrowseTarget(
			CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConDatabaseUnawareMediaLibraryBrowseObserver& aMlObserver, 
			MRemConDatabaseUnawareNowPlayingBrowseObserver& aNpObserver, 
			MRemConDatabaseUnawareMediaLibraryBrowse*& aMlInterface, 
			MRemConDatabaseUnawareNowPlayingBrowse*& aNpInterface);
	
	void ConstructL(TBool aSearchSupported);
	};

#endif //REMCONDATABASEUNAWAREMEDIABROWSETARGET_H
