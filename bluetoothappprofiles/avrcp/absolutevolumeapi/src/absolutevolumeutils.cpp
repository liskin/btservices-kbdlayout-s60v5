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

#include <absolutevolumeutils.h>

EXPORT_C
void RRemConAbsoluteVolume::Close()
	{
	iStream.Close();
	iReadStream.Close();
	}

EXPORT_C 
void RRemConAbsoluteVolumeRequest::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iVolume = iReadStream.ReadUint32L();
	iMaxVolume = iReadStream.ReadUint32L();
	iReadStream.ReadUint32L();
	iReadStream.Close();
	}

EXPORT_C
void RRemConAbsoluteVolumeRequest::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	iStream.WriteUint32L(iVolume);
	iStream.WriteUint32L(iMaxVolume);
	iStream.WriteUint32L(NULL);
	iStream.CommitL();
	}

EXPORT_C
void RRemConAbsoluteVolumeResponse::ReadL(const TDesC8& aData)
	{
	iReadStream.Open(aData);
	iError = iReadStream.ReadInt32L();
	if (iError == KErrNone)
	    {
	    iVolume = iReadStream.ReadUint32L();
	    iMaxVolume = iReadStream.ReadUint32L();
	    }
	iReadStream.Close();
	}

EXPORT_C
void RRemConAbsoluteVolumeResponse::WriteL(TDes8& aOutData)
	{
	aOutData.Zero();
	iStream.Open(aOutData);
	iStream.WriteInt32L(iError);
	if (KErrNone == iError)
		{
		iStream.WriteUint32L(iVolume);
		iStream.WriteUint32L(iMaxVolume);
		}
	iStream.CommitL();
	}
