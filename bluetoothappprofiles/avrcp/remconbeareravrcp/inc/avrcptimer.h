// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef AVRCPTIMER_H
#define AVRCPTIMER_H

/**
@file
@internalComponent
@released
*/

#include <e32base.h>

class CControlCommand;
NONSHARABLE_CLASS(TAvrcpTimerExpiryInfo)
	{
public:
	inline TAvrcpTimerExpiryInfo(CBase* aHandler, CControlCommand& aCommand);
public:
	CBase*			iHandler;
	CControlCommand&	iCommand;
	};
	

//------------------------------------------------------------------------------------
// TAvrcpTimerEntry
//------------------------------------------------------------------------------------

/** Constructor.

@param aHandler	The CommandHandler to be informed on timer expiry.
@param aCommand	The CControlCommand that this timer event relates to.
@return A fully constructed TAvrcpTimerExpiryInfo.
*/
inline TAvrcpTimerExpiryInfo::TAvrcpTimerExpiryInfo(CBase* aHandler, CControlCommand& aCommand)
	: iHandler(aHandler), iCommand(aCommand)
	{
	}

#endif // AVRCPTIMER_H
