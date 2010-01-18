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

#include <remcondatabaseawarenowplayingtarget.h>

EXPORT_C CRemConDatabaseAwareNowPlayingTarget* CRemConDatabaseAwareNowPlayingTarget::NewL(
	CRemConInterfaceSelector& aInterfaceSelector,
	MRemConDatabaseAwareNowPlayingTargetObserver& aObserver)
	{
	CRemConDatabaseAwareNowPlayingTarget* self = new(ELeave)CRemConDatabaseAwareNowPlayingTarget(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
			
CRemConDatabaseAwareNowPlayingTarget::~CRemConDatabaseAwareNowPlayingTarget()
	{
	}

CRemConDatabaseAwareNowPlayingTarget::CRemConDatabaseAwareNowPlayingTarget(
	CRemConInterfaceSelector& aInterfaceSelector,
	MRemConDatabaseAwareNowPlayingTargetObserver& aObserver)
	: CRemConNowPlayingTargetBase(aInterfaceSelector, aObserver)
	{
	}
			
void CRemConDatabaseAwareNowPlayingTarget::ConstructL()
	{
	BaseConstructL();
	}


