// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef REMCONBATTERY_H
#define REMCONBATTERY_H

#include <e32base.h>


//-------------------------------------------------------------------------- 
// Interface Data
//--------------------------------------------------------------------------

/**
The UID identifying this outer-layer RemCon interface.
*/
const TInt KRemConBatteryApiUid = 0x10285AE7;

struct TRemConBatteryApiResponse
	{
	// Error code 
	TInt	iError;
	};

const TInt KLengthBatteryStatusMsg = 1;
	

#endif // REMCONBATTERY_H
