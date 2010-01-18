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
 @internalComponent
 @released
*/

#ifndef EVENTSMASK_H
#define EVENTSMASK_H

// INCLUDES
#include <e32base.h>
#include <remcon/avrcpspec.h>

/**
This class stores the supported events list in a bitmask. 
The event ids are numbers from 0x01 to 0x09 (from specs).
A 1 in the 0xXX position means that the XX eventid event is supported, a 0 in 
the same position means that it's not supported.
 */
NONSHARABLE_CLASS(TEventsMask) 
	{
public:
	TEventsMask();
	void Reset();
	TBool Find(TRegisterNotificationEvent aEvent) const; 
	void Append(TRegisterNotificationEvent aEvent);
	void Remove(TRegisterNotificationEvent aEvent);
	void Begin();
	TBool Next();
	TRegisterNotificationEvent Get() const;
private:
	TUint16							iMask;
	TUint8							iNextSupportedEvent;
	};

#endif // EVENTSMASK_H
