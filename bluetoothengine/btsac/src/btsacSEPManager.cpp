/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE FILES
#include <bluetoothav.h>

#include "btsacSEPManager.h"   
#include "btsacStreamEndPoint.h"
#include "debug.h"


// CONSTANTS
const TInt KBTAddLength = 12; // BT address length

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTSACSEPManager::NewL
// -----------------------------------------------------------------------------
//
CBTSACSEPManager* CBTSACSEPManager::NewL(const TBTDevAddr& aAddr)
	{
	return new(ELeave) CBTSACSEPManager(aAddr);
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::CBTSACSEPManager
// -----------------------------------------------------------------------------
//
CBTSACSEPManager::CBTSACSEPManager(const TBTDevAddr& aAddr)	
: iAddr(aAddr)
	{
	}

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTSACSEPManager::~CBTSACSEPManager()
	{
	TRACE_FUNC
	iStreamEndPoints.ResetAndDestroy();
	iStreamEndPoints.Close();
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::FindSEP
// -----------------------------------------------------------------------------
//
TInt CBTSACSEPManager::FindSEP(TSEID aSEID, CBTSACStreamEndPoint& aSEP)
	{
	TRACE_FUNC
    CBTSACStreamEndPoint* SEP = &aSEP; 
	SEP = SEP; // remove compiler warning
	for (TInt i=0;i<iStreamEndPoints.Count();i++)
		{
		if (iStreamEndPoints[i]->SEID()==aSEID)
			{
			 SEP = iStreamEndPoints[i];
			 return KErrNone;
			}
		}
	TRACE_INFO((_L("CBTSACSEPManager::FindSEP() SEP Not Found !")))
	return KErrNotFound;
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::GetSEP
// -----------------------------------------------------------------------------
//
TInt CBTSACSEPManager::GetSEP(TSEID aSEID, CBTSACStreamEndPoint& aSEP)
	{
	TRACE_FUNC
	return FindSEP(aSEID, aSEP);
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::SetState
// -----------------------------------------------------------------------------
//	
TInt CBTSACSEPManager::SetState(TSEID aSEID, CBTSACStreamEndPoint::TSEPState aState, const TAny* aBlob)
	{
	TRACE_FUNC
	CBTSACStreamEndPoint* SEP = NULL;
	
	if (!FindSEP(aSEID, *SEP) )
		{	
		SEP->SetState(aState, aBlob); 
		return KErrNone; 
		}
	else
		{
		TRACE_INFO((_L("CBTSACSEPManager::SetState() SEP Not Found !")))
		return KErrNotFound; 	
		}	
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::GetDeviceAddr
// -----------------------------------------------------------------------------
//
const TBTDevAddr& CBTSACSEPManager::GetDeviceAddr()
	{
	TRACE_FUNC
	TBuf<KBTAddLength> bdaddrbuf;
    iAddr.GetReadable( bdaddrbuf );
	TRACE_INFO((_L("CBTSACSEPManager::GetDeviceAddr() %S"), &bdaddrbuf))
	return iAddr; 
	}


// -----------------------------------------------------------------------------
// CBTSACSEPManager::StoreCaps
// -----------------------------------------------------------------------------
//	
TInt CBTSACSEPManager::StoreCaps(TInt aIndex, TAvdtpServiceCapability* aCaps)
	{
	TRACE_INFO((_L("CBTSACSEPManager::StoreCaps() %d"), aIndex))
	if (aIndex < iStreamEndPoints.Count() )
		{
		iStreamEndPoints[aIndex]->StoreCaps(aCaps); 
		return KErrNone; 
		}
	else
		{
		TRACE_INFO((_L("CBTSACSEPManager::StoreCaps() SEP Not Found !")))
		return KErrNotFound; 	
		}	
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::GetCaps
// -----------------------------------------------------------------------------
//	
TInt CBTSACSEPManager::GetCaps(TInt aIndex, RPointerArray<TAvdtpServiceCapability>& aCaps )
	{
	TRACE_INFO((_L("CBTSACSEPManager::GetCaps() %d"), aIndex))
	if (aIndex < iStreamEndPoints.Count() )
		{
		iStreamEndPoints[aIndex]->GetCaps(aCaps); 
		return KErrNone; 
		}
	else
		{
		TRACE_INFO((_L("CBTSACSEPManager::GetCaps() SEP Not Found !")))
		return KErrNotFound; 	
		}	
	}
	
// -----------------------------------------------------------------------------
// CBTSACSEPManager::NoOfSEPs
// -----------------------------------------------------------------------------
//
TInt CBTSACSEPManager::NoOfSEPs()
	{
	TRACE_INFO((_L("CBTSACSEPManager::NoOfSEPs() %d"), iStreamEndPoints.Count()))
	return iStreamEndPoints.Count();
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::GetInfo
// -----------------------------------------------------------------------------
//	
TInt CBTSACSEPManager::GetInfo(TInt aIndex, TAvdtpSEPInfo& aSEPInfo)
	{
	TRACE_INFO((_L("CBTSACSEPManager::GetInfo() %d"), aIndex))
	if (aIndex < iStreamEndPoints.Count() )
		{
		iStreamEndPoints[aIndex]->GetInfo(aSEPInfo); 
		return KErrNone; 
		}
	else
		{
		TRACE_INFO((_L("CBTSACSEPManager::GetInfo() SEP Not Found !")))
		return KErrNotFound; 	
		}	
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::BindSEPs
// -----------------------------------------------------------------------------
//	
void CBTSACSEPManager::BindSEPs(TSEID /*aLocalSEID*/, TSEID /*aRemoteSEID*/)
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBTSACSEPManager::NewSEPL
// -----------------------------------------------------------------------------
//
void CBTSACSEPManager::NewSEPL(const TAvdtpSEPInfo& aInfo)
	{
	TRACE_FUNC
	CBTSACStreamEndPoint* sep = CBTSACStreamEndPoint::NewL(aInfo);
	CleanupStack::PushL(sep);
	User::LeaveIfError(iStreamEndPoints.Append(sep));
	CleanupStack::Pop(sep);
	}

// end of class
