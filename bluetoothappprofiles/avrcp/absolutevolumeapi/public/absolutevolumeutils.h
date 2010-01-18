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
 @publishedPartner
 @released
*/

#ifndef ABSOLUTEVOLUMEUTILS_H_
#define ABSOLUTEVOLUMEUTILS_H_

#include <e32base.h>
#include <s32mem.h>

class RRemConAbsoluteVolume
	{
public:
    virtual void ReadL(const TDesC8& aData) = 0;
    virtual void WriteL(TDes8& aOutData) = 0;
	IMPORT_C void Close();
	
public:
	TUint32  iVolume;
	TUint32  iMaxVolume;	
	
protected:
	RDesWriteStream    iStream;
	RDesReadStream     iReadStream;
	};

class RRemConAbsoluteVolumeRequest : public RRemConAbsoluteVolume
	{
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
	};

class RRemConAbsoluteVolumeResponse : public RRemConAbsoluteVolume
	{
public:
	IMPORT_C void ReadL(const TDesC8& aData);
	IMPORT_C void WriteL(TDes8& aOutData);
public:
	TInt32   iError;
	};

#endif /*ABSOLUTEVOLUMEUTILS_H_*/
