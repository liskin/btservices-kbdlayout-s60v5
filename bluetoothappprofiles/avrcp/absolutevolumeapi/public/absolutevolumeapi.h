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

#ifndef ABSOLUTEVOLUMEAPI_H_
#define ABSOLUTEVOLUMEAPI_H_

#include <e32base.h>

const TInt KRemConAbsoluteVolumeControllerApiUid = 0x1020831E;
const TInt KRemConAbsoluteVolumeTargetApiUid = 0x10215bbd;

const TInt KErrAbsoluteVolumeInternalError     = -6537;

/**
Error code(4 bytes) + The relative volume(4 bytes) + 
The maximum volume against which volume is relative(4 bytes)
*/
const TUint KAbsoluteVolumeResponseDataSize = 12;

/** Operation Id of set absolute volume */
const TUint    KRemConSetAbsoluteVolume						= 0x1050;
/** Operation Id of register volume change notification */
const TUint    KRemConAbsoluteVolumeNotification			= 0x0d31;

/**
The relative volume to be set(4 bytes) + 
The maximum volume against which volume is relative(4 bytes) + 
Reserved(4 bytes)
*/
const TInt KAbsoluteVolumeRequestDataSize = 12;

/**
The absolute volume is represented in one octet. The top bit(bit 7)
is reserved for future use.
*/
const TUint8 KAbsoluteVolumeMask = 0x7f;

/** PDU length of set absolute volume response */
const TUint8 KLengthSetAbsoluteVolumeResponse = 11;
/** PDU length of notify volume change response */
const TUint8 KLengthNotifyVolumeChangeResponse = 12;

/** Absolute Volume(1 byte) */
const TUint8 KLengthSetAbsoluteVolumeRequestParameter = 1;
/** Event ID(1 byte) + Reserved(4 bytes) */
const TUint8 KLengthNotifyVolumeChangeRequestParameter = 5;

/** Absolute Volume(1 byte) */
const TUint8 KLengthSetAbsoluteVolumeResponseParamter = 1;
/** Event ID(1 byte) + Absolute Volume(1 byte) */
const TUint8 KLengthNotifyVolumeChangeResponseParameter = 2;
	
#endif /*ABSOLUTEVOLUMEAPI_H_*/
