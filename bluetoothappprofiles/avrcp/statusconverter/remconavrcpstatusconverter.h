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
*/

#ifndef REMCONAVRCPSTATUSCONVERTER_H
#define REMCONAVRCPSTATUSCONVERTER_H

#include <e32base.h>
#include <remcon/remconconverterinterface.h>
#include <remcon/remconconverterplugin.h>

// These values are in bytes
const TUint KUnitIdOffset = 4;
const TUint KUnitIdLength = 1;
const TUint KUnitTypeOffset = 4;
const TUint KUnitTypeLength =1;
const TUint KUnitTypeExtendOffset = 5;
const TUint KUnitTypeExtendExtendOffset = 6;
const TUint KVendorIdBaseOffset = 5;
const TUint KVendorIdLength = 3;
const TUint KUnitInfoResponseLength = 8;
const TUint KRemConStatusApiDefaultPageDataLength = 4;

const TUint KSubunitPageOffset = 3;
const TUint KSubunitPageLength = 1;
const TUint KSubunitExtensionOffset = 3;
const TUint KSubunitExtensionLength = 1;
const TUint KSubunitPageDataOffset = 4;

// These values are in bits
const TUint KUnitTypeShift = 3;

// Defined values for comparison
const TUint KUnitTypeExtend = 0x1e;
const TUint KUnitTypeExtendExtend = 0xff;
const TUint KSubunitPageMask = 0xf8;
const TUint KSubunitExtensionMask = 0x7;

const TUint KRemConStatusApiDefaultVendorId = 0xffffff;
const TUint KRemConStatusApiDefaultPageData = 0xffffff48;

_LIT(KStatusConverterPanicName, "Stat Conv Panic");

enum TStatusConverterPanic
	{
	EStatusConverterWrongInterface = 0,
	EStatusConverterDataTooShort = 1,
	EAvrcpPassedTooSmallABuffer = 2,
	};

class CAVCFrame;
NONSHARABLE_CLASS(CRemConAvrcpStatusConverter) : public CRemConConverterPlugin, public MRemConConverterInterface
	{
public:
	static CRemConAvrcpStatusConverter* NewL();
	~CRemConAvrcpStatusConverter();

private: // from CRemConConverterPlugin
	TAny* GetInterface(TUid aUid);

	// from MRemConConverterInterface
	TInt InterfaceToBearer(TUid aInterfaceUid, 
		TUint aOperationId,
		const TDesC8& aData,
		TRemConMessageType aMsgType, 
		TDes8& aBearerData) const;
	
	TInt BearerToInterface(const TDesC8& aBearerData,
		TUid& aInterfaceUid, 
		TUint& aOperationId,
		TRemConMessageType& aMsgType, 
		TDes8& aData) const;

	TBool SupportedUids(const TUid& aInterfaceUid, 
		const TUid& aBearerUid) const;
	TBool SupportedInterface(const TDesC8& aInterfaceData, 
		const TUid& aBearerUid) const;

private:
	CRemConAvrcpStatusConverter();
	
	static CAVCFrame* UnitInfoL();
	static CAVCFrame* SubunitInfoL();
	
	static void SetUnitInfoResponseDataL(const TDesC8& aBearerData,
		TDes8& aRemConData);
	static void SetSubunitInfoResponseDataL(const TDesC8& aBearerData, 
		TDes8& aRemConData);
	
	static void ReadCommandDataToInt(const TDesC8& aCommandData, 
		TInt aOffset, TInt aLength, TInt& aValue);
	static void SetCommandDataFromInt(TDes8& aCommandData, 
		TInt aOffset, TInt aLength, TInt aValue);
	
	static void Panic(TStatusConverterPanic aPanic);
	};

#endif // REMCONAVRCPSTATUSCONVERTER_H
