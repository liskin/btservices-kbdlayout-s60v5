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

#ifndef REMCONABSOLUTEVOLUMEAPITARGETOBSERVER_H
#define REMCONABSOLUTEVOLUMEAPITARGETOBSERVER_H

#include <e32base.h>

/**
Clients must implement this interface in order to instantiate objects of type 
CRemConAbsoluteVolumeTarget. This interface passes incoming commands from 
RemCon to the client.
*/
NONSHARABLE_CLASS(MRemConAbsoluteVolumeTargetObserver)
	{
public:
    /** 
	This is called when the controller wishes to set absolute volume on the 
	client. Response must be provided by the client by calling 
	CRemConAbsoluteVolumeTarget::SetAbsoluteVolumeResponse()
	
	@see CRemConAbsoluteVolumeTarget::SetAbsoluteVolumeResponse()
	@param aVolume The relative volume.
	@param aMaxVolume The maximum volume against which aVolume is relative.
	*/
	virtual void MrcavtoSetAbsoluteVolumeRequest(TUint32 aVolume, 
			TUint32 aMaxVolume) = 0;
	};

#endif // REMCONABSOLUTEVOLUMEAPITARGETOBSERVER_H
