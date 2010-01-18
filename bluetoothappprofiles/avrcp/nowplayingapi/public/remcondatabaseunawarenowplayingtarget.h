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

#ifndef REMCONDATABASEUNAWARENOWPLAYINGTARGET_H
#define REMCONDATABASEUNAWARENOWPLAYINGTARGET_H

#include <e32base.h>
#include <remcon/remconnowplayingtargetbase.h>

class CRemConInterfaceSelector;

NONSHARABLE_CLASS(CRemConDatabaseUnawareNowPlayingTarget) : public CRemConNowPlayingTargetBase
	{
public:
	IMPORT_C static CRemConDatabaseUnawareNowPlayingTarget* NewL(CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseUnawareNowPlayingTargetObserver& aObserver);
	virtual ~CRemConDatabaseUnawareNowPlayingTarget();


private:
	CRemConDatabaseUnawareNowPlayingTarget(CRemConInterfaceSelector& aInterfaceSelector,
			MRemConDatabaseUnawareNowPlayingTargetObserver& aObserver);
	void ConstructL();
	};

#endif //REMCONDATABASEUNAWARENOWPLAYINGTARGET_H
