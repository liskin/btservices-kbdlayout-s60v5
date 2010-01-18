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

#ifndef AVRCPINTERNALINTERFACE_H
#define AVRCPINTERNALINTERFACE_H

/**
@file
@internalTechnology
@prototype
*/

#include <e32base.h>
#include "avrcpipc.h"
#include "browsingframe.h"

const TInt KUidAvrcpInternalInterface = 0x10285eef;

enum TAvrcpInternalInterfaceOperationId
	{
	EAvrcpInternalGetFolderItems = 0x0,
	EAvrcpInternalAvailablePlayersNotification = 0xa31,
	EAvrcpInternalAddressedPlayerNotification = 0xb31,
	EAvrcpInternalUidChangedNotification = 0xc31,
	EAvrcpInternalSetAddressedPlayer = 0x60,
	};

const TInt KSetAddressedPlayerResponseSize = 8;
const TInt KRegisterNotificationEmptyResponseSize = 4;
const TInt KRegisterNotificationAddressedPlayerResponseSize = 8;
const TInt KRegisterNotificationUidChangedResponseSize = 6;
const TUint16 KMediaPlayerListUidCounter = 0;

class RMediaPlayerItem
	{
public:
	IMPORT_C void Close();
	IMPORT_C TInt Size();
	
public:
	AvrcpBrowsing::TItemType	iType;
	TUint16						iLength;
	TUint16						iPlayerId;
	
	TUint8						iPlayerType;
	TUint32						iPlayerSubType;
	TUint8						iPlayStatus;
	TBuf8<16>					iFeatureBitmask;
	
	TUint16						iCharset;
	TUint16						iNameLength;
	RBuf8						iName;
	};

//--------------------------------------------------
// SetAddressedPlayer 0x60
//--------------------------------------------------
class RAvrcpSetAddressedPlayerRequest : public RAvrcpIPC
	{
public:
	TUint16	iPlayerId;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

class RAvrcpSetAddressedPlayerResponse : public RAvrcpIPC
	{
public:
	TUint8					iStatus;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

//--------------------------------------------------
// GetFolderItems Media Player Scope 0x71
//--------------------------------------------------
class RAvrcpGetFolderItemsRequest : public RAvrcpIPC
	{
public:
	AvrcpBrowsing::TScope	iScope;
	TUint					iStartItem;
	TUint					iEndItem;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
	};

class RAvrcpGetFolderItemsResponse : public RAvrcpIPC
	{
public:
	TUint8					iPduId;
	TUint16					iParamLength;
	TUint8					iStatus;
	TUint16					iUidCounter;
	TUint16 				iNumberItems;
	RArray<RMediaPlayerItem>	iItems;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C TInt Size();
	IMPORT_C void Close();
	
private:
	void ReadItemL();
	void WriteItemL(TInt aIndex);
	};

//--------------------------------------------------
// Register Notification Addressed Player
//--------------------------------------------------
class RAvrcpAddressedPlayerNotificationResponse : public RAvrcpIPC
	{
public:
	TUint16		iPlayerId;
	TUint16		iUidCounter;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	};

//--------------------------------------------------
// Register Notification Uid Counter
//--------------------------------------------------
class RAvrcpUidCounterNotificationResponse : public RAvrcpIPC
	{
public:
	TUint16		iUidCounter;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8&);
	};


#endif //AVRCPINTERNALINTERFACE_H
