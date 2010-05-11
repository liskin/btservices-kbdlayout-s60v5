// Copyright (c) 2004-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef AVCFRAME_H
#define AVCFRAME_H

/**
@file
@released
@publishedPartner
*/

#include <avcpanel.h>
#include <e32base.h>

static const TInt	KAVCFrameMaxLength			= 0x200;
static const TInt	KAVCFrameHeaderLength		= 0x3;
static const TInt	KAVCPassthroughFrameLength	= 0x5;
static const TUint8	KAVCCommandTypeMask			= 0x0f;
static const TUint8	KAVCResponseTypeMask		= 0x0f;
static const TUint8	KAVCSubunitTypeMask			= 0xf8;
static const TUint8	KAVCSubunitIDMask			= 0x07;
static const TUint8	KAVCCommandMaxRangeLength	= 0x7;  

static const TUint KAVCVendorUniquePassthroughHeader = 7;
static const TUint KAVCVendorIdBaseOffset = 5;
static const TUint KAVCVendorIdLength = 3;

static const TInt KAVCMaxVendorDependentPayload = KAVCFrameMaxLength-KAVCFrameHeaderLength-KAVCVendorIdLength;

namespace AVC
	{
	enum TFrameType
		{
		ECommand,
		EResponse
		};
	/**
	As per section 7.3.1, AV/C Digital Interface Command Set General Specification v4.0
	As per section 7.3.2, AV/C Digital Interface Command Set General Specification v4.0
	*/
	enum TCType
		{
		EControl			= 0x0,
		EStatus				= 0x1,
		ESpecificEnquiry	= 0x2,
		ENotify				= 0x3,
		EGeneralEnquiry		= 0x4,
		EReserved1			= 0x5,
		EReserved2			= 0x6,
		EReserved3			= 0x7,
		ENotImplemented		= 0x8,
		EAccepted			= 0x9,
		ERejected			= 0xa,
		EInTransition		= 0xb,
		EImplemented		= 0xc,
		EStable				= 0xc,	// NB: EImplemented and EStable are the same!
		EChanged			= 0xd,
		EReserved4			= 0xe,
		EInterim			= 0xf
		};
	/**
	As per sections 7.3.4.1 and 7.3.4.3, AV/C Digital Interface Command Set General Specification v4.0

	Extended subunit types should be coded as 0xXYY in this enumeration where
	X is the nth extension and YY is the type number.  e.g. 0x101 would correspond
	to example 3 in table 7.9 and 0x201 would correspond to example 4.
	*/		
	enum TSubunitType
		{
		EMonitor			= 0x00,
		EAudio				= 0x01,
		EPrinter			= 0x02,
		EDisc				= 0x03,
		ETape				= 0x04,
		ETuner				= 0x05,
		ECA					= 0x06,
		ECamera				= 0x07,
		EPanel				= 0x09,
		EBulletinBoard		= 0x0a,
		ECameraStorage		= 0x0b,
		EVendorUnique		= 0x1c,
		ETypeExtended1		= 0x1e,
		EUnit				= 0x1f,
		ETypeExtended2		= 0xff
		};
	/**
	As per sections 7.3.4.1 and 7.3.4.3, AV/C Digital Interface Command Set General Specification v4.0

	Extended subunit IDs should be coded as 0xXYY in this enumeration where
	X is the nth extension and YY is the ID number.  e.g. 0x102 would correspond
	to example 2 in table 7.10 and 0x201 would correspond to example 3.
	*/	
	enum TSubunitID
		{
		EID0				= 0x00,
		EID1				= 0x01,
		EID2				= 0x02,
		EID3				= 0x04,
		EIDExtended1		= 0x05,
		EIgnore				= 0x07,
		EIDExtended2		= 0xff
		};
	/**
	As per section 10.1.2, AV/C Digital Interface Command Set General Specification v4.0

	AV/C unit source plug addresses
	*/	
	enum TUnitInputPlug
		{
		ESerialIsochronousInput0	= 0x00,
		ESerialIsochronousInput1	= 0x01,
		ESerialIsochronousInput2	= 0x02,
		ESerialIsochronousInput3	= 0x03,
		ESerialIsochronousInput4	= 0x04,
		ESerialIsochronousInput5	= 0x05,
		ESerialIsochronousInput6	= 0x06,
		ESerialIsochronousInput7	= 0x07,
		ESerialIsochronousInput8	= 0x08,
		ESerialIsochronousInput9	= 0x09,
		ESerialIsochronousInput10	= 0x0a,
		ESerialIsochronousInput11	= 0x0b,
		ESerialIsochronousInput12	= 0x0c,
		ESerialIsochronousInput13	= 0x0d,
		ESerialIsochronousInput14	= 0x0e,
		ESerialIsochronousInput15	= 0x0f,
		ESerialIsochronousInput16	= 0x10,
		ESerialIsochronousInput17	= 0x11,
		ESerialIsochronousInput18	= 0x12,
		ESerialIsochronousInput19	= 0x13,
		ESerialIsochronousInput20	= 0x14,
		ESerialIsochronousInput21	= 0x15,
		ESerialIsochronousInput22	= 0x16,
		ESerialIsochronousInput23	= 0x17,
		ESerialIsochronousInput24	= 0x18,
		ESerialIsochronousInput25	= 0x19,
		ESerialIsochronousInput26	= 0x1a,
		ESerialIsochronousInput27	= 0x1b,
		ESerialIsochronousInput28	= 0x1c,
		ESerialIsochronousInput29	= 0x1d,
		ESerialIsochronousInput30	= 0x1e,
		EAnySerialIsochronousInput	= 0x7f,
		EExternalInput0				= 0x80,
		EExternalInput1				= 0x81,
		EExternalInput2				= 0x82,
		EExternalInput3				= 0x83,
		EExternalInput4				= 0x84,
		EExternalInput5				= 0x85,
		EExternalInput6				= 0x86,
		EExternalInput7				= 0x87,
		EExternalInput8				= 0x88,
		EExternalInput9				= 0x89,
		EExternalInput10			= 0x8a,
		EExternalInput11			= 0x8b,
		EExternalInput12			= 0x8c,
		EExternalInput13			= 0x8d,
		EExternalInput14			= 0x8e,
		EExternalInput15			= 0x8f,
		EExternalInput16			= 0x90,
		EExternalInput17			= 0x91,
		EExternalInput18			= 0x92,
		EExternalInput19			= 0x93,
		EExternalInput20			= 0x94,
		EExternalInput21			= 0x95,
		EExternalInput22			= 0x96,
		EExternalInput23			= 0x97,
		EExternalInput24			= 0x98,
		EExternalInput25			= 0x99,
		EExternalInput26			= 0x9a,
		EExternalInput27			= 0x9b,
		EExternalInput28			= 0x9c,
		EExternalInput29			= 0x9d,
		EExternalInput30			= 0x9e,
		ESerialAsynchronousInput0	= 0xa0,
		ESerialAsynchronousInput1	= 0xa1,
		ESerialAsynchronousInput2	= 0xa2,
		ESerialAsynchronousInput3	= 0xa3,
		ESerialAsynchronousInput4	= 0xa4,
		ESerialAsynchronousInput5	= 0xa5,
		ESerialAsynchronousInput6	= 0xa6,
		ESerialAsynchronousInput7	= 0xa7,
		ESerialAsynchronousInput8	= 0xa8,
		ESerialAsynchronousInput9	= 0xa9,
		ESerialAsynchronousInput10	= 0xaa,
		ESerialAsynchronousInput11	= 0xab,
		ESerialAsynchronousInput12	= 0xac,
		ESerialAsynchronousInput13	= 0xad,
		ESerialAsynchronousInput14	= 0xae,
		ESerialAsynchronousInput15	= 0xaf,
		ESerialAsynchronousInput16	= 0xb0,
		ESerialAsynchronousInput17	= 0xb1,
		ESerialAsynchronousInput18	= 0xb2,
		ESerialAsynchronousInput19	= 0xb3,
		ESerialAsynchronousInput20	= 0xb4,
		ESerialAsynchronousInput21	= 0xb5,
		ESerialAsynchronousInput22	= 0xb6,
		ESerialAsynchronousInput23	= 0xb7,
		ESerialAsynchronousInput24	= 0xb8,
		ESerialAsynchronousInput25	= 0xb9,
		ESerialAsynchronousInput26	= 0xba,
		ESerialAsynchronousInput27	= 0xbb,
		ESerialAsynchronousInput28	= 0xbc,
		ESerialAsynchronousInput29	= 0xbd,
		ESerialAsynchronousInput30	= 0xbe,
		EAnySerialAsynchronousInput	= 0xbf,
		EInvalidInput				= 0xfe,
		EAnyExternalInput			= 0xff
		};
	/**
	As per section 10.1.2, AV/C Digital Interface Command Set General Specification v4.0

	AV/C unit destination plug addresses	
	*/	
	enum TUnitOutputPlug
		{
		ESerialIsochronousOutput0	= 0x00,
		ESerialIsochronousOutput1	= 0x01,
		ESerialIsochronousOutput2	= 0x02,
		ESerialIsochronousOutput3	= 0x03,
		ESerialIsochronousOutput4	= 0x04,
		ESerialIsochronousOutput5	= 0x05,
		ESerialIsochronousOutput6	= 0x06,
		ESerialIsochronousOutput7	= 0x07,
		ESerialIsochronousOutput8	= 0x08,
		ESerialIsochronousOutput9	= 0x09,
		ESerialIsochronousOutput10	= 0x0a,
		ESerialIsochronousOutput11	= 0x0b,
		ESerialIsochronousOutput12	= 0x0c,
		ESerialIsochronousOutput13	= 0x0d,
		ESerialIsochronousOutput14	= 0x0e,
		ESerialIsochronousOutput15	= 0x0f,
		ESerialIsochronousOutput16	= 0x10,
		ESerialIsochronousOutput17	= 0x11,
		ESerialIsochronousOutput18	= 0x12,
		ESerialIsochronousOutput19	= 0x13,
		ESerialIsochronousOutput20	= 0x14,
		ESerialIsochronousOutput21	= 0x15,
		ESerialIsochronousOutput22	= 0x16,
		ESerialIsochronousOutput23	= 0x17,
		ESerialIsochronousOutput24	= 0x18,
		ESerialIsochronousOutput25	= 0x19,
		ESerialIsochronousOutput26	= 0x1a,
		ESerialIsochronousOutput27	= 0x1b,
		ESerialIsochronousOutput28	= 0x1c,
		ESerialIsochronousOutput29	= 0x1d,
		ESerialIsochronousOutput30	= 0x1e,
		EAnySerialIsochronousOutput	= 0x7f,
		EExternalOutput0			= 0x80,
		EExternalOutput1			= 0x81,
		EExternalOutput2			= 0x82,
		EExternalOutput3			= 0x83,
		EExternalOutput4			= 0x84,
		EExternalOutput5			= 0x85,
		EExternalOutput6			= 0x86,
		EExternalOutput7			= 0x87,
		EExternalOutput8			= 0x88,
		EExternalOutput9			= 0x89,
		EExternalOutput10			= 0x8a,
		EExternalOutput11			= 0x8b,
		EExternalOutput12			= 0x8c,
		EExternalOutput13			= 0x8d,
		EExternalOutput14			= 0x8e,
		EExternalOutput15			= 0x8f,
		EExternalOutput16			= 0x90,
		EExternalOutput17			= 0x91,
		EExternalOutput18			= 0x92,
		EExternalOutput19			= 0x93,
		EExternalOutput20			= 0x94,
		EExternalOutput21			= 0x95,
		EExternalOutput22			= 0x96,
		EExternalOutput23			= 0x97,
		EExternalOutput24			= 0x98,
		EExternalOutput25			= 0x99,
		EExternalOutput26			= 0x9a,
		EExternalOutput27			= 0x9b,
		EExternalOutput28			= 0x9c,
		EExternalOutput29			= 0x9d,
		EExternalOutput30			= 0x9e,
		ESerialAsynchronousOutput0	= 0xa0,
		ESerialAsynchronousOutput1	= 0xa1,
		ESerialAsynchronousOutput2	= 0xa2,
		ESerialAsynchronousOutput3	= 0xa3,
		ESerialAsynchronousOutput4	= 0xa4,
		ESerialAsynchronousOutput5	= 0xa5,
		ESerialAsynchronousOutput6	= 0xa6,
		ESerialAsynchronousOutput7	= 0xa7,
		ESerialAsynchronousOutput8	= 0xa8,
		ESerialAsynchronousOutput9	= 0xa9,
		ESerialAsynchronousOutput10	= 0xaa,
		ESerialAsynchronousOutput11	= 0xab,
		ESerialAsynchronousOutput12	= 0xac,
		ESerialAsynchronousOutput13	= 0xad,
		ESerialAsynchronousOutput14	= 0xae,
		ESerialAsynchronousOutput15	= 0xaf,
		ESerialAsynchronousOutput16	= 0xb0,
		ESerialAsynchronousOutput17	= 0xb1,
		ESerialAsynchronousOutput18	= 0xb2,
		ESerialAsynchronousOutput19	= 0xb3,
		ESerialAsynchronousOutput20	= 0xb4,
		ESerialAsynchronousOutput21	= 0xb5,
		ESerialAsynchronousOutput22	= 0xb6,
		ESerialAsynchronousOutput23	= 0xb7,
		ESerialAsynchronousOutput24	= 0xb8,
		ESerialAsynchronousOutput25	= 0xb9,
		ESerialAsynchronousOutput26	= 0xba,
		ESerialAsynchronousOutput27	= 0xbb,
		ESerialAsynchronousOutput28	= 0xbc,
		ESerialAsynchronousOutput29	= 0xbd,
		ESerialAsynchronousOutput30	= 0xbe,
		EAnySerialAsynchronousOutput= 0xbf,
		EMultipleOutputs			= 0xfd,
		EInvalidOutput				= 0xfe,
		EAnyExternalOutput			= 0xff
		};
	/**
	As per section 10.2.3, AV/C Digital Interface Command Set General Specification v4.0

	AV/C subunit source plug addresses 
	subunit does not have to implement any subunit plug if it does not send or receive any signals		
	*/
	enum TSubunitSourcePlug
		{
		ESource0			= 0x00,
		ESource1			= 0x01,
		ESource2			= 0x02,
		ESource3			= 0x03,
		ESource4			= 0x04,
		ESource5			= 0x05,
		ESource6			= 0x06,
		ESource7			= 0x07,
		ESource8			= 0x08,
		ESource9			= 0x09,
		ESource10			= 0x0a,
		ESource11			= 0x0b,
		ESource12			= 0x0c,
		ESource13			= 0x0d,
		ESource14			= 0x0e,
		ESource15			= 0x0f,
		ESource16			= 0x10,
		ESource17			= 0x11,
		ESource18			= 0x12,
		ESource19			= 0x13,
		ESource20			= 0x14,
		ESource21			= 0x15,
		ESource22			= 0x16,
		ESource23			= 0x17,
		ESource24			= 0x18,
		ESource25			= 0x19,
		ESource26			= 0x1a,
		ESource27			= 0x1b,
		ESource28			= 0x1c,
		ESource29			= 0x1d,
		ESource30			= 0x1e,
		EInvalidSource		= 0xfe,
		EAnySource			= 0xff
		};
	/**
	As per section 10.2.3, AV/C Digital Interface Command Set General Specification v4.0

	AV/C subunit destination plug		
	subunit does not have to implement any subunit plug if it does not send or receive any signals	
	*/
	enum TSubunitDestinationPlug
		{
		EDestination0			= 0x00,
		EDestination1			= 0x01,
		EDestination2			= 0x02,
		EDestination3			= 0x03,
		EDestination4			= 0x04,
		EDestination5			= 0x05,
		EDestination6			= 0x06,
		EDestination7			= 0x07,
		EDestination8			= 0x08,
		EDestination9			= 0x09,
		EDestination10			= 0x0a,
		EDestination11			= 0x0b,
		EDestination12			= 0x0c,
		EDestination13			= 0x0d,
		EDestination14			= 0x0e,
		EDestination15			= 0x0f,
		EDestination16			= 0x10,
		EDestination17			= 0x11,
		EDestination18			= 0x12,
		EDestination19			= 0x13,
		EDestination20			= 0x14,
		EDestination21			= 0x15,
		EDestination22			= 0x16,
		EDestination23			= 0x17,
		EDestination24			= 0x18,
		EDestination25			= 0x19,
		EDestination26			= 0x1a,
		EDestination27			= 0x1b,
		EDestination28			= 0x1c,
		EDestination29			= 0x1d,
		EDestination30			= 0x1e,
		EMultipleDestinations	= 0xfe,
		EInvalidDestination		= 0xfe,
		EAnyDestination			= 0xff
		};
	/**
	As per sections 11 and 12, AV/C Digital Interface Command Set General Specification v4.0
	Other commands are defined by the various subunit specifications		
	*/
	enum TOpcode
		{
		EPower						= 0xb2,
		EUnitInfo					= 0x30,
		ESubunitInfo				= 0x31,
		EReserve					= 0x01,
		EVersion					= 0xb0,
		EVendorDependent			= 0x00,
		EPlugInfo					= 0x02,
		EChannelUsage				= 0x12, 
		EConnect					= 0x24,
		EConnectAV					= 0x20,
		EConnections				= 0x22,
		EDigitalInput				= 0x11,
		EDigitalOutput				= 0x10,
		EDisconnect					= 0x25,
		EDisconnectAV				= 0x21,
		EInputPlugSignalFormat		= 0x19,
		EOutputPlugSignalFormat		= 0x18,
		EPassThrough				= 0x7c
		};
	
	enum TPacketType
		{
		EASingle      = 0x00,
		EStart        = 0x01,
		EContinue     = 0x10,
		EEnd          = 0x11,
		};
	
		typedef TUint TAVCVendorId;
	}

/**
AVC frame creation and utilities.
@released
@publishedPartner
*/
NONSHARABLE_CLASS(CAVCFrame) : public CBase
	{
public:
	
	// Construction / destruction
	// TBH this acts as a factory now and could (eventually!) return a derived class
	// the caller could check by calling Opcode
	// at the moment we are going to use the derived classes just for outbound responses
	IMPORT_C static CAVCFrame* NewL(AVC::TFrameType aFrameType, 
		AVC::TCType aType, 
		AVC::TSubunitType aSubunitType, 
		AVC::TSubunitID aSubunitID);
		
	IMPORT_C static CAVCFrame* NewL(const TDesC8& aBuffer, AVC::TFrameType aType);
	
	IMPORT_C virtual ~CAVCFrame();

	// Utility
	IMPORT_C AVC::TFrameType FrameType() const;
	IMPORT_C static AVC::TFrameType FrameType(const TDesC8& aFrame);
	IMPORT_C void SetFrameType(AVC::TFrameType aFrameType);
	
	IMPORT_C AVC::TCType Type() const;
	IMPORT_C void SetType(AVC::TCType aType);
	
	IMPORT_C AVC::TSubunitType SubunitType() const;
	IMPORT_C AVC::TSubunitID SubunitID() const;
	IMPORT_C TUint8 Opcode() const;
	IMPORT_C TInt OperationId(TUint8& aOpId) const;
	IMPORT_C TInt ButtonAct(AVCPanel::TButtonAction& aButtonAction) const;
	
	IMPORT_C const TUint8& operator[](TInt aIndex) const;
	IMPORT_C const TDesC8& Data() const;
	
	IMPORT_C void Append(const TDesC8& aDes);
	IMPORT_C void Append(TChar aChar);
	IMPORT_C TInt DataLength() const;
	
	IMPORT_C static AVC::TOpcode OpcodeL(const TDesC8& aBuffer);

private:
	CAVCFrame(AVC::TFrameType aFrameType);
	
	void ConstructL(AVC::TCType aType, 
		AVC::TSubunitType aSubunitType, 
		AVC::TSubunitID aSubunitID);
	void ConstructL(const TDesC8& aBuffer);
	static void FindExtensionL(const TDesC8& aBuffer, TInt& aSubunitTypeExtensionBytes, TInt& aSubunitIDExtensionBytes);

private:
	RBuf8			iBuffer;

	AVC::TFrameType	iFrameType;
	TInt			iSubunitTypeExtensionBytes;
	TInt			iSubunitIDExtensionBytes;
	};


// Factory pattern stuff - these are useful to do donkey work,
// but have existing clients treat them as the base class

class CAVCVendorDependentResponse			// codescanner::missingcclass
	{
public:
	IMPORT_C static CAVCFrame* NewL(TUint aVendorId);
	};

class CAVCPassthroughCommand				// codescanner::missingcclass
	{
public:
	};

// likely not have newl because dont allocate on parse pattern
class CAVCVendorDependentCommand			// codescanner::missingcclass
	{
public:
	IMPORT_C static TPtrC8 GetPayloadAndVID(const CAVCFrame& aFrame, TUint& aVID);
	};
	
// likely not have newl because dont allocate on parse pattern
class CAVCVendorUniquePassthroughCommand	// codescanner::missingcclass
	{
public:
	IMPORT_C static TPtrC8 GetPayloadAndVID(const CAVCFrame& aFrame, TUint& aVID);
	};

#endif // AVCFRAME_H
