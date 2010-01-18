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
 @publishedAll
 @released
*/

#ifndef REMCONBATTERYTARGETOBSERVER_H
#define REMCONBATTERYTARGETOBSERVER_H

#include <e32base.h>

enum TControllerBatteryStatus
	{
	ENormal     = 0,
	EWarning    = 1,
	ECritical   = 2,
	EExternal   = 3,
	EFullCharge = 4,
	};

/**
Clients must implement this interface in order to instantiate objects of type 
CRemConBatteryTarget. This interface passes incoming commands from RemCon to 
the client. 
*/
class MRemConBatteryTargetObserver
	{
public:
	/** 
	A 'InformBatteryStatusOfCT' has been received.
	@param aBatteryStatus The current battery status.
	*/
	virtual void MrcbstoBatteryStatus(TControllerBatteryStatus& aBatteryStatus) = 0;

	};

#endif // REMCONBATTERYSTATUSTARGETOBSERVER_H
