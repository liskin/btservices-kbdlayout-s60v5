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


#ifndef REMCONABSOLUTEVOLUMEAPICONTROLLEROBSERVER_H_
#define REMCONABSOLUTEVOLUMEAPICONTROLLEROBSERVER_H_

#include <e32base.h>

/**
Clients must implement this interface in order to instantiate objects of type 
CRemConAbsoluteVolumeController. This interface passes incoming responses 
from RemCon to the client.
*/
class MRemConAbsoluteVolumeControllerObserver
	{
public:
	/** 
	The current volume has been received.
	 
	@param aVolume The current relative volume on the target device.
	@param aMaxVolume The maximum volume against which aVolume is relative.
	@param aError The response error.
	*/
	virtual void MrcavcoCurrentVolume(TUint32 aVolume, 
			TUint32 aMaxVolume, 
			TInt aError)=0;

	/**
	The response for setting absolute volume.
	
	@param aVolume The relative volume that has been set on the target device.
	@param aMaxVolume The maximum volume against which aVolume is relative.
	@param aError The response error.
	*/
	virtual void MrcavcoSetAbsoluteVolumeResponse(TUint32 aVolume, 
			TUint32 aMaxVolume, 
			TInt aError)=0;
	
	/**
	The error occurs for absolute volume notification. 
	
	When this is called, the client must call
	RegisterAbsoluteVolumeNotification() to register again if the client 
	wants to receive the notification.
	*/
	virtual void MrcavcoAbsoluteVolumeNotificationError() = 0;

	};

#endif /*REMCONABSOLUTEVOLUMEAPICONTROLLEROBSERVER_H_*/
