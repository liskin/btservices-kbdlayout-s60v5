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

#include <remconaddress.h>
#include <remcon/avrcpspec.h>
#include "avrcpinternalinterface.h"
#include "avrcplog.h"
#include "avrcputils.h"
#include "browsecommand.h"
#include "browsingframe.h"
#include "mediabrowse.h"
#include "nowplaying.h"

TInt CBrowseCommand::HandleSetBrowsedPlayer()
	{
	LOG_FUNC	
	
	TPtrC8 payload;
	AvrcpBrowsing::BrowsingFrame::Payload(iFrame, payload);
	if (payload.Length() < KMinLengthSetBrowsedPlayerPdu)
		{
		return KErrAvrcpAirInvalidCommand;
		}

	iOperationId = ESetBrowsedPlayerOperationId;
	iInterfaceUid = TUid::Uid(KRemConMediaBrowseApiUid);
	
	RRemConSetBrowsedPlayerRequest request;
	request.iPlayerId = static_cast<TUint16>(payload[0]) << 8;
	request.iPlayerId |= payload[1];
	request.iMaxResponse = iMaxResponse;
	TBuf8<KMediaBrowseOutBufMaxLength> buf;
	// Only reason for WriteL to leave is insufficently large buffer
	TRAP_IGNORE(request.WriteL(buf));
	TInt err = AppendIncomingPayload(buf);
	return (err == KErrNone) ? KErrAvrcpFurtherProcessingRequired : err;
	}

TInt CBrowseCommand::HandleGetFolderItems()
	{
	LOG_FUNC	

	TPtrC8 payload;
	AvrcpBrowsing::BrowsingFrame::Payload(iFrame, payload);
	if (payload.Length() < KMinLengthGetFolderItemsPdu)
		{
		return KErrAvrcpAirInvalidCommand;
		}
	
	TInt err = KErrAvrcpInvalidScope;
	switch(iFrame[3])
		{
	case AvrcpBrowsing::KMediaPlayerScope:
		{
		iOperationId = EAvrcpInternalGetFolderItems;
		iInterfaceUid = TUid::Uid(KUidAvrcpInternalInterface);
		err = (AppendIncomingPayload(payload) == KErrNone) ? KErrAvrcpInternalCommand : err;
		break;
		}
	case AvrcpBrowsing::KVirtualFilesystemScope:
	case AvrcpBrowsing::KSearchScope:
	case AvrcpBrowsing::KNowPlayingScope:
		{
		iOperationId = EGetFolderItemsOperationId;
		iInterfaceUid = TUid::Uid(KRemConMediaBrowseApiUid);
		err = AppendIncomingPayload(payload);
		break;
		}
		};
	
	return err;
	}

TInt CBrowseCommand::HandleChangePath()
	{
	LOG_FUNC	
	
	TPtrC8 payload;
	AvrcpBrowsing::BrowsingFrame::Payload(iFrame, payload);
	if (payload.Length() < KMinLengthChangePathPdu)
		{
		return KErrAvrcpAirInvalidCommand;
		}
		
	iOperationId = EChangePathOperationId;
	iInterfaceUid = TUid::Uid(KRemConMediaBrowseApiUid);
	return AppendIncomingPayload(payload);
	}

TInt CBrowseCommand::HandleGetItemAttributes()
	{
	LOG_FUNC	
	
	TPtrC8 payload;
	AvrcpBrowsing::BrowsingFrame::Payload(iFrame, payload);
	if (payload.Length() < KMinLengthGetItemAttributesPdu)
		{
		return KErrAvrcpAirInvalidCommand;
		}
	
	iInterfaceUid = TUid::Uid(KRemConMediaBrowseApiUid);
	iOperationId = EGetItemAttributesOperationId;
	return AppendIncomingPayload(payload);
	}

TInt CBrowseCommand::HandleSearch()
	{
	LOG_FUNC	
	
	TPtrC8 payload;
	AvrcpBrowsing::BrowsingFrame::Payload(iFrame, payload);
	if (payload.Length() < KMinLengthSearchPdu)
		{
		return KErrAvrcpAirInvalidCommand;
		}
		
	iOperationId = ESearchOperationId;
	iInterfaceUid = TUid::Uid(KRemConMediaBrowseApiUid);
	return AppendIncomingPayload(payload);
	}

/** Allocate correct space and append the payload to iCommandData
 */
TInt CBrowseCommand::AppendIncomingPayload(const TPtrC8& aPayload)
	{
	iCommandData.Close();
	if (iCommandData.Create(aPayload.Length()) != KErrNone)
		{
		return KErrAvrcpAirInternalError;
		}

	iCommandData.Append(aPayload);

	return KErrNone;
	}

TInt CBrowseCommand::HandleUnknownPdu()
	{
	iFrame.Close();
	TInt err = iFrame.CreateMax(KMinLengthGeneralReject);
	if(!err)
		{
		iFrame[0] = AvrcpBrowsing::EGeneralReject;
		iFrame[1] = (KMinLengthGeneralReject - AvrcpBrowsing::KHeaderLength) >> 8;
		iFrame[2] = (KMinLengthGeneralReject - AvrcpBrowsing::KHeaderLength);
		iFrame[3] = KErrAvrcpAirInvalidCommand - KErrAvrcpAirBase;
		}
	return err ? err : KErrAvrcpHandledInternallyRespondNow;
	}
