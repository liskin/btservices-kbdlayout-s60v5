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

#include <remcondatabaseawaremediabrowsetarget.h>

EXPORT_C CRemConDatabaseAwareMediaBrowseTarget* CRemConDatabaseAwareMediaBrowseTarget::NewL(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConDatabaseAwareMediaLibraryBrowseObserver& aMlObserver, 
		MRemConDatabaseAwareNowPlayingBrowseObserver& aNpObserver,
		TBool aSearchSupported, 
		MRemConDatabaseAwareMediaLibraryBrowse*& aMlInterface, 
		MRemConDatabaseAwareNowPlayingBrowse*& aNpInterface,
		TUint16 aMediaLibraryStateCookie)
	{
	CRemConDatabaseAwareMediaBrowseTarget* self = new(ELeave)CRemConDatabaseAwareMediaBrowseTarget(
			aInterfaceSelector, aMlObserver, aNpObserver, aMlInterface, aNpInterface, aMediaLibraryStateCookie);
	CleanupStack::PushL(self);
	self->ConstructL(aSearchSupported);
	CleanupStack::Pop(self);
	return self;
	}

CRemConDatabaseAwareMediaBrowseTarget::CRemConDatabaseAwareMediaBrowseTarget(
		CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConDatabaseAwareMediaLibraryBrowseObserver& aMlObserver, 
		MRemConDatabaseAwareNowPlayingBrowseObserver& aNpObserver, 
		MRemConDatabaseAwareMediaLibraryBrowse*& aMlInterface, 
		MRemConDatabaseAwareNowPlayingBrowse*& aNpInterface,
		TUint16 aMediaLibraryStateCookie)
	: CRemConMediaBrowseTargetBase(aInterfaceSelector, aMlObserver, aNpObserver, aMediaLibraryStateCookie)
	{
	aMlInterface = static_cast<MRemConDatabaseAwareMediaLibraryBrowse*>(this);
	aNpInterface = static_cast<MRemConDatabaseAwareNowPlayingBrowse*>(this);
	}

void CRemConDatabaseAwareMediaBrowseTarget::ConstructL(
		TBool aSearchSupported)
	{
	BaseConstructL(aSearchSupported);
	}

CRemConDatabaseAwareMediaBrowseTarget::~CRemConDatabaseAwareMediaBrowseTarget()
	{
	}
	
void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint16 aMediaLibraryStateCookie, 
		TUint aResult)
	{
	DoFolderListing(aFolderListing, aMediaLibraryStateCookie, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoFolderUpResult(
		TUint aItemCount, TInt aResult)
	{
	DoFolderUpResult(aItemCount, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoFolderDownResult(
		TUint aItemCount, TInt aResult)
	{
	DoFolderDownResult(aItemCount, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoGetPathResult(
		TUint aItemCount, 
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	DoGetPathResult(aItemCount, aMediaLibraryStateCookie, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoSearchResult(
		TUint aNumberItemsFound,
		TUint16 aMediaLibraryStateCookie, 
		TInt aResult)
	{
	DoSearchResult(aNumberItemsFound, aMediaLibraryStateCookie, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoMediaLibraryStateChange(
		TUint16 aMediaLibraryStateCookie)
	{
	DoMediaLibraryStateChange(aMediaLibraryStateCookie);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoFolderItemResult(
		const TRemConItemUid& aFolderID,
		const TDesC8& aFolderName, 
		TFolderItemType aFolderType, 
		TFolderItemPlayable aPlayable,
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	DoFolderItemResult(aFolderID,aFolderName, aFolderType, aPlayable, aAttributes, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdamlbDoMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	DoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdanpbDoFolderListing(
		const TArray<TRemConItem>& aFolderListing, 
		TUint16 aMediaLibraryStateCookie, 
		TUint aResult)
	{
	DoFolderListing(aFolderListing, aMediaLibraryStateCookie, aResult);
	}

void CRemConDatabaseAwareMediaBrowseTarget::MrcdanpbDoMediaElementItemResult(
		const TRemConItemUid& aMediaID, 
		const TDesC8& aMediaName, 
		TMediaItemType aMediaType, 
		const TArray<TMediaElementAttribute>& aAttributes,
		TInt aResult)
	{
	DoMediaElementItemResult(aMediaID, aMediaName, aMediaType, aAttributes, aResult);
	}
