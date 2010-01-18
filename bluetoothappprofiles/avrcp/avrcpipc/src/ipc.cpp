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
 @internalTechnology
 @released
*/

#include <e32debug.h>
#include <remconmediaerror.h>
#include "avrcpipcutils.h"
#include "avrcpinternalinterface.h"
#include "mediabrowse.h"
#include "mediainformation.h"
#include "nowplaying.h"
#include "playerinformation.h"

/*
 * These methods are intended to provide structured IPC communication between
 * the AVRCP bearer, and the client API DLLs. They are intended to be 'simple'
 * classes, in which the basic operation are as follows:
 *
 *   reading: call ReadL(request) and pull information out of member variables
 *   writing: put information into member variables, then call WriteL(request)
 *
 * The ReadL() and WriteL() methods effectively just serialize the information
 * already stored in the member varables. However, they serialize in Big-Endian
 * format, so that after calling WriteL() and then sending the response back to
 * the AVRCP bearer via IPC, the bearer can just append the response straight
 * into an AVC frame; no bit-twiddling is necessary.
 *
 * Therefore it is important that these classes _ALWAYS_ write in Big-Endian
 * format, and write out responses that conform to the AVRCP specification for
 * PDUs listed in sections 5.1 - 5.4 of the AVRCP 1.3 specification. The ReadL()
 * operation is the inverse of the WriteL() operation and reads an AVRCP request
 * in the format listed in the AVRCP specification.
 *
 * There's also a Size() method, which will return the size of the response
 * before WriteL() is called. This is required particularly for responses with
 * text strings so that the correct-sized buffer can be allocated, as these
 * could be very large responses (theoretically up to 16 megabytes!) sent via
 * IPC (just once) and then kept and fragmented in the AVRCP bearer. The Close()
 * method frees up any memory allocated with these classes.
 */


// --------------------------------------------------------------------------------
// Used for constructing PDU 0x30 responses

EXPORT_C void RRemConPlayerInformationGetPlayStatusResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iTrackLength      = Read32L();
	iPlayPos          = Read32L();
	iStatus           = (MPlayerEventsObserver::TPlaybackStatus) Read8L();
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerInformationGetPlayStatusResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	Write32L(iTrackLength);
	Write32L(iPlayPos);
	Write8L(iStatus);
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for constructing PDU 0xff requests

EXPORT_C void RRemConPlayerInformationGetPlayStatusUpdateRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iStatus           = (MPlayerEventsObserver::TPlaybackStatus) Read8L();
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerInformationGetPlayStatusUpdateRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write8L(iStatus);
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for constructing PDU 0xff responses

EXPORT_C void RRemConPlayerInformationGetPlayStatusUpdateResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iStatus           = (MPlayerEventsObserver::TPlaybackStatus) Read8L();
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerInformationGetPlayStatusUpdateResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	Write8L(iStatus);
	iStream.CommitL();
	}
// --------------------------------------------------------------------------------
// Used for constructing PDU 0x15 and PDU 0x16 responses

EXPORT_C void RRemConGetPlayerApplicationTextResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iNumberAttributes = Read8L();
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		RSettingWithCharset setting;
		CleanupClosePushL(setting);
		setting.iAttributeId = Read8L();
		setting.iCharset     = Read16L();
		setting.iStringLen   = Read8L();
		setting.iString      = HBufC8::NewL(setting.iStringLen);
		TPtr8 ptr            = setting.iString->Des();
		iReadStream.ReadL(ptr);
		iAttributes.AppendL(setting);
		CleanupStack::Pop(&setting);
		}
	
	}

EXPORT_C void RRemConGetPlayerApplicationTextResponse::WriteL(TDes8& aOutData)
	{
	// The caller should have called Size() to pre-allocate enough buffer space
	__ASSERT_DEBUG(aOutData.MaxLength() >= Size(), AvrcpIpcUtils::Panic(EAvrcpIpcCommandDataTooLong));
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	Write8L(iNumberAttributes);
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write8L(iAttributes[i].iAttributeId);
		Write16L(iAttributes[i].iCharset);
		Write8L(iAttributes[i].iStringLen);
		iStream.WriteL(iAttributes[i].iString->Des());
		}
	iStream.CommitL();
	}

EXPORT_C TInt RRemConGetPlayerApplicationTextResponse::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	
	TInt size = 5; // 5 bytes: status code + number attributes
	for (TInt i = 0; i < iNumberAttributes; i++)
		{
		size += 4;   // 4 bytes: attribute id + charset + stringlen
		size += iAttributes[i].iString->Length();
		}
	return size;
	}

EXPORT_C void RRemConGetPlayerApplicationTextResponse::Close()
	{
	for (TInt i = 0; i < iAttributes.Count(); i++)
		{
		iAttributes[i].Close();
		}
	iAttributes.Close();
	RAvrcpIPC::Close();
	}

// --------------------------------------------------------------------------------
// Used for constructing PDU 0x10 responses

EXPORT_C void RRemConGetCapabilitiesResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iCapabilityId     = Read8L();
	if (   iCapabilityId != ECapabilityIdCompanyID
	    && iCapabilityId != ECapabilityIdEventsSupported)
		{
		iReadStream.Close();
		User::Leave(KErrNotSupported);
		}
	
	iCapabilityCount  = Read8L();
	for (TInt i = 0; i < iCapabilityCount; i++ )
		{
		if (iCapabilityId == ECapabilityIdCompanyID)
			{
			iCapabilities.AppendL(Read24L());  // Read 3 bytes
			}
		else
			{
			iCapabilities.AppendL(Read8L());   // Read 1 byte
			}
		}
	iReadStream.Close();
	}

EXPORT_C void RRemConGetCapabilitiesResponse::WriteL(TDes8& aOutData)
	{
	if (   iCapabilityId != ECapabilityIdCompanyID
	    && iCapabilityId != ECapabilityIdEventsSupported)
		{
		User::Leave(KErrNotSupported);
		}
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	Write8L(iCapabilityId);
	Write8L(iCapabilityCount);
	for (TInt i = 0; i < iCapabilityCount; i++ )
		{
		if (iCapabilityId == ECapabilityIdCompanyID)
			{
			Write24L(iCapabilities[i]);  // Write 3 bytes
			}
		else
			{
			Write8L(iCapabilities[i]);   // Write 1 byte
			}
		}
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for constructing and parsing PDU 0x13 (response) and PDU 0x14 (request)

EXPORT_C void RRemConPlayerAttributeIdsAndValues::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iNumberAttributes = Read8L();
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		TInt attributeId     = Read8L();
		TInt attributeValue = Read8L();
		iAttributeId.AppendL(attributeId);
		iAttributeValue.AppendL(attributeValue);
		}
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerAttributeIdsAndValues::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	Write8L(iNumberAttributes);
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write8L(iAttributeId[i]);
		Write8L(iAttributeValue[i]);
		}
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for constructing PDU 0x11 and PDU 0x12 responses

EXPORT_C void RRemConPlayerListOfAttributes::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iNumberAttributes = Read8L();
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		TInt attribute = Read8L();
		iAttributes.AppendL(attribute);
		}
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerListOfAttributes::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	iAttributes.Sort();
	Write32L(KErrNone);   // Successful operation
	Write8L(iNumberAttributes);
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write8L(iAttributes[i]);
		}
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for parsing PDU 0x20 requests

EXPORT_C void RRemConGetElementAttributesRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iElement = Read64L();
	iNumberAttributes = Read8L();
	
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		TInt attribute = Read32L();
		iAttributes.AppendL(attribute);
		}
	iReadStream.Close();
	}

EXPORT_C void RRemConGetElementAttributesRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	iAttributes.Sort();
	Write32L(KErrNone);   // Successful operation
	
	// 64 bits of data
	Write64L(iElement);
	Write8L(iNumberAttributes);

	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write32L(iAttributes[i]);
		}
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for PDU 0x20 responses

EXPORT_C void RRemConGetElementAttributesResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iNumberAttributes = Read8L();
	
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		REAResponse eattr;
		CleanupClosePushL(eattr);
		eattr.iAttributeId = Read32L();
		eattr.iCharset     = Read16L();
		eattr.iStringLen   = Read16L();
		eattr.iString      = HBufC8::NewL(eattr.iStringLen);
		TPtr8 ptr          = eattr.iString->Des();
		iReadStream.ReadL(ptr);
		iAttributes.AppendL(eattr);
		CleanupStack::Pop(&eattr);
		}
	
	}

EXPORT_C void RRemConGetElementAttributesResponse::WriteL(TDes8& aOutData)
	{
	__ASSERT_DEBUG(aOutData.MaxLength() >= Size(), AvrcpIpcUtils::Panic(EAvrcpIpcCommandDataTooLong));
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write8L(iNumberAttributes);

	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write32L(iAttributes[i].iAttributeId);
		Write16L(iAttributes[i].iCharset);
		Write16L(iAttributes[i].iStringLen);
		iStream.WriteL(iAttributes[i].iString->Des());
		}
	iStream.CommitL();
	}

EXPORT_C TInt RRemConGetElementAttributesResponse::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	
	TInt size = 5; // 5 bytes: status code + number attributes
	for (TInt i = 0; i < iNumberAttributes; i++)
		{
		size += 4+2+2; // 8 bytes: attrId (4 bytes) + charset (2 bytes) + stringlen (2 bytes)
		size += iAttributes[i].iString->Length();
		}
	return size;
	}

EXPORT_C void RRemConGetElementAttributesResponse::Close()
	{
	for (TInt i = 0; i < iAttributes.Count(); i++)
		{
		iAttributes[i].Close();
		}
	iAttributes.Close();
	RAvrcpIPC::Close();
	}

// --------------------------------------------------------------------------------

EXPORT_C void RRemConPlayerInformation8BitResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iValue = Read8L();
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerInformation8BitResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write8L(iValue);
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------

EXPORT_C void RRemConPlayerInformation32BitResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iValue = Read32L();
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerInformation32BitResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write32L(iValue);
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------

EXPORT_C void RRemConPlayerInformation64BitResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iValue = Read64L();
	iReadStream.Close();
	}

EXPORT_C void RRemConPlayerInformation64BitResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write64L(iValue);
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------

EXPORT_C void RSettingWithCharset::Close()
	{
	delete iString;
	iString = NULL;
	}

EXPORT_C void REAResponse::Close()
	{
	delete iString;
	iString = NULL;
	}

EXPORT_C void RRemConGetCapabilitiesResponse::Close()
	{
	iCapabilities.Close();
	}

EXPORT_C void RRemConPlayerListOfAttributes::Close()
	{
	iAttributes.Close();
	}

EXPORT_C void RRemConGetElementAttributesRequest::Close()
	{
	iAttributes.Close();
	}

EXPORT_C void RRemConPlayerAttributeIdsAndValues::Close()
	{
	iAttributeId.Close();
	iAttributeValue.Close();
	}

EXPORT_C void RItem::Close()
	{
	delete iName;
	for(TInt i = 0; i<iAttributes.Count(); i++)
		{
		iAttributes[i].Close();
		}
	}

EXPORT_C TInt RItem::Size()
	{
	// Base size:
	// type + length field + length
	return 3 + iLength;
	}

EXPORT_C void RMediaPlayerItem::Close()
	{
	iName.Close();
	}

EXPORT_C TInt RMediaPlayerItem::Size()
	{
	// Base size:
	// type + length field + length
	return 3 + iLength;
	}

/**
Parses PDU 0x74 and 0x90 requests.
*/
EXPORT_C void RRemConNowPlayingRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iScope = static_cast<TRemConFolderScope>(Read8L());
	iElement = Read64L();
	iUidCounter = Read16L();
	}

/**
Constructs PDU 0x74 and 0x90 requests.
*/
EXPORT_C void RRemConNowPlayingRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	Write8L(iScope);
	Write64L(iElement);
	Write16L(iUidCounter);
	iStream.CommitL();
	}

/**
Parses PDU 0x74 and 0x90 responses
*/
EXPORT_C void RRemConNowPlayingResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iStatus = Read8L();
	}

/**
Constructs PDU 0x74 and 0x90 responses.
*/
EXPORT_C void RRemConNowPlayingResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	if ((KErrAvrcpAirBase - KErrAvrcpAirSuccess) == iStatus)
	    {
	    Write32L(KErrNone);   // Successful operation
	    Write8L(iStatus);
	    }
	else
		{
		Write32L(KErrAvrcpAirBase - iStatus);
		}
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for parsing PDU 0x71 requests
EXPORT_C void RRemConGetFolderItemsRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iScope = Read8L();
	iStartItem = Read32L();
	iEndItem = Read32L();
	iNumberAttributes = Read8L();
	
	if (iNumberAttributes == 0)
		{
		// spec says this is a request for all attribs
		// current spec has 7 specified (0x01 to 0x07)
		for (TInt i = 1; i <= KMaxMediaAttributeValue; i++)
			{
			iAttributes.AppendL(i);
			}
		}
	else if (iNumberAttributes == 0xff)
		{
		// No attributes requested
		}
	else
		{
		for (TInt i = 0; i < iNumberAttributes; i++ )
			{
			TInt attribute = Read32L();
			if (attribute > 0 && attribute <= KMaxMediaAttributeValue )
				{
				iAttributes.AppendL(attribute);
				}
			}
		}

	iReadStream.Close();
	}

EXPORT_C void RRemConGetFolderItemsRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	iAttributes.Sort();
	Write32L(KErrNone);   // Successful operation
	
	Write8L(iScope);
	Write32L(iStartItem);
	Write32L(iEndItem);
	Write8L(iNumberAttributes);

	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write32L(iAttributes[i]);
		}

	iStream.CommitL();
	}

EXPORT_C void RRemConGetFolderItemsRequest::Close()
	{
	iAttributes.Close();
	}

EXPORT_C TInt RRemConGetFolderItemsRequest::CopyAttributes(RArray<TMediaAttributeId>& aOutAttributes)
	{
	TInt result = KErrNone;
	TMediaAttributeId attributeId;
	TInt attributeCount = iAttributes.Count();
	for (TInt i = 0; i < attributeCount; i++)
		{
		attributeId = static_cast<TMediaAttributeId>(iAttributes[i]);
		result = aOutAttributes.Append(attributeId);
		if (result != KErrNone)
			{
			break;
			}
		}
	return result;
	}
// --------------------------------------------------------------------------------
// Used for PDU 0x71 responses

EXPORT_C void RRemConGetFolderItemsResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iPduId = Read8L();
	iParamLength = Read16L();
	iStatus = Read8L();
	iUidCounter = Read16L();
	iNumberItems = Read16L();
	
	for(TInt i = 0; i < iNumberItems; i++)
		{
		ReadItemL();
		}
	
	iReadStream.Close();
	}

void RRemConGetFolderItemsResponse::ReadItemL()
	{
	RItem item;
	CleanupClosePushL(item);
	
	item.iType = static_cast<AvrcpBrowsing::TItemType>(Read8L());
	item.iLength = Read16L();
	item.iUid = Read64L();
	
	if(item.iType == AvrcpBrowsing::EFolderItem)
		{
		item.iFolderType = static_cast<AvrcpBrowsing::TFolderType>(Read8L());
		item.iPlayable = Read8L();
		}
	else 
		{
		item.iMediaType = Read8L();
		}
	
	item.iCharset = Read16L();
	item.iNameLength = Read16L();
	item.iName = HBufC8::NewL(item.iNameLength);
	TPtr8 ptr = item.iName->Des();
	iReadStream.ReadL(ptr);
	
	if(item.iType == AvrcpBrowsing::EMediaElement)
		{
		item.iNumberAttributes = Read32L();
		
		for (TInt i = 0; i < item.iNumberAttributes; i++ )
			{
			REAResponse eattr;
			CleanupClosePushL(eattr);
			
			eattr.iAttributeId = Read32L();
			eattr.iCharset     = Read16L();
			eattr.iStringLen   = Read16L();
			eattr.iString      = HBufC8::NewL(eattr.iStringLen);
			TPtr8 ptr          = eattr.iString->Des();
			iReadStream.ReadL(ptr);
			
			item.iAttributes.AppendL(eattr);
			CleanupStack::Pop(&eattr);
			}
		}
	
	iItems.AppendL(item);
	CleanupStack::Pop(&item);
	}

EXPORT_C void RRemConGetFolderItemsResponse::WriteL(TDes8& aOutData)
	{
	__ASSERT_DEBUG(aOutData.MaxLength() >= Size(), AvrcpIpcUtils::Panic(EAvrcpIpcCommandDataTooLong));
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write8L(iPduId);
	Write16L(iParamLength);
	Write8L(iStatus);
	
	if(iStatus == 0x4)
		{
		Write16L(iUidCounter);
		Write16L(iNumberItems);
	
		for (TInt i = 0; i < iNumberItems; i++ )
			{
			WriteItemL(i);
			}
		}
	
	iStream.CommitL();
	}

void RRemConGetFolderItemsResponse::WriteItemL(TInt aIndex)
	{
	RItem& item = iItems[aIndex];
	
	Write8L(item.iType);
	Write16L(item.iLength);
	Write64L(item.iUid);
	
	if(item.iType == AvrcpBrowsing::EFolderItem)
		{
		Write8L(item.iFolderType);
		Write8L(item.iPlayable);
		}
	else
		{
		Write8L(item.iMediaType);
		}
	
	Write16L(item.iCharset);
	Write16L(item.iNameLength);
	iStream.WriteL(item.iName->Des());
	
	if(item.iType == AvrcpBrowsing::EMediaElement)
		{
		Write8L(item.iNumberAttributes);
		
		for(TInt i = 0; i < item.iNumberAttributes; i++)
			{
			Write32L(item.iAttributes[i].iAttributeId);
			Write16L(item.iAttributes[i].iCharset);
			Write16L(item.iAttributes[i].iStringLen);
			iStream.WriteL(item.iAttributes[i].iString->Des());
			}
		}
	}

EXPORT_C TInt RRemConGetFolderItemsResponse::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	
	// base size 
	// pduid + paramlength + status + uidcount + number items
	TInt size = KGetFolderItemsResponseBaseSize;
	
	for (TInt i = 0; i < iNumberItems; i++)
		{
		size += iItems[i].Size();
		}
	return size;
	}

EXPORT_C void RRemConGetFolderItemsResponse::Close()
	{
	for (TInt i = 0; i < iItems.Count(); i++)
		{
		iItems[i].Close();
		}
	iItems.Reset();
	}

EXPORT_C TInt RRemConGetFolderItemsResponse::CopyItems(
		const TArray<TRemConItem>& aItems)
	{
	TInt err = KErrNone;
	for(TInt i = 0; i < aItems.Count(); i++)
		{
		RItem item;
		item.iName = NULL;
		item.iUid = aItems[i].iUid;
		item.iType = static_cast<AvrcpBrowsing::TItemType>(aItems[i].iType);
		
		err = iItems.Append(item);
		if(err)
			{
			break;
			}
		}
	return err;
	}

EXPORT_C TBool RRemConGetFolderItemsResponse::RequestNextItem(TInt& aError, 
		RBuf8& aOutBuf, TUint16 aCookie)
	{
	aError = KErrNone;
	// Work out if we have as many items as fit
	TBool getMoreItems = ETrue;
	iCurrentListingSize += (iCurrentItem + 1) ? iItems[iCurrentItem].Size() : 0;
	if(iCurrentListingSize > iMaxResponse)
		{
		// We can't fit this item in the listing
		iItems[iCurrentItem].Close();
		getMoreItems = EFalse;
		iItems.Remove(iCurrentItem);
		}
	
	if(getMoreItems && ++iCurrentItem < iItems.Count())
		{
		return ETrue;
		}
	else
		{
		// We have all items, send the response
		iNumberItems = iCurrentItem;
		if(aOutBuf.Create(Size()) != KErrNone)
			{
			aError = KErrGeneral;
			return EFalse;
			}
		
		iPduId = AvrcpBrowsing::EGetFolderItems;
		iStatus = KErrAvrcpAirBase - KErrAvrcpAirSuccess;//0x4
		iUidCounter = aCookie;
		TRAP(aError, WriteL(aOutBuf));
		}
	return EFalse;
	}
// --------------------------------------------------------------------------------
// Used for parsing PDU 0x72 requests

EXPORT_C void RRemConChangePathRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iUidCounter = Read16L();
	iDirection = Read8L();
	
	if(iDirection == AvrcpBrowsing::KDown)
		{
		iElement = Read64L();
		}

	iReadStream.Close();
	}

EXPORT_C void RRemConChangePathRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write16L(iUidCounter);
	Write8L(iDirection);
	Write64L(iElement);

	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for PDU 0x72 responses

EXPORT_C void RRemConChangePathResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iStatus = Read8L();
	iNumberItems = Read32L();
	
	iReadStream.Close();
	}

EXPORT_C void RRemConChangePathResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write8L(iPduId);
	Write16L(iParamLength);
	Write8L(iStatus);
	
	if(iStatus == 0x4)
		{
		Write32L(iNumberItems);
		}
		
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for parsing PDU 0x73 requests

EXPORT_C void RRemConGetItemAttributesRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iScope = Read8L();
	iElement = Read64L();
	iUidCounter = Read16L();
	iNumberAttributes = Read8L();
	
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		TInt attribute = Read32L();
		iAttributes.AppendL(attribute);
		}
	
	iReadStream.Close();
	}

EXPORT_C void RRemConGetItemAttributesRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	iAttributes.Sort();
	Write32L(KErrNone);   // Successful operation
	
	Write8L(iScope);
	// 64 bits of data
	Write64L(iElement);
	Write16L(iUidCounter);
	Write8L(iNumberAttributes);

	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		Write32L(iAttributes[i]);
		}
	iStream.CommitL();
	}

EXPORT_C void RRemConGetItemAttributesRequest::Close()
	{
	iAttributes.Close();
	}

// --------------------------------------------------------------------------------
// Used for PDU 0x73 responses

EXPORT_C void RRemConGetItemAttributesResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iStatus = Read8L();
	iNumberAttributes = Read8L();
	
	for (TInt i = 0; i < iNumberAttributes; i++ )
		{
		REAResponse eattr;
		eattr.iAttributeId = Read32L();
		eattr.iCharset     = Read16L();
		eattr.iStringLen   = Read16L();
		eattr.iString      = HBufC8::NewL(eattr.iStringLen);
		TPtr8 ptr          = eattr.iString->Des();
		iReadStream.ReadL(ptr);
		iAttributes.AppendL(eattr);
		}
	iReadStream.Close();
	}

EXPORT_C void RRemConGetItemAttributesResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write8L(iPduId);
	Write16L(iParamLength);
	Write8L(iStatus);
	
	if(iStatus == 0x4)
		{
		Write8L(iNumberAttributes);
	
		for (TInt i = 0; i < iNumberAttributes; i++ )
			{
			Write32L(iAttributes[i].iAttributeId);
			Write16L(iAttributes[i].iCharset);
			Write16L(iAttributes[i].iStringLen);
			iStream.WriteL(iAttributes[i].iString->Des());
			}
		}
	iStream.CommitL();
	}

EXPORT_C TInt RRemConGetItemAttributesResponse::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	
	TInt size = 9; // 6 bytes: error + pduid + paramlength + status code + number attributes
	for (TInt i = 0; i < iAttributes.Count(); i++)
		{
		size += 4+2+2; // 8 bytes: attrId (4 bytes) + charset (2 bytes) + stringlen (2 bytes)
		size += iAttributes[i].iString->Length();
		}
	return size;
	}

EXPORT_C void RRemConGetItemAttributesResponse::Close()
	{
	for (TInt i = 0; i < iAttributes.Count(); i++)
		{
		iAttributes[i].Close();
		}
	iAttributes.Close();
	}

// --------------------------------------------------------------------------------
// Used for parsing PDU 0x80 requests

EXPORT_C void RRemConSearchRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iCharset = Read16L();
	iStringLen = Read16L();
	
	TInt dataLength = aData.Length();
	TPtrC8 pString = aData.Right(dataLength - 2*sizeof(TUint16));
	if (pString.Length() != iStringLen)
		{
		User::Leave(KErrArgument);
		}
	
	RBuf8 searchBuf;
	searchBuf.CreateL(iStringLen);
	CleanupClosePushL(searchBuf);
	
	iReadStream.ReadL(searchBuf);
	
	CleanupStack::Pop(&searchBuf);
	iSearchString.Assign(searchBuf);

	iReadStream.Close();
	}

EXPORT_C void RRemConSearchRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write16L(iCharset);
	Write16L(iStringLen);
	WriteL(iSearchString);
	
	iStream.CommitL();
	}

EXPORT_C TInt RRemConSearchRequest::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	TInt size = sizeof(iCharset) + sizeof(iStringLen) + iSearchString.Length(); 
	
	return size;
	}

EXPORT_C void RRemConSearchRequest::Close()
	{
	iSearchString.Close();
	}

// --------------------------------------------------------------------------------
// Used for PDU 0x80 responses

EXPORT_C void RRemConSearchResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iStatus = Read8L();
	iUidCounter = Read16L();
	iNumberItems = Read32L();
	
	iReadStream.Close();
	}

EXPORT_C void RRemConSearchResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write8L(iPduId);
	Write16L(iParamLength);
	Write8L(iStatus);
	
	if(iStatus == 0x4)
		{
		Write16L(iUidCounter);
		Write32L(iNumberItems);
		}

	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for parsing SetBrowsedPlayer requests

EXPORT_C void RRemConSetBrowsedPlayerRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	// iMaxResponse is little endian
	iMaxResponse = iReadStream.ReadInt32L();
	iPlayerId = Read16L();
	
	iReadStream.Close();
	}

EXPORT_C void RRemConSetBrowsedPlayerRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	// Use little endian on iMaxResponse
	iStream.WriteInt32L(iMaxResponse);
	Write16L(iPlayerId);
	
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for SetBrowsedPlayer responses

EXPORT_C void RRemConGetPathResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iStatus = Read8L();
	iUidCounter = Read16L();
	iNumberItems = Read32L();
	
	iReadStream.Close();
	}

EXPORT_C void RRemConGetPathResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write8L(iPduId);
	Write16L(iParamLength);
	Write8L(iStatus);
	
	if(iStatus != 0x4)
		{
		iStream.CommitL();
		return;
		}
    
	// The depth is equal to the number of items in the array.
	iFolderDepth = iPath.Count();
	Write16L(iUidCounter);
	Write32L(iNumberItems);
	Write16L(KUtf8MibEnum);
	Write8L(iFolderDepth);
	
	for(TInt i = 0; i < iFolderDepth; i++)
		{
		Write16L((iPath[i])->Length());
		iStream.WriteL(*(iPath[i]));
		}
	
	iStream.CommitL();
	}

EXPORT_C TInt RRemConGetPathResponse::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	iFolderDepth = iPath.Count();
	TInt size = 13 + (iFolderDepth*2);
	
	for(TInt i = 0; i < iFolderDepth; i++)
		{
		size += (iPath[i])->Length();
		}
	
	return size;
	}

EXPORT_C void RRemConGetPathResponse::Close()
	{
	iPath.ResetAndDestroy();
	}

// --------------------------------------------------------------------------------
// Used for parsing SetAddressedPlayer requests

EXPORT_C void RAvrcpSetAddressedPlayerRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iPlayerId = Read16L();
	
	iReadStream.Close();
	}

EXPORT_C void RAvrcpSetAddressedPlayerRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write16L(iPlayerId);
	
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for SetAddressedPlayer responses

EXPORT_C void RAvrcpSetAddressedPlayerResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iStatus = Read8L();
	
	iReadStream.Close();
	}

EXPORT_C void RAvrcpSetAddressedPlayerResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write32L(KErrNone);   // Successful operation
	Write8L(iStatus);
	
	iStream.CommitL();
	}

// --------------------------------------------------------------------------------
// Used for parsing PDU 0x71 requests with media player list scope

EXPORT_C void RAvrcpGetFolderItemsRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iScope = Read8L();
	iStartItem = Read32L();
	iEndItem = Read32L();
	}

EXPORT_C void RAvrcpGetFolderItemsRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write8L(iScope);
	Write32L(iStartItem);
	Write32L(iEndItem);

	iStream.CommitL();
	}

EXPORT_C void RAvrcpGetFolderItemsRequest::Close()
	{
	RAvrcpIPC::Close();
	}

// --------------------------------------------------------------------------------
// Used for PDU 0x71 responses with media player list scope

EXPORT_C void RAvrcpGetFolderItemsResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	
	iPduId = Read8L();
	iParamLength = Read16L();
	iStatus = Read8L();
	iUidCounter = Read16L();
	iNumberItems = Read16L();
	
	for(TInt i = 0; i < iNumberItems; i++)
		{
		ReadItemL();
		}
	
	iReadStream.Close();
	}

void RAvrcpGetFolderItemsResponse::ReadItemL()
	{
	RMediaPlayerItem item;
	CleanupClosePushL(item);
	
	item.iType = static_cast<AvrcpBrowsing::TItemType>(Read8L());
	item.iLength = Read16L();
	item.iPlayerId = Read16L();
	
	item.iPlayerType = Read8L();
	item.iPlayerSubType = Read32L();
	item.iPlayStatus = Read8L();
	iReadStream.ReadL(item.iFeatureBitmask);
	
	item.iCharset = Read16L();
	item.iNameLength = Read16L();
	
	item.iName.CreateL(item.iNameLength);
	iReadStream.ReadL(item.iName);
	
	iItems.AppendL(item);
	CleanupStack::Pop(&item);
	}

EXPORT_C void RAvrcpGetFolderItemsResponse::WriteL(TDes8& aOutData)
	{
	__ASSERT_DEBUG(aOutData.MaxLength() >= Size(), AvrcpIpcUtils::Panic(EAvrcpIpcCommandDataTooLong));
	aOutData.Zero();
	iStream.Open(aOutData);
	
	Write8L(iPduId);
	Write16L(iParamLength);
	Write8L(iStatus);
	
	if(iStatus == 0x4)
		{
		Write16L(iUidCounter);
		Write16L(iNumberItems);
	
		for (TInt i = 0; i < iNumberItems; i++ )
			{
			WriteItemL(i);
			}
		}
	
	iStream.CommitL();
	}

void RAvrcpGetFolderItemsResponse::WriteItemL(TInt aIndex)
	{
	RMediaPlayerItem& item = iItems[aIndex];
	
	Write8L(item.iType);
	Write16L(item.iLength);
	Write16L(item.iPlayerId);
	
	Write8L(item.iPlayerType);
	Write32L(item.iPlayerSubType);
	Write8L(item.iPlayStatus);
	iStream.WriteL(item.iFeatureBitmask);
	
	Write16L(item.iCharset);
	Write16L(item.iNameLength);
	iStream.WriteL(item.iName);
	}

EXPORT_C TInt RAvrcpGetFolderItemsResponse::Size()
	{
	// Return the size that a buffer needs to be allocated to
	// serialise the data encapsulated within this data structure.
	
	// base size 
	// pduid + paramlength + status + uidcount + number items
	TInt size = KGetFolderItemsResponseBaseSize;
	
	for (TInt i = 0; i < iNumberItems; i++)
		{
		size += iItems[i].Size();
		}
	return size;
	}

EXPORT_C void RAvrcpGetFolderItemsResponse::Close()
	{
	for (TInt i = 0; i < iNumberItems; i++)
		{
		iItems[i].Close();
		}
	iItems.Reset();
	}

// --------------------------------------------------------------------------------
// Used for PDU 0x30 responses with event addressed player changed

EXPORT_C void RAvrcpAddressedPlayerNotificationResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iPlayerId = Read16L();
	iUidCounter = Read16L();
	iReadStream.Close();
	}

EXPORT_C void RAvrcpAddressedPlayerNotificationResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write16L(iPlayerId);
	Write16L(iUidCounter);
	iStream.CommitL();
	}

EXPORT_C void RAvrcpUidCounterNotificationResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iUidCounter = Read16L();
	iReadStream.Close();
	}

EXPORT_C void RAvrcpUidCounterNotificationResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write32L(KErrNone);   // Successful operation
	
	Write16L(iUidCounter);
	iStream.CommitL();
	}

EXPORT_C void RRemConMediaErrorResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iPduId = Read8L();
	iLength = Read16L();
	iStatus = Read8L();
	iReadStream.Close();
	}

EXPORT_C void RRemConMediaErrorResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write8L(iPduId);
	Write16L(KErrResponseParameterLength);
	Write8L(iStatus);
	iStream.CommitL();
	}

EXPORT_C TUint8 RAvrcpIPC::SymbianErrToStatus(TInt aErr)
	{
	return KErrAvrcpAirBase - SymbianErrorCheck(aErr);
	}

/**
Ensure we get a known error.
*/
EXPORT_C TInt RAvrcpIPC::SymbianErrorCheck(TInt aError)
	{
	TInt error = aError;
	
	switch (error)
		{
		case KErrNone:
			error = KErrAvrcpAirSuccess;
			break;
		case KErrAvrcpAirInvalidCommand:
		case KErrAvrcpAirInvalidParameter:
		case KErrAvrcpAirParameterNotFound:
		case KErrAvrcpAirInternalError:
		case KErrAvrcpAirSuccess:
		case KErrAvrcpAirUidChanged:
		case KErrAvrcpAirReserved:
		case KErrAvrcpAirInvalidDirection:
		case KErrAvrcpAirNotADirectory:
		case KErrAvrcpAirDoesNotExist:
		case KErrAvrcpAirInvalidScope:
		case KErrAvrcpAirRangeOutOfBounds:
		case KErrAvrcpAirUidIsADirectory:
		case KErrAvrcpAirMediaInUse:
		case KErrAvrcpAirNowPlayingListFull:
		case KErrAvrcpAirSearchNotSupported:
		case KErrAvrcpAirSearchInProgress:
		case KErrAvrcpAirInvalidPlayerId:
		case KErrAvrcpAirPlayerNotBrowesable:
		case KErrAvrcpAirPlayerNotAddressed:
		case KErrAvrcpAirNoValidSearchResults:
		case KErrAvrcpAirNoAvailablePlayers:
		case KErrAvrcpAirAddressedPlayerChanged:
		case KErrAvrcpInvalidScope:
			break;
		default:
			error = KErrAvrcpAirInternalError;
		}
	
	return error;
	}

EXPORT_C void RRemConUidsChangedRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iInitialUidCounter = Read16L();
	}

EXPORT_C void RRemConUidsChangedRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write16L(iInitialUidCounter);
	iStream.CommitL();
	}

EXPORT_C void RRemConUidsChangedResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iUidCounter = Read16L();
	}

EXPORT_C void RRemConUidsChangedResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	Write16L(iUidCounter);
	iStream.CommitL();
	}
