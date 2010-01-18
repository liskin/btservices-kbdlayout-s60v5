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

#include "eventsmask.h"

#ifdef _DEBUG
_LIT(KEventsMask, "TEventsMask");
#endif

TEventsMask::TEventsMask ()
	{
	Reset();
	}

void TEventsMask::Reset()
	{
	iMask = 0x0000;
	iNextSupportedEvent = 0x00;
	}

TBool TEventsMask::Find(TRegisterNotificationEvent aEvent) const
	{
	return ( (0x0001 << (aEvent -1)) & iMask ) ? ETrue : EFalse;
	}

void TEventsMask::Append(TRegisterNotificationEvent aEvent)
	{
	// the assertion assures that aEvent is a valid argument (in the range) 
	__ASSERT_DEBUG((aEvent >= ERegisterNotificationPlaybackStatusChanged) && 
		(aEvent < ERegisterNotificationReservedLast), 
		User::Panic(KEventsMask, KErrArgument));

	iMask |= (0x0001 << (aEvent -1));
	}

void TEventsMask::Remove(TRegisterNotificationEvent aEvent)
	{
	// the assertion assures that aEvent is a valid argument (in the range)
	__ASSERT_DEBUG((aEvent >= ERegisterNotificationPlaybackStatusChanged) && 
		(aEvent < ERegisterNotificationReservedLast), 
		User::Panic(KEventsMask, KErrArgument));
		
	iMask &= ~(0x0001 << (aEvent -1));
	}

void TEventsMask::Begin()
	{
	iNextSupportedEvent = 0x00;
	}

TBool TEventsMask::Next()
	{
	TUint index = iNextSupportedEvent+1;
	
	if (index == ERegisterNotificationReservedLast)
		{
		return EFalse;
		}
	
	while( (!Find((TRegisterNotificationEvent)index)) && (index < ERegisterNotificationReservedLast) )
		{
		index++;
		}
	
	if (index < ERegisterNotificationReservedLast)
		{
		iNextSupportedEvent = index;
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

TRegisterNotificationEvent TEventsMask::Get() const
	{
	// if Get() is called before calling Begin() and at least one call to Next()
	// it Panics because iNextSupportedEvent is invalid (equals to 0x00).
	__ASSERT_DEBUG(iNextSupportedEvent >= ERegisterNotificationPlaybackStatusChanged, User::Panic(KEventsMask, KErrUnderflow));
	// this should never happen, it would mean that iNextSupport overtakes the
	// last valid value
	__ASSERT_DEBUG(iNextSupportedEvent < ERegisterNotificationReservedLast, User::Panic(KEventsMask, KErrOverflow));

	return static_cast<TRegisterNotificationEvent>(iNextSupportedEvent);
	}
