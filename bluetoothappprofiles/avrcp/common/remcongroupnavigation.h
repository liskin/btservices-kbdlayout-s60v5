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
 @internalTechnology
*/

#ifndef REMCONGROUPNAVIGATION_H
#define REMCONGROUPNAVIGATION_H

#include <e32base.h>

//-------------------------------------------------------------------------- 
// Interface Data
//--------------------------------------------------------------------------

/**
The UID identifying the RemCon Group Navigation interface.
*/
const TInt KRemConGroupNavigationApiUid = 0x10285B01;

const TInt KMaxSizeRemConGroupNavigationMsg =1;
	
struct TRemConGroupNavigationResponse
	{
	// Error code 
	TInt	iError;
	};

#endif // REMCONGROUPNAVIGATION_H
