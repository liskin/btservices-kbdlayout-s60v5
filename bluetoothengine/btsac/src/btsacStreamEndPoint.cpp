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
#include "debug.h"


// -----------------------------------------------------------------------------
// CBTSACStreamEndPoint::NewL
// -----------------------------------------------------------------------------
//
CBTSACStreamEndPoint* CBTSACStreamEndPoint::NewL(const TAvdtpSEPInfo& aInfo)
	{
	CBTSACStreamEndPoint* sep = new(ELeave) CBTSACStreamEndPoint(aInfo);
	return sep;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamEndPoint::CBTSACStreamEndPoint
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CBTSACStreamEndPoint::CBTSACStreamEndPoint(const TAvdtpSEPInfo& aInfo)
: iInfo(aInfo)
	{
	
	}

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTSACStreamEndPoint::~CBTSACStreamEndPoint()
	{
	iSEPCapabilities.ResetAndDestroy();
	iSEPCapabilities.Close();
	}

	
// -----------------------------------------------------------------------------
// CBTSACStreamEndPoint::GetInfo
// -----------------------------------------------------------------------------
//
void CBTSACStreamEndPoint::GetInfo(TAvdtpSEPInfo& aInfo)
	{
	TRACE_FUNC
	aInfo = iInfo; 
	}

// -----------------------------------------------------------------------------
// CBTSACStreamEndPoint::StoreCaps
// -----------------------------------------------------------------------------
//
void CBTSACStreamEndPoint::StoreCaps(TAvdtpServiceCapability* aCaps)
	{
	TRACE_FUNC
	iSEPCapabilities.Append(aCaps);
	}
	
// -----------------------------------------------------------------------------
// CBTSACStreamEndPoint::GetCaps
// -----------------------------------------------------------------------------
//
void CBTSACStreamEndPoint::GetCaps(RPointerArray<TAvdtpServiceCapability>& aCaps)
	{
	TRACE_FUNC
	TInt count = iSEPCapabilities.Count();
	for (TInt i = 0; i < count; i++)
		{
		aCaps.Append(iSEPCapabilities[i]);
		}
	}
	
// -----------------------------------------------------------------------------
// CBTSACStreamEndPoint::SetState
// -----------------------------------------------------------------------------
//
void CBTSACStreamEndPoint::SetState(TSEPState aState, const TAny* /*aBlob=NULL*/)
	{
	TRACE_FUNC
	switch (aState)
		{
		case EIdle:
			{
			TRACE_INFO((_L("CBTSACStreamEndPoint::SetState KSEPIdle")))
			break;
			}
		case EDiscoveredRemote:
		case ERegistered:
		case ERemotelyBound:
			{
			// ERemotelyBound -> extra param is the remote to which we are bound
			break;
			}
		case ELocallyConfigured:
			{
			TRACE_INFO((_L("CBTSACStreamEndPoint::SetState KSEPLocallyConfigured")))
			break;
			}
		case EOpen:
		case EStreaming:
		case ESuspended:
		case EAborted:
			{
			break;
			}

		default:
			{
			TRACE_INFO((_L("CBTSACStreamEndPoint::SetState ***UNKNOWN***")))
			}
		}
	}

// end of file
