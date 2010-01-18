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
// This file contains structures shared by the AVRCP API with the AVRCP bearer.
// 
//

/**
 @file
 @internalTechnology
 @released
*/

#ifndef AVRCPIPC_H
#define AVRCPIPC_H

#include <s32mem.h>
#include <remcon/avrcpspec.h>

// Structure of commands for AVRCP player information requests via IPC
class RAvrcpIPC
{
public:
	IMPORT_C static TMetadataTransferPDU GetPDUIdFromIPCOperationId(TInt aOperationId);
	IMPORT_C static TRegisterNotificationEvent GetEventIdFromIPCOperationId(TInt aOperationId);
	IMPORT_C static TInt SetIPCOperationIdFromEventId(TRegisterNotificationEvent aEventId);
	IMPORT_C static TUint8 SymbianErrToStatus(TInt aErr);
    IMPORT_C static TInt SymbianErrorCheck(TInt aError);
	
public:
	virtual void ReadL(const TDesC8& aData) = 0;
	virtual void WriteL(TDes8& aOutData) = 0;
	
protected:
	// Big-endian methods; Symbian native methods are little-endian
	TUint8  Read8L();
	TUint16 Read16L();
	TUint32 Read32L();
	TUint32 Read24L();   // Required for PDU 0x10
	TUint64 Read64L();
	void Write8L(TUint8 aVal);
	void Write16L(TUint16 aVal);
	void Write24L(TUint32 aVal); // Required for PDU 0x10
	void Write32L(TUint32 aVal);
	void Write64L(TUint64 aVal);
	void Close();

public:
	RDesWriteStream iStream;
	RDesReadStream iReadStream;
};

const TUint8 KLengthErrorResponse = 4;               // 4 bytes for errors returned via IPC
class RAvrcpIPCError : public RAvrcpIPC
{
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	
public:
	TInt iError;
};

#endif // AVRCPIPC_H
