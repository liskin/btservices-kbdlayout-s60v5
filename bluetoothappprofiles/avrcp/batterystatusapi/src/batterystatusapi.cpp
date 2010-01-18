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
 @publishedAll
 @released
*/

#include <bluetooth/logger.h>
#include <remconbatterytarget.h>
#include <remconbatterytargetobserver.h>
#include <remcon/avrcpspec.h>
#include <remconinterfaceselector.h>

#include "remconbattery.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_BATTERY_STATUS);
#endif

EXPORT_C CRemConBatteryApiTarget* CRemConBatteryApiTarget::NewL(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConBatteryTargetObserver& aObserver)
	{
	LOG_STATIC_FUNC

	CRemConBatteryApiTarget* self = new(ELeave) CRemConBatteryApiTarget(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CRemConBatteryApiTarget::CRemConBatteryApiTarget(CRemConInterfaceSelector& aInterfaceSelector, 
		MRemConBatteryTargetObserver& aObserver)
:	CRemConInterfaceBase(TUid::Uid(KRemConBatteryApiUid), 
						 KLengthBatteryStatusMsg, 
						 aInterfaceSelector,
						 ERemConClientTypeTarget), 
	iObserver(aObserver)
	{
	}
	
void CRemConBatteryApiTarget::ConstructL()
	{
	// Create a output buffer (arbitrary value of 32 for granularity of buffer expansion)
	iOutData = CBufFlat::NewL(32);

	// allocate a output buffer big enough for a TRemConBatteryApiResponse  
	iOutData->SetReserveL(sizeof(TPckg<TRemConBatteryApiResponse>)); //do this now for easier writing to buffer
	BaseConstructL();
	}
	
EXPORT_C CRemConBatteryApiTarget::~CRemConBatteryApiTarget()
	{
	delete iOutData;
	}

/** Gets a pointer to a specific interface version.
@return A pointer to the interface, NULL if not supported.
@internalComponent
@released
*/
TAny* CRemConBatteryApiTarget::GetInterfaceIf(TUid aUid)
	{
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConInterfaceIf1) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConInterfaceIf*>(this)
			);
		}

	return ret;
	}

// from MRemConInterfaceIf
void CRemConBatteryApiTarget::MrcibNewMessage(TUint /* aOperationId */, const TDesC8& aData)
	{
	// don't bother to log the operation ID as this was causing a build warning
	LOG1(_L("\taInformBatteryStatusOfCT Data.Length = %d"), aData.Length());

	TRemConBatteryApiResponse batteryApiResponse;
	batteryApiResponse.iError = KErrNone;
	iOutData->Reset();
	
	// check for malformed messages, there should be 1 byte of data
	if ( aData.Length() == 1 )
		{	
		// check the battery status is validvalue to the client observer
		if ( aData[0] <= EFullCharge )
			{
			TControllerBatteryStatus value = (TControllerBatteryStatus)aData[0];
			iObserver.MrcbstoBatteryStatus(value);
			}
		else	
			{
			LOG(_L("\tMalformed message, data out range"));
			batteryApiResponse.iError = KErrAvrcpMetadataInvalidParameter;
			}
		}
	else
		{
		LOG(_L("\tMalformed message, data missing"));
		batteryApiResponse.iError = KErrAvrcpMetadataParameterNotFound;
		}

	// send a response to remcon (if OOM silently drop message)
	TRAPD(error, iOutData->ResizeL(sizeof(TRemConBatteryApiResponse)));
	if (error == KErrNone)
		{
		iOutData->Write(0,TPckg<TRemConBatteryApiResponse>(batteryApiResponse));
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConBatteryApiUid),
												EInformBatteryStatusOfCT, ERemConResponse, iOutData->Ptr(0));
		}
	}
	
