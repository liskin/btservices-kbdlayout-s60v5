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

#include <remcondatabaseunawarenowplayingtarget.h>

EXPORT_C CRemConDatabaseUnawareNowPlayingTarget* CRemConDatabaseUnawareNowPlayingTarget::NewL(
	CRemConInterfaceSelector& aInterfaceSelector,
	MRemConDatabaseUnawareNowPlayingTargetObserver& aObserver)
	{
	CRemConDatabaseUnawareNowPlayingTarget* self = new(ELeave)CRemConDatabaseUnawareNowPlayingTarget(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
			
CRemConDatabaseUnawareNowPlayingTarget::~CRemConDatabaseUnawareNowPlayingTarget()
	{
	}

CRemConDatabaseUnawareNowPlayingTarget::CRemConDatabaseUnawareNowPlayingTarget(
	CRemConInterfaceSelector& aInterfaceSelector,
	MRemConDatabaseUnawareNowPlayingTargetObserver& aObserver)
	: CRemConNowPlayingTargetBase(aInterfaceSelector, aObserver)
	{
	}
			
void CRemConDatabaseUnawareNowPlayingTarget::ConstructL()
	{
	BaseConstructL();
	}
