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
// playerinformation.h This file contains structures shared by the AVRCP API with the AVRCP bearer.
// 
//

/**
 @file
 @internalTechnology
 @released
*/

#ifndef REMCONPLAYERINFORMATION_H
#define REMCONPLAYERINFORMATION_H

#include "playerinformationtarget.h"
#include "avrcpipc.h"
#include <avcframe.h>

/**
 * The UID identifying the RemCon PlayerInformation interface.
 */
const TInt KRemConPlayerInformationUid = 0x10285B21;

// The Bluetooth SIG registered VendorId, see table 4.7
const AVC::TAVCVendorId KBluetoothSIGVendorId = 0x001958;

// This is the maximum length of a request that can be received
// GetElementAttributes (PDU 0x20) has the largest possible request
// which is identifier (8 bytes) + numAttr (1 byte) + attributes
// (255 * 4 bytes) the total of which is 1029 bytes.
const TInt KMaxLengthPlayerInformationMsg = 1029;


// Used by PDU 0x11 and PDU 0x12
class RRemConPlayerListOfAttributes : public RAvrcpIPC
{
public:
	TInt			iNumberAttributes;
	RArray<TInt>	iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
};

// Used by PDU 0x13 (response) and PDU 0x14 (request)
class RRemConPlayerAttributeIdsAndValues : public RAvrcpIPC
{
public:
	TInt			iNumberAttributes;
	RArray<TInt>	iAttributeId;
	RArray<TInt>	iAttributeValue;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
};

class RSettingWithCharset
	{
public:
	IMPORT_C void Close();
	
public:
	TInt	iAttributeId;
	TUint16	iCharset;
	TInt	iStringLen;
	HBufC8*	iString;
};

// Used by PDU 0x15 and PDU 0x16
class RRemConGetPlayerApplicationTextResponse  : public RAvrcpIPC
{
public:
	TInt						iNumberAttributes;
	RArray<RSettingWithCharset>	iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
	IMPORT_C TInt Size();
};

// Used by PDU 0x10
class RRemConGetCapabilitiesResponse : public RAvrcpIPC
{
public:
	TUint16			iCapabilityId;
	TUint16			iCapabilityCount;
	RArray<TUint32>	iCapabilities;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
};

// Used by PDU 0x30
class RRemConPlayerInformationGetPlayStatusResponse : public RAvrcpIPC
	{
public:
	TUint			iPlayPos;
	TUint			iTrackLength;
	MPlayerEventsObserver::TPlaybackStatus	iStatus;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	};

// Used by PDU 0xff
class RRemConPlayerInformationGetPlayStatusUpdateRequest : public RAvrcpIPC
	{
public:
	MPlayerEventsObserver::TPlaybackStatus	iStatus;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	};

// Used by PDU 0xff
class RRemConPlayerInformationGetPlayStatusUpdateResponse : public RAvrcpIPC
	{
public:
	MPlayerEventsObserver::TPlaybackStatus	iStatus;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	};

class RRemConPlayerInformation8BitResponse : public RAvrcpIPC
	{
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	TUint8			iValue;
	};

class RRemConPlayerInformation32BitResponse : public RAvrcpIPC
	{
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	TInt			iValue;
	};

class RRemConPlayerInformation64BitResponse : public RAvrcpIPC
	{
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	TUint64			iValue;
	};

#endif // REMCONPLAYERINFORMATION_H
