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
// This file contains structures shared by the AVRCP API with the AVRCP bearer.
// 
//

/**
 @file
 @internalTechnology
 @released
*/

#include <e32std.h>
#include <remcon/avrcpspec.h>
#include "avrcpipc.h"


EXPORT_C TMetadataTransferPDU RAvrcpIPC::GetPDUIdFromIPCOperationId(TInt aOperationId)
	{
	// We send the PDU id through IPC to the playerinformationapi, mediainformationapi
	// groupnavigationapi or batterystatusapi. However, PDU 0x31 (Register Notification
	// also requires sending an event id. We encode this as the top byte, and the PDU
	// id as the lower byte, so mask off the top-byte and return just the PDU id.
	return static_cast<TMetadataTransferPDU>(aOperationId & 0x00FF);
	}

EXPORT_C TRegisterNotificationEvent RAvrcpIPC::GetEventIdFromIPCOperationId(TInt aOperationId)
	{
	// See the above comment in GetPDUIdFromIPCOperationId. This performs the opposite operation
	return static_cast<TRegisterNotificationEvent>((aOperationId & 0xFF00) >> 8);
	}

EXPORT_C TInt RAvrcpIPC::SetIPCOperationIdFromEventId(TRegisterNotificationEvent aEventId)
	{
	// See the comment in GetPDUIdFromIPCOperationId. This sets the PDU id
	// as the lower byte to RegisterNotification (0x31) and the eventId as
	// the top byte.
	return (ERegisterNotification + (aEventId << 8));
	}

// Big-endian methods; Symbian native methods are little-endian
// This means that we can directly send these packets into the
// wire in the bearer, since they're big-endian. We could have
// used the BigEndian class, but that requires a dependency on
// the esock.lib library, so do it ourselves.

TUint8 RAvrcpIPC::Read8L()
	{
	return iReadStream.ReadUint8L();
	}

TUint16 RAvrcpIPC::Read16L()
	{
	TUint16 val = 0;
	val  = (static_cast<TUint16>(iReadStream.ReadUint8L()) << 8);
	val += iReadStream.ReadUint8L();
	return val;
	}

TUint32 RAvrcpIPC::Read24L()
	{
	TUint32 val = 0;
	val += (static_cast<TUint32>(iReadStream.ReadUint8L()) << 16);
	val += (static_cast<TUint32>(iReadStream.ReadUint8L()) << 8);
	val += iReadStream.ReadUint8L();
	return val;
	}

TUint32 RAvrcpIPC::Read32L()
	{
	TUint32 val = 0;
	val  = (static_cast<TUint32>(iReadStream.ReadUint8L()) << 24);
	val += (static_cast<TUint32>(iReadStream.ReadUint8L()) << 16);
	val += (static_cast<TUint32>(iReadStream.ReadUint8L()) << 8);
	val += iReadStream.ReadUint8L();
	return val;
	}

TUint64 RAvrcpIPC::Read64L()
	{
	TUint64 val;
	val  = (static_cast<TUint64>(iReadStream.ReadUint8L()) << 56);
	val += (static_cast<TUint64>(iReadStream.ReadUint8L()) << 48);
	val += (static_cast<TUint64>(iReadStream.ReadUint8L()) << 40);
	val += (static_cast<TUint64>(iReadStream.ReadUint8L()) << 32);
	val += (static_cast<TUint64>(iReadStream.ReadUint8L()) << 24);
	val += (static_cast<TUint64>(iReadStream.ReadUint8L()) << 16);
	val += (static_cast<TUint64>(iReadStream.ReadUint8L()) << 8);
	val += iReadStream.ReadUint8L();
	return val;
	}

void RAvrcpIPC::Write8L(TUint8 aVal)
	{
	iStream.WriteUint8L(aVal);
	}

void RAvrcpIPC::Write16L(TUint16 aVal)
	{
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 8));
	iStream.WriteUint8L(static_cast<TUint8>(aVal));
	}

void RAvrcpIPC::Write24L(TUint32 aVal)
	{
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 16));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 8));
	iStream.WriteUint8L(static_cast<TUint8>(aVal));
	}

void RAvrcpIPC::Write32L(TUint32 aVal)
	{
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 24));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 16));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 8));
	iStream.WriteUint8L(static_cast<TUint8>(aVal));
	}

void RAvrcpIPC::Write64L(TUint64 aVal)
	{
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 56));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 48));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 40));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 32));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 24));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 16));
	iStream.WriteUint8L(static_cast<TUint8>(aVal >> 8));
	iStream.WriteUint8L(static_cast<TUint8>(aVal));
	}
void RAvrcpIPC::Close()
	{
	iReadStream.Close();
	iStream.Close();
	}

EXPORT_C void RAvrcpIPCError::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iError = Read32L();   // Read Big-Endian error code
	iReadStream.Close();
	}

EXPORT_C void RAvrcpIPCError::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();      // Error code is always before any other data
	iStream.Open(aOutData);
	Write32L(iError);
	iStream.CommitL();
	}

