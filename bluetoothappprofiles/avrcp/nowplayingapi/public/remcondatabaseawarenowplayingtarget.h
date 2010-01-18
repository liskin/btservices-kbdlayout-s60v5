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

#ifndef REMCONDATABASEAWARENOWPLAYINGTARGET_H
#define REMCONDATABASEAWARENOWPLAYINGTARGET_H


#include <e32base.h>
#include <remcon/remconnowplayingtargetbase.h>

class CRemConInterfaceSelector;

NONSHARABLE_CLASS(CRemConDatabaseAwareNowPlayingTarget) : public CRemConNowPlayingTargetBase
	{
public:
	IMPORT_C static CRemConDatabaseAwareNowPlayingTarget* NewL(CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseAwareNowPlayingTargetObserver& aObserver);
	virtual ~CRemConDatabaseAwareNowPlayingTarget();


private:
	CRemConDatabaseAwareNowPlayingTarget(CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseAwareNowPlayingTargetObserver& aObserver);
	void ConstructL();
	};

#endif //REMCONDATABASEAWARENOWPLAYINGTARGET_H
