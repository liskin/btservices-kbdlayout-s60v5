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
* Description:  Contains definition of CBTSACGavdp class which uses GAVDP API. 
*				 Two other classes CAVStreamEndPoint and CBTSACSEPManager are helper 
*				 classes to use GAVDP. This class also constructs CBTSACStreamerController
*				 class.
*
*/


#ifndef BTSAC_SEP_MANAGER_H
#define BTSAC_SEP_MANAGER_H

#include "btsacStreamEndPoint.h"


// DATA TYPES
_LIT(KSEPIdle, "Idle");
_LIT(KSEPDiscoveredRemote, "Remote");
_LIT(KSEPRegistered, "Registered");
_LIT(KSEPLocallyConfigured, "Configured");
_LIT(KSEPRemotelyBound, "Remotely bound");
_LIT(KSEPOpen, "Open");
_LIT(KSEPStreaming, "Streaming");
_LIT(KSEPSuspended, "Suspended");
_LIT(KSEPAborted, "Aborted");
_LIT(KSEPErrored, "Errored");


// FORWARD DECLARATION


NONSHARABLE_CLASS(CBTSACSEPManager) : public CBase
	{
public:
	static CBTSACSEPManager* NewL(const TBTDevAddr& aAddr);
	
	TInt SetState(TSEID aSEID, CBTSACStreamEndPoint::TSEPState aState, const TAny* aBlob=NULL);
	void BindSEPs(TSEID aLocalSEID, TSEID aRemoteSEID);
	void NewSEPL(const TAvdtpSEPInfo& aInfo);
	TInt GetSEP(TSEID aSEID, CBTSACStreamEndPoint& aSEP);
	TInt NoOfSEPs();
	TInt GetInfo(TInt aIndex, TAvdtpSEPInfo& aInfo);
	TInt StoreCaps( TInt aIndex, TAvdtpServiceCapability* aCaps );
	TInt GetCaps(TInt aIndex, RPointerArray<TAvdtpServiceCapability>& );
	virtual ~CBTSACSEPManager();
	const TBTDevAddr& GetDeviceAddr();
private:
	TInt FindSEP(TSEID aSEID, CBTSACStreamEndPoint& aSEP);
	CBTSACSEPManager();
	CBTSACSEPManager(const TBTDevAddr& aAddr);
private:
	RPointerArray<CBTSACStreamEndPoint>	iStreamEndPoints;
	TBTDevAddr iAddr; 
	};
	
	
#endif	