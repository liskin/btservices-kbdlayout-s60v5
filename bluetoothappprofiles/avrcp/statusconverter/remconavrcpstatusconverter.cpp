// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
 @released
*/

#include <avcframe.h>
#include <remconbeareravrcp.h>

#include "remconstatusapi.h"
#include "remconavrcpstatusconverter.h"

/** Factory function.

@return Ownership of a new CRemConAvrcpStatusConverter.
*/
CRemConAvrcpStatusConverter* CRemConAvrcpStatusConverter::NewL()
	{
	CRemConAvrcpStatusConverter* self = new(ELeave) CRemConAvrcpStatusConverter;
	return self;
	}

/** Destructor.
*/
CRemConAvrcpStatusConverter::~CRemConAvrcpStatusConverter()
	{
	}

/** Constructor.
*/
CRemConAvrcpStatusConverter::CRemConAvrcpStatusConverter()
	{
	}

/** Called to get a converter interface.

@param aUid The uid of the desired interface.
@return An instance of the desired interface, NULL if
		one could not be found.
*/
TAny* CRemConAvrcpStatusConverter::GetInterface(TUid aUid)
	{
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConConverterInterface1) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConConverterInterface*>(this)
			);
		}

	return ret;
	}

/** Decides whether this converter supports this interface.

This converter supports only the AVRCP bearer and Status API.
This function checks whether the Uids provided match AVRCP and
Status.

@param aInterfaceData The Uid of the originating RemCon interface.
@param aBearerUid The Uid of the bearer this data is destined for.
@return Whether this data can be converted.	
*/
TBool CRemConAvrcpStatusConverter::SupportedUids(const TUid& aInterfaceUid, const TUid& aBearerUid) const
	{
	return (aInterfaceUid == TUid::Uid(KRemConStatusApiUid) &&
	   aBearerUid == TUid::Uid(KRemConBearerAvrcpImplementationUid));
	}

/** Decides whether this converter supports this interface.

This converter supports only the AVRCP bearer.  That bearer publishes
interface data in the format of an AV/C frame.  This function 
checks that aBearerUid is that of AVRCP, and if so examines the AV/C
frame in aInterfaceData to see if it is a supported command.

@param aInterfaceData Data in the format of the bearer identified by
					  aBearerUid.  
@param aBearerUid The Uid of the bearer this data originates from.
@return Whether this data can be converted.
*/	
TBool CRemConAvrcpStatusConverter::SupportedInterface(const TDesC8& aInterfaceData, const TUid& aBearerUid) const
	{
	TInt supported = EFalse;
	if(aBearerUid == TUid::Uid(KRemConBearerAvrcpImplementationUid))
		{
		AVC::TOpcode opcode = AVC::EPower; // arbitrary initialisation to avoid compiler warning
		TRAPD(err, opcode = CAVCFrame::OpcodeL(aInterfaceData));
		
		if (		err == KErrNone
			&&		( opcode == AVC::EUnitInfo || opcode == AVC::ESubunitInfo )
			)
			{
			supported = ETrue;
			}
		}
		
	return supported;
	}

/** Convert data from the API's form (as we're given it by RemCon) to 
bearer-specific form.

We only deal with the AVRCP bearer.  This requires us to produce a
CAVCFrame.

The supported commands are Unit Info and Subunit Info.

@param aInterfaceUid The Uid of the originating RemCon interface.
@param aOperationId The id of the operation within the interface.
@param aData Data in the format defined by the RemCon interface.
@param aMsgType Whether this is a command or response.
@param aBearerData On return, an AV/C frame representing the operation.
*/
TInt CRemConAvrcpStatusConverter::InterfaceToBearer(TUid aInterfaceUid, 
		TUint aOperationId,
		const TDesC8& /*aData*/,
		TRemConMessageType aMsgType, 
		TDes8& aBearerData) const
	{
	__ASSERT_ALWAYS(aInterfaceUid.iUid == KRemConStatusApiUid, CRemConAvrcpStatusConverter::Panic(EStatusConverterWrongInterface));
	TInt err = KErrCorrupt;
	
	switch(aOperationId)
		{
		case ERemConStatusApiUnitInfo:
			{
			if(aMsgType == ERemConCommand)
				{
				CAVCFrame* frame = NULL;
				TRAP(err, frame = UnitInfoL());
				if(!err)
					{
					aBearerData = frame->Data();
					err = KErrNone;
					delete frame;
					}
				}
			else if(aMsgType == ERemConResponse)
				{
				// These should be generated within the bearer
				// as it knows the manufacturer id
				err = KErrNotSupported;
				}
			break;
			}
		case ERemConStatusApiSubunitInfo:
			{
			if(aMsgType == ERemConCommand)
				{
				CAVCFrame* frame = NULL;
				TRAP(err, frame = SubunitInfoL());
				if(!err)
					{
					aBearerData = frame->Data();
					err = KErrNone;
					delete frame;
					}
				}
			else if(aMsgType == ERemConResponse)
				{
				// These should be generated within the bearer
				// as it knows the manufacturer id
				err = KErrNotSupported;
				}
			break;
			}
		default:
			{
			err = KErrNotSupported;
			break;
			}
		}

	return err;
	}

/** Produce a Unit Info AV/C frame.

@return A new AV/C frame representing unit info.
@leave System wide error.
*/
CAVCFrame* CRemConAvrcpStatusConverter::UnitInfoL()
	{
	CAVCFrame* frame = CAVCFrame::NewL(AVC::ECommand, AVC::EStatus, AVC::EUnit, AVC::EIgnore);
	frame->Append(TChar(AVC::EUnitInfo));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	return frame;
	}

/** Produce a Subunit Info AV/C frame.

@return A new AV/C frame representing subunit info.
@leave System wide error.
*/	
CAVCFrame* CRemConAvrcpStatusConverter::SubunitInfoL()
	{
	CAVCFrame* frame = CAVCFrame::NewL(AVC::ECommand, AVC::EStatus, AVC::EUnit, AVC::EIgnore);
	frame->Append(TChar(AVC::ESubunitInfo));
	frame->Append(TChar(0x7));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	return frame;
	}

/** Convert data from the bearer format to RemCon interface format.

We only deal with the Status Api. The supported commands are Unit 
Info response and Subunit Info response.

Support of these commands is mandatory in the targets.  In the case
where a remote sends us something rubbish, we'll just provide our
client with the default info.

@param aBearerData An AV/C frame to convert.
@param aInterfaceUid On return, the Uid of the RemCon interface.
@param aOperationId On return, the id of the operation within the 
					interface.
@param aMsgType Whether this is a command or response.
@param aData On return, Data in the format defined by the RemCon interface.
@return Whether the command was successfully converted.
*/
TInt CRemConAvrcpStatusConverter::BearerToInterface(const TDesC8& aBearerData,
		TUid& aInterfaceUid, 
		TUint& aOperationId,
		TRemConMessageType& aMsgType, 
		TDes8& aData) const
	{
	TInt err = KErrCorrupt;
	
	switch(aOperationId)
		{
		case ERemConStatusApiUnitInfo:
			{
			if(aMsgType == ERemConCommand)
				{
				// These should be generated within the bearer
				// as it knows the manufacturer id
				err = KErrNotSupported;
				}
			else if(aMsgType == ERemConResponse)
				{
				// We try to parse the data, but if what the remote's sent
				// us is invalid we'll fill in sensible values later
				TRAP(err, SetUnitInfoResponseDataL(aBearerData, aData));
				aInterfaceUid = TUid::Uid(KRemConStatusApiUid);
				aOperationId = ERemConStatusApiUnitInfo;

				if(err == KErrCorrupt)
					{
					// The data is set to the correct length in SetUnitInfoResponseDataL
					// Stick some default data in there 
					SetCommandDataFromInt(aData, KRemConStatusApiUnitOffset, 
						KRemConStatusApiUnitLength, AVC::EID0);

					SetCommandDataFromInt(aData, KRemConStatusApiUnitTypeOffset, 
						KRemConStatusApiUnitTypeLength, AVC::EPanel);
				
					SetCommandDataFromInt(aData, KRemConStatusApiExtendedUnitTypeOffset, 
						KRemConStatusApiExtendedUnitTypeLength, 0);		
							
					SetCommandDataFromInt(aData, KRemConStatusApiVendorIdOffset, 
						KRemConStatusApiVendorIdLength, KRemConStatusApiDefaultVendorId);

					err = KErrNone;
					}
				}
			break;
			}
		case ERemConStatusApiSubunitInfo:
			{
			if(aMsgType == ERemConCommand)
				{
				// These should be generated within the bearer
				// as it knows the manufacturer id
				err = KErrNotSupported;
				}
			else if(aMsgType == ERemConResponse)
				{
				// We try to parse the data, but if what the remote's sent
				// us is invalid we'll fill in sensible values later
				TRAP(err, SetSubunitInfoResponseDataL(aBearerData, aData));
				aInterfaceUid = TUid::Uid(KRemConStatusApiUid);
				aOperationId = ERemConStatusApiSubunitInfo;

				if(err == KErrCorrupt)
					{
					// Subunit response data varies in length, so the length
					// set in SetSubunitInfoResponseDataL may be wrong.
					aData.FillZ(KRemConStatusApiPageLength + KRemConStatusApiExtensionLength + KRemConStatusApiDefaultPageDataLength);
				
					// Stick some default data in there 
					SetCommandDataFromInt(aData, KRemConStatusApiPageOffset, 
						KRemConStatusApiPageLength, 0);	
						
					SetCommandDataFromInt(aData, KRemConStatusApiExtensionOffset, 
						KRemConStatusApiExtensionLength, AVC::EIgnore);	

					SetCommandDataFromInt(aData, KRemConStatusApiPageDataOffset, 
						KRemConStatusApiDefaultPageDataLength, KRemConStatusApiDefaultPageData);	

					err = KErrNone;
					}
				}
			break;
			}
		default:
			{
			err = KErrNotSupported;
			break;
			}
		}

	return err;
	}
	
/** Parses command data from the buffer.

@param aCommandData The buffer from which to read the data.
@param aVendorId On return, the vendor id
@param aUnit On return, the unit
@param aUnitType On return, the unit type 
@param aExtendedUnitType On return, the extended unit type

@internalComponent
@released
*/
void CRemConAvrcpStatusConverter::SetUnitInfoResponseDataL(const TDesC8& aBearerData,
	TDes8& aRemConData)
	{
	// AVRCP should pass us a sensible sized buffer!
	__ASSERT_ALWAYS(aRemConData.MaxLength() >= KUnitInfoResponseLength, CRemConAvrcpStatusConverter::Panic(EAvrcpPassedTooSmallABuffer));

	// Get rid of any junk
	aRemConData.FillZ(KRemConStatusApiUnitLength + KRemConStatusApiUnitTypeLength
					  + KRemConStatusApiExtendedUnitTypeLength
					  + KRemConStatusApiVendorIdLength);

	// Get unit id if there's enough data for it
	if(aBearerData.Length() < KUnitIdOffset + KUnitIdLength)
		{
		User::Leave(KErrCorrupt);
		}
	TInt unit;
	ReadCommandDataToInt(aBearerData, KUnitIdOffset, KUnitIdLength, unit);
	unit &= KAVCSubunitIDMask;
	SetCommandDataFromInt(aRemConData, KRemConStatusApiUnitOffset, 
		KRemConStatusApiUnitLength, unit);
	
	// Get unit type (maybe extended) if there's enough data
	if(aBearerData.Length() < KUnitTypeOffset + KUnitTypeLength)
		{
		User::Leave(KErrCorrupt);
		}
	TInt unitType;
	ReadCommandDataToInt(aBearerData, KUnitTypeOffset, KUnitTypeLength, unitType);
	unitType &= KAVCSubunitTypeMask;
	unitType >>= KUnitTypeShift;
	SetCommandDataFromInt(aRemConData, KRemConStatusApiUnitTypeOffset, 
		KRemConStatusApiUnitTypeLength, unitType);
	
	TInt unitTypeExtend = 0;
	TInt vendorIdOffset = KVendorIdBaseOffset;
	if(unitType == KUnitTypeExtend)
		{
		// Extended unit type.  VendorId is offset by 1
		vendorIdOffset++;
		
		// Read the next byte if it's there
		if(aBearerData.Length() < KUnitTypeExtendOffset + 1)
			{
			User::Leave(KErrCorrupt);
			}
		ReadCommandDataToInt(aBearerData, KUnitTypeExtendOffset, 1, unitTypeExtend);
		unitTypeExtend <<= 8;
		
		// 	Double extended unit type. 
		if(unitTypeExtend == KUnitTypeExtendExtend)
			{
			// VendorId is offset by 1
			vendorIdOffset++;
			
			// Read the next byte
			TInt unitTypeExtendExtend = 0;
			if(aBearerData.Length() < KUnitTypeExtendExtendOffset + 1)
				{
				User::Leave(KErrCorrupt);
				}
			ReadCommandDataToInt(aBearerData, KUnitTypeExtendExtendOffset, 1, 
								 unitTypeExtendExtend);	
			unitTypeExtend |= unitTypeExtendExtend;
			}

		SetCommandDataFromInt(aRemConData, KRemConStatusApiExtendedUnitTypeOffset, 
			KRemConStatusApiExtendedUnitTypeLength, unitTypeExtend);
		}		

	// Get vendor id
	if(aBearerData.Length() < vendorIdOffset + KVendorIdLength)
		{
		User::Leave(KErrCorrupt);
		}
	TInt vendorId;
	ReadCommandDataToInt(aBearerData, vendorIdOffset, KVendorIdLength, vendorId);				
	SetCommandDataFromInt(aRemConData, KRemConStatusApiVendorIdOffset, 
		KRemConStatusApiVendorIdLength, vendorId);
	}

/** Parses command data from the buffer.

@param aCommandData The buffer from which to read the data.
@param aPage On return, the page
@param aExtension On return, the extension
@param aPageData On return, the page data

@internalComponent
@released
*/
void CRemConAvrcpStatusConverter::SetSubunitInfoResponseDataL(const TDesC8& aBearerData,
	TDes8& aRemConData)
	{
	// AVRCP should pass us a sensible sized buffer!
	__ASSERT_ALWAYS(aRemConData.MaxLength() >= KUnitInfoResponseLength, CRemConAvrcpStatusConverter::Panic(EAvrcpPassedTooSmallABuffer));

	CAVCFrame* frame = CAVCFrame::NewL(aBearerData, AVC::EResponse); //Qualified
	CleanupStack::PushL(frame);		
	TPtrC8 ptr = frame->Data();
	
	// Work out how long a buffer we need for the RemCon data.
	// This is KRemConStatusApiPageLength + KRemConStatusApiExtensionLength
	// + the length of the page data.
	// We can work out the length of the page data from the frame - 
	// it's the remainder of the frame after KSubunitPageDataOffset.
	// This gives us:
	TInt remConDataLen = KRemConStatusApiPageLength 
						+ KRemConStatusApiPageLength 
						+ (ptr.Length() - KSubunitPageDataOffset);

	if(aRemConData.MaxLength() < remConDataLen)
		{
		User::Leave(KErrCorrupt);
		}

	// Get rid of any junk
	aRemConData.FillZ(remConDataLen);
	
	// Get page
	if(aBearerData.Length() < KSubunitPageOffset + KSubunitPageLength)
		{
		User::Leave(KErrCorrupt);
		}
	TInt page;
	ReadCommandDataToInt(aBearerData, KSubunitPageOffset, KSubunitPageLength, page);
	page &= KSubunitPageMask;
	SetCommandDataFromInt(aRemConData, KRemConStatusApiPageOffset, 
		KRemConStatusApiPageLength, page);	
		
	// Get extension code
	if(aBearerData.Length() < KSubunitExtensionOffset + KSubunitExtensionLength)
		{
		User::Leave(KErrCorrupt);
		}
	TInt extensionCode;
	ReadCommandDataToInt(aBearerData, KSubunitExtensionOffset, KSubunitExtensionLength, 
		extensionCode);
	extensionCode &= KSubunitExtensionMask;
	SetCommandDataFromInt(aRemConData, KRemConStatusApiExtensionOffset, 
		KRemConStatusApiExtensionLength, extensionCode);		
		
	// Get page data
	TPtrC8 pageDataPtr = aBearerData.Mid(KSubunitPageDataOffset);
	aRemConData.Replace(KRemConStatusApiPageDataOffset, pageDataPtr.Length(), pageDataPtr);
	
	CleanupStack::PopAndDestroy(frame);	
	}
	
/** Reads command data from the buffer to an int.

@param aCommandData The buffer from which to read the data.
@param aOffset The offset within aCommandData read from.
@param aLength The length of data to read.  This must not be
				more than 4.
@param aValue On return, the value of the specified data section.

@internalComponent
@released
*/	
void CRemConAvrcpStatusConverter::ReadCommandDataToInt(const TDesC8& aCommandData, 
	TInt aOffset, TInt aLength, TInt& aValue)
	{
	__ASSERT_DEBUG(aLength <= 4, CRemConAvrcpStatusConverter::Panic(EStatusConverterDataTooShort));

	aValue = 0;
	
	for(TInt i = 0 ; i < aLength; i++)
		{
		aValue |= aCommandData[aOffset+i]<<(8*i);
		}
	}
	
/** Set the command data.  This overwrites the current
contents of the data buffer.

@param aCommandData The buffer in which to set the data.
@param aOffset The offset within aCommandData to set the data.
@param aLength The length of data to replace.
@param aValue The new value for the replaced data.
*/
void CRemConAvrcpStatusConverter::SetCommandDataFromInt(TDes8& aCommandData, 
	TInt aOffset, TInt aLength, TInt aValue)
	{
	__ASSERT_DEBUG(aLength <= 4, CRemConAvrcpStatusConverter::Panic(EStatusConverterDataTooShort));
	
	for(TInt i = 0; i < aLength; i++)
		{
		aCommandData[aOffset+i] = aValue >> (8*i);
		}
	}

/** Utility Status Converter panic function.

@param aPanic The panic number.
*/
void CRemConAvrcpStatusConverter::Panic(TStatusConverterPanic aPanic)
	{
	User::Panic(KStatusConverterPanicName, aPanic);
	}

//
// End of file
