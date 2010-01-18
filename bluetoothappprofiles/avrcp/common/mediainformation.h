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

/**
 @file
 @internalTechnology
 @released
*/

#ifndef REMCONMEDIAINFORMATION_H
#define REMCONMEDIAINFORMATION_H

#include "avrcpipc.h"

//-------------------------------------------------------------------------- 
// Interface Data
//--------------------------------------------------------------------------

/**
 * The UID identifying the RemCon MediaInformation interface.
 */
const TInt KRemConMediaInformationApiUid = 0x10285B20;

// identifier (8bytes) NumAttributes (1) AttributeID (max is 4*7) = 37
const TInt KMaxLengthMediaInformationMsg = 37;
const TInt KMaxMediaAttributeValue = 7;

class RRemConGetElementAttributesRequest : public RAvrcpIPC
	{
public:
	TUint64 				iElement;
	TInt 					iNumberAttributes;
	RArray<TInt>			iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C void Close();
	};


class REAResponse
	{
public:
	IMPORT_C void Close();
public:
	TInt    iAttributeId;
	TUint16 iCharset;
	TUint16 iStringLen;
	HBufC8* iString;
	};


class RRemConGetElementAttributesResponse : public RAvrcpIPC
	{
public:
	TInt 					iNumberAttributes;
	RArray<REAResponse>		iAttributes;

public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	IMPORT_C TInt Size();
	IMPORT_C void Close();
	};

#endif // REMCONMEDIAINFORMATION_H
