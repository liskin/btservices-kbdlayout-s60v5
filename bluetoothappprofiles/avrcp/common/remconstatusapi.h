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

#ifndef REMCONSTATUSAPI_H
#define REMCONSTATUSAPI_H

#include <e32base.h>

const TUint KRemConStatusApiMaxOperationSpecificDataSize = 10;

//-------------------------------------------------------------------------- 
// Unit info response data format
//--------------------------------------------------------------------------

/** Length of the VendorId field in bytes. */
const TUint KRemConStatusApiVendorIdLength = 3;

/** Offset for VendorId field */
const TUint KRemConStatusApiVendorIdOffset = 0; 

/** Length of the Unit field in bytes. */
const TUint KRemConStatusApiUnitLength = 1;

/** Offset for Unit field */
const TUint KRemConStatusApiUnitOffset = KRemConStatusApiVendorIdLength;

/** Length of the Unit type field in bytes. */
const TUint KRemConStatusApiUnitTypeLength = 1;

/** Offset for Unit type field */
const TUint KRemConStatusApiUnitTypeOffset = KRemConStatusApiVendorIdLength + KRemConStatusApiUnitLength;

/** Length of the Extended Unit type field in bytes. */
const TUint KRemConStatusApiExtendedUnitTypeLength = 2;

/** Offset for Extended Unit type field.

This data is only valid if Unit type is 0x1e.
The second byte is only valid if the first byte is 0xff.
See AV/C general specification for details.
*/
const TUint KRemConStatusApiExtendedUnitTypeOffset = 
	KRemConStatusApiVendorIdLength + KRemConStatusApiUnitLength +KRemConStatusApiUnitTypeLength;

//-------------------------------------------------------------------------- 
// SubUnit info response data format
//--------------------------------------------------------------------------

/** Length of the page field in bytes. */
const TUint KRemConStatusApiPageLength = 1;

/** Offset for page field */
const TUint KRemConStatusApiPageOffset = 0; 

/** Length of the extension field in bytes. */
const TUint KRemConStatusApiExtensionLength = 1;

/** Offset for extension field */
const TUint KRemConStatusApiExtensionOffset = KRemConStatusApiPageLength; 

/** Length of the page data field in bytes. 

This is defined as being all data from KRemConStatusApiPageDataOffset
to the end of the response data.
*/

/** Offset for page data field */
const TUint KRemConStatusApiPageDataOffset = KRemConStatusApiPageLength + KRemConStatusApiExtensionLength; 

//-------------------------------------------------------------------------- 
// Interface Data
//--------------------------------------------------------------------------

/**
The UID identifying this outer-layer RemCon interface.
*/
const TInt KRemConStatusApiUid = 0x10207ea5;

/**
Operation ids belonging to the Status API.
*/
enum TRemConStatusApiOperationId
	{
	ERemConStatusApiUnitInfo					= 0x00,
	ERemConStatusApiSubunitInfo					= 0x01,
	};

#endif // REMCONSTATUSAPI_H
