/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*				 
*				 
*				 
*
*/


#ifndef BTSAC_STREAM_END_POINT_H
#define BTSAC_STREAM_END_POINT_H

#include <bluetoothav.h>		// Protocol definitions, e.g., TAvdtpMediaCodecCapabilities


NONSHARABLE_CLASS(CBTSACStreamEndPoint) : public CBase
	{
public:
enum TSEPState
	{
	EIdle,
	EDiscoveredRemote,
	ERegistered,
	ELocallyConfigured,
	ERemotelyBound,
	EOpen,
	EStreaming,
	ESuspended,
	EAborted,
	};	
public:
	static CBTSACStreamEndPoint* NewL(const TAvdtpSEPInfo& aInfo);
	void SetState(TSEPState aState, const TAny* aBlob=NULL);
	void GetInfo(TAvdtpSEPInfo&  aInfo);
	void StoreCaps(TAvdtpServiceCapability* aCaps);
	void GetCaps(RPointerArray<TAvdtpServiceCapability>& aCaps);
	virtual ~CBTSACStreamEndPoint();
	inline TSEID SEID() const 
		{
		return iInfo.SEID();
		}
private:
	CBTSACStreamEndPoint();
	CBTSACStreamEndPoint(const TAvdtpSEPInfo& aInfo);
private:
	const TAvdtpSEPInfo iInfo;
	TBool	iIsRemote;
	RPointerArray<TAvdtpServiceCapability> iSEPCapabilities;
	};


#endif

