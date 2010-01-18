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
 @publishedPartner
 @released
*/

#include <bluetooth/logger.h>
#include <avcframe.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_FRAME);
#endif

#ifdef _DEBUG
PANICCATEGORY("avctpframe");
#endif

/** Constructor.

@param aFrameType ECommand if this is a command, EResponse
				  if it's a response.
@internalComponent
@released	
*/
CAVCFrame::CAVCFrame(AVC::TFrameType aFrameType)
	: iFrameType(aFrameType)
	{
	LOG_FUNC
	}

/** Destructor.

@publishedPartner
@released
*/
EXPORT_C CAVCFrame::~CAVCFrame()
	{
	LOG_FUNC
	iBuffer.Close();
	}

/** Factory function.

This overload should be called when an AV/C frame is
to be constructed, that is it is probably an outgoing frame.

@param aFrameType ECommand if this is a command, EResponse
				  if it's a response.
@param aType The AV/C CType for this frame.
@param aSubunitType The AV/C subunit type for this frame.
@param aSubunitId The AV/C subunit id for this frame.
@return A fully constructed CAVCFrame.
@leave System wide error code.
@publishedPartner
@released
*/
EXPORT_C CAVCFrame* CAVCFrame::NewL(AVC::TFrameType aFrameType, 
	AVC::TCType aType, 
	AVC::TSubunitType aSubunitType, 
	AVC::TSubunitID aSubunitID)
	{
	CAVCFrame* frame = new(ELeave)CAVCFrame(aFrameType);
	CleanupStack::PushL(frame);
	frame->ConstructL(aType, aSubunitType, aSubunitID);
	CleanupStack::Pop(frame);
	return frame;
	}

/** Factory function.

This overload should be used when a data buffer should
be parsed as an AV/C frame, that is it is probably an 
incoming frame.

@param aBuffer A buffer to be parsed as an AV/C frame.
@param aFrameType ECommand if this is a command, EResponse
				  if it's a response.
@return A fully constructed CAVCFrame.
@leave System wide error code.				  
@publishedPartner
@released
*/
EXPORT_C CAVCFrame* CAVCFrame::NewL(const TDesC8& aBuffer, AVC::TFrameType aFrameType)
	{
	CAVCFrame* frame = new(ELeave)CAVCFrame(aFrameType);
	CleanupStack::PushL(frame);
	frame->ConstructL(aBuffer);
	CleanupStack::Pop(frame);
	return frame;
	}
	
/** Second phase construction.

This overload is used when an AV/C frame is
to be constructed, that is it is probably an outgoing frame.

@param aType The AV/C CType for this frame.
@param aSubunitType The AV/C subunit type for this frame.
@param aSubunitId The AV/C subunit id for this frame.
@return A fully constructed CAVCFrame.
@leave System wide error code.
@internalComponent
@released
*/
void CAVCFrame::ConstructL(AVC::TCType aType, AVC::TSubunitType aSubunitType, AVC::TSubunitID aSubunitID)
	{
	iBuffer.CreateL(KAVCFrameMaxLength);
	iBuffer.Zero();
	iBuffer.Append(TChar(aType));

	TInt subType	= aSubunitType;
	TInt subID		= aSubunitID;

	if (subType > AVC::ETypeExtended2)
		{
		iSubunitTypeExtensionBytes = 1;
		iBuffer.Append(TChar(AVC::ETypeExtended1 << 3));
		subType -= 0x100;

		while (subType > 0x100)
			{
			iBuffer.Append(TChar(AVC::ETypeExtended2));
			subType -= 0x100;
			}

		iBuffer.Append(TChar(subType));
		}
	else
		{
		iBuffer.Append(TChar(subType << 3));
		}

	if (subID > AVC::EIDExtended2)
		{
		iSubunitIDExtensionBytes = 1;
		iBuffer[1] |= AVC::EIDExtended1;
		subID -= 0x100;

		while (subID > 0x100)
			{
			iBuffer.Append(TChar(AVC::EIDExtended2));
			subID -= 0x100;
			}

		iBuffer.Append(TChar(subID));
		}
	else
		{
		iBuffer[1] |= subID;
		}
	}

/** Second phase construction.

This overload is used when a data buffer should
be parsed as an AV/C frame, that is it is probably an 
incoming frame.

For details of parsing refer to the AV/C digital
interface command set specification.

@param aBuffer A buffer to be parsed as an AV/C frame.
@return A fully constructed CAVCFrame.
@leave System wide error code.	
@publishedPartner
@released
*/	
void CAVCFrame::ConstructL(const TDesC8& aBuffer)
	{
	iBuffer.CreateL(aBuffer);
	FindExtensionL(iBuffer, iSubunitTypeExtensionBytes, iSubunitIDExtensionBytes);
	}

/** Gets the AV/C frame type.

@return ECommand if this is a command, EResponse if this
		is a response.
@publishedPartner
@released
*/
EXPORT_C AVC::TFrameType CAVCFrame::FrameType() const
	{
	return iFrameType;
	}

/** Gets the AV/C frame type.

@param aFrame The frame to get the frame type for.
@return ECommand if this is a command, EResponse if this
		is a response.
@publishedPartner
@released
*/	
EXPORT_C AVC::TFrameType CAVCFrame::FrameType(const TDesC8& aFrame)
	{
	AVC::TFrameType frameType = AVC::ECommand;
	
	if( aFrame[0] > KAVCCommandMaxRangeLength )  
		{
			frameType = AVC::EResponse;
		}
	return frameType;	
	}

/** Set the AV/C frame type for this frame.

@param The frame type to set.
@publishedPartner
@released
*/	
EXPORT_C void CAVCFrame::SetFrameType(AVC::TFrameType aFrameType)
	{
	iFrameType = aFrameType;
	}

/** Get the AV/C CType for this frame.

@return The AV/C CType for this frame.
@publishedPartner
@released
*/
EXPORT_C AVC::TCType CAVCFrame::Type() const
	{
	return static_cast<AVC::TCType>(iBuffer[0]);
	}

/** Set the AV/C CType for this frame.

@param aType The AV/C CType to set.
@publishedPartner
@released
*/	
EXPORT_C void CAVCFrame::SetType(AVC::TCType aType)
	{
	iBuffer[0] = aType;
	}

/** Get the AV/C subunit type for this frame.

@return The AV/C subunit type for this frame.
@publishedPartner
@released
*/
EXPORT_C AVC::TSubunitType CAVCFrame::SubunitType() const
	{
	if (iSubunitTypeExtensionBytes == 0)
		{
		return static_cast<AVC::TSubunitType>((iBuffer[1] & KAVCSubunitTypeMask) >> 3);
		}

	return static_cast<AVC::TSubunitType>(iBuffer[1 + iSubunitTypeExtensionBytes] + (iSubunitTypeExtensionBytes * 0x100));
	}

/** Get the AV/C subunit id for this frame.

@return The AV/C subunit id for this frame.
@publishedPartner
@released
*/
EXPORT_C AVC::TSubunitID CAVCFrame::SubunitID() const
	{
	if (iSubunitIDExtensionBytes == 0)
		{
		return static_cast<AVC::TSubunitID>(iBuffer[1] & KAVCSubunitIDMask);
		}

	return static_cast<AVC::TSubunitID>(iBuffer[1 + iSubunitTypeExtensionBytes + iSubunitIDExtensionBytes] + (iSubunitIDExtensionBytes * 0x100));
	}

/** Find extension bytes for the frame.

@param aBuffer buffer to be used.
@return True if its a valid frame.
@leave System wide error code.
@internalComponent
@released
*/
/* static */ void CAVCFrame::FindExtensionL(const TDesC8& aBuffer, TInt& aSubunitTypeExtensionBytes, TInt& aSubunitIDExtensionBytes)
	{
	TInt minLength = KAVCFrameHeaderLength;
	if(aBuffer.Length() < minLength)
		{
		User::Leave(KErrCorrupt);
		}
	
	if (static_cast<AVC::TSubunitType>((aBuffer[1] & KAVCSubunitTypeMask) >> 3) == AVC::ETypeExtended1)
		{
		aSubunitTypeExtensionBytes++;
		minLength++;

		while (aBuffer[1 + aSubunitTypeExtensionBytes] == AVC::ETypeExtended2)
			{
			if(aBuffer.Length() < minLength)
				{
				User::Leave(KErrCorrupt);
				}
			
			aSubunitTypeExtensionBytes++;
			minLength++;
			}
		}

	if (static_cast<AVC::TSubunitID>(aBuffer[1] & KAVCSubunitIDMask) == AVC::EIDExtended1)
		{
		aSubunitIDExtensionBytes++;
		minLength++;

		while (aBuffer[1 + aSubunitIDExtensionBytes] == AVC::EIDExtended1)
			{
			if(aBuffer.Length() < minLength)
				{
				User::Leave(KErrCorrupt);
				}
			
			aSubunitIDExtensionBytes++;
			minLength++;
			}
		}
		
	//Ensure frame is a valid length i.e. the Opcode() method can be safely called.
	if(aBuffer.Length() < minLength)
		{
		User::Leave(KErrCorrupt);
		}	
	}

/** Get the AV/C opcode for this frame.

@param aBuffer buffer to search.
@return The AV/C opcode for this frame.
@leave System wide error code.
@internalComponent
@released
*/
/* static */ EXPORT_C AVC::TOpcode CAVCFrame::OpcodeL(const TDesC8& aBuffer)
	{
	TInt subunitTypeExtensionBytes=0;
	TInt subunitIDExtensionBytes=0;
	
	FindExtensionL(aBuffer, subunitTypeExtensionBytes, subunitIDExtensionBytes);
	return static_cast<AVC::TOpcode> (aBuffer[KAVCFrameHeaderLength + subunitTypeExtensionBytes + subunitIDExtensionBytes - 1]);
	}

/** Get the AV/C opcode for this frame.

@return The AV/C opcode for this frame.
@publishedPartner
@released
*/
EXPORT_C TUint8 CAVCFrame::Opcode() const
	{
	return iBuffer[KAVCFrameHeaderLength + iSubunitTypeExtensionBytes + iSubunitIDExtensionBytes - 1];
	}

/** Get the AV/C OperationId for this frame.

This is only valid for passthrough commands.

@param aOpId On return, the AV/C opcode for this frame.
@return KErrNotSupported if this is not a passthrough command,
		KErrCorrupt if this passthrough command does not contain a OpId,
		KErrNone otherwise.
@publishedPartner
@released
*/	
EXPORT_C TInt CAVCFrame::OperationId(TUint8& aOpId) const
	{
	TInt err = KErrNotSupported;
	
	if(Opcode() == AVC::EPassThrough)
		{
		if(DataLength())
			{
			aOpId = (iBuffer[iSubunitTypeExtensionBytes + iSubunitIDExtensionBytes + KAVCFrameHeaderLength]) & 0x7f;
			err = KErrNone;			
			}
		else
			{
			err = KErrCorrupt;
			}
		}
		
	return err;
	}

/** Get the AV/C button action for this frame.

This is only valid for passthrough commands.

@param aOpId On return, the AV/C button action for this frame.
@return KErrNotSupported if this is not a passthrough command,
		KErrCorrupt if this passthrough command does not contain a button action,
		KErrNone otherwise.
@publishedPartner
@released
*/	
EXPORT_C TInt CAVCFrame::ButtonAct(AVCPanel::TButtonAction& aButtonAction) const
	{
	TInt err = KErrNotSupported;
	
	if(Opcode() == AVC::EPassThrough)
		{
		if(DataLength())
			{
			aButtonAction = (((iBuffer[iSubunitTypeExtensionBytes + iSubunitIDExtensionBytes + KAVCFrameHeaderLength]) & 0x80) == AVCPanel::EButtonRelease) ? AVCPanel::EButtonRelease : AVCPanel::EButtonPress;
			err = KErrNone;	
			}
		else
			{
			err = KErrCorrupt;
			}
		}
	return err;
	}

/** Retrieve data from the AV/C frame.

@param aIndex The offset of the data element within the data segment of the frame
@return The data element at aIndex.
@panic If aIndex is outside the frame. DataLength() should be used to check the length of the data segment before using the [] operator.
@publishedPartner
@released
*/
EXPORT_C const TUint8& CAVCFrame::operator[](TInt aIndex) const
	{
	return iBuffer[aIndex + iSubunitTypeExtensionBytes + iSubunitIDExtensionBytes + KAVCFrameHeaderLength];
	}

/** Retrieve the entire AV/C frame.

@return The AV/C frame.
@publishedPartner
@released
*/
EXPORT_C const TDesC8& CAVCFrame::Data() const
	{
	return iBuffer;
	}

/** Append data to the AV/C frame.

@param aDes The data to be appended.
@publishedPartner
@released
*/
EXPORT_C void CAVCFrame::Append(const TDesC8& aDes)
	{
	iBuffer.Append(aDes);
	}

/** Append data to the AV/C frame.

@param aChar The data to be appended.
@publishedPartner
@released
*/
EXPORT_C void CAVCFrame::Append(TChar aChar)
	{
	iBuffer.Append(aChar);
	}

/** Return the length of the data in the AV/C frame

@return The length of the data in the AV/C frame
@publishedPartner
@released
*/
EXPORT_C TInt CAVCFrame::DataLength() const
	{
	return (iBuffer.Length() - iSubunitTypeExtensionBytes - iSubunitIDExtensionBytes - KAVCFrameHeaderLength);
	}

EXPORT_C CAVCFrame* CAVCVendorDependentResponse::NewL(TUint aVendorID)
	{
	using namespace AVC;
	CAVCFrame* frame = CAVCFrame::NewL(EResponse,
										ENotImplemented, //client can override
										EPanel,
										EID0);
	// stupid frames don't know about themselves so we construct in derived classes
	// first opcode  - base class REALLY ought to have opcode setter
	frame->Append(0); //opcode for VD frame
	// second vendor
	frame->Append(aVendorID>>16);	
	frame->Append(aVendorID>>8);	
	frame->Append(aVendorID);	
	return frame;
	}

EXPORT_C TPtrC8 CAVCVendorDependentCommand::GetPayloadAndVID(const CAVCFrame& aFrame, TUint& aVID)
	{
	ASSERT_DEBUG(aFrame.Opcode()==AVC::EVendorDependent); //opcode
	aVID = (aFrame.operator[](0)<<16) | 
			(aFrame.operator[](1)<<8) |
			(aFrame.operator[](2));

	return (aFrame.Data().Right(aFrame.DataLength()-KAVCVendorIdLength));
	}

EXPORT_C TPtrC8 CAVCVendorUniquePassthroughCommand::GetPayloadAndVID(const CAVCFrame& aFrame, TUint& aVID)
	{
	ASSERT_DEBUG(aFrame.Opcode()==AVC::EPassThrough); //opcode
	aVID = (aFrame.operator[](2)<<16) | 
			(aFrame.operator[](3)<<8) |
			(aFrame.operator[](4));

	return (aFrame.Data().Right(aFrame.DataLength()-KAVCVendorIdLength-2));
	}
