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

#include <remcondatabaseunawaremediabrowsetarget.h>

EXPORT_C CRemConDatabaseUnawareMediaBrowseTarget* CRemConDatabaseUnawareMediaBrowseTarget::NewL(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConDatabaseUnawareMediaLibraryBrowseObserver& aMlObserver, 
		MRemConDatabaseUnawareNowPlayingBrowseObserver& aNpObserver,
		TBool aSearchSupported, 
		MRemConDatabaseUnawareMediaLibraryBrowse*& aMlInterface, 
		MRemConDatabaseUnawareNowPlayingBrowse*& aNpInterface)
	{
	CRemConDatabaseUnawareMediaBrowseTarget* self = new(ELeave)CRemConDatabaseUnawareMediaBrowseTarget(
			aInterfaceSelector, aMlObserver, aNpObserver, aMlInterface, aNpInterface);
	CleanupStack::PushL(self);
	self->ConstructL(aSearchSupported);
	CleanupStack::Pop(self);
	return self;
	}

CRemConDatabaseUnawareMediaBrowseTarget::CRemConDatabaseUnawareMediaBrowseTarget(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConDatabaseUnawareMediaLibraryBrowseObserver& aMlObserver, 
		MRemConDatabaseUnawareNowPlayingBrowseObserver& aNpObserver, 
		MRemConDatabaseUnawareMediaLibraryBrowse*& aMlInterface, 
		MRemConDatabaseUnawareNowPlayingBrowse*& aNpInterface)
	: CRemConMediaBrowseTargetBase(aInterfaceSelector, aMlObserver, aNpObserver)
	{
	aMlInterface = this;
	aNpInterface = this;
	}

void CRemConDatabaseUnawareMediaBrowseTarget::ConstructL(
		TBool aSearchSupported)
	{
	BaseConstructL(aSearchSupported);
	}

CRemConDatabaseUnawareMediaBrowseTarget::~CRemConDatabaseUnawareMediaBrowseTarget()
	{
	}
	
void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint aResult)
	{
	DoFolderListing(aFolderListing, 0, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoFolderUpResult(
		TUint aItemCount, TInt aResult)
	{
	DoFolderUpResult(aItemCount, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoFolderDownResult(
		TUint aItemCount, TInt aResult)
	{
	DoFolderDownResult(aItemCount, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoGetPathResult(
		TUint aItemCount,  
		TInt aResult)
	{
	DoGetPathResult(aItemCount, 0, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoSearchResult(
		TUint aNumberItemsFound,
		TInt aResult)
	{
	DoSearchResult(aNumberItemsFound, 0, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoMediaLibraryStateChange()
	{
	DoMediaLibraryStateChange(0);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoFolderItemResult(
		const TRemConItemUid& aFolderID,
		const TDesC8& aFolderName, 
		TFolderItemType aFolderType, 
		TFolderItemPlayable aPlayable,
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	DoFolderItemResult(aFolderID,aFolderName, aFolderType, aPlayable, aAttributes, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdumlbDoMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	DoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdunpbDoFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint aResult)
	{
	DoFolderListing(aFolderListing, 0, aResult);
	}

void CRemConDatabaseUnawareMediaBrowseTarget::MrcdunpbDoMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	DoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}

