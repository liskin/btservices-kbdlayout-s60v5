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
 @publishedAll
 @released
*/

#include <bluetooth/logger.h>
#include <remconstatusapicontroller.h>
#include <remconstatusapicontrollerobserver.h>
#include <remconinterfaceselector.h>
#include "remconstatusapi.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_STATUS);
#endif

/** Creates new Status Api Controller.

@param aInterfaceSelector An interface selector for use with this interface.
@param aObserver An observer to be notified of responses to commands issued
				to this interface.
@return A fully constructed CRemConStatusApiController
*/
EXPORT_C CRemConStatusApiController* CRemConStatusApiController::NewL(CRemConInterfaceSelector& aInterfaceSelector, 
											  MRemConStatusApiControllerObserver& aObserver)
	{
	LOG_STATIC_FUNC

	CRemConStatusApiController* self = new(ELeave) CRemConStatusApiController(aInterfaceSelector, aObserver);
	CleanupStack::PushL(self);
	self->BaseConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/** Constructs this interface.

@param aInterfaceSelector An interface selector for use with this interface.
@param aObserver An observer to be notified of responses to commands issued
				to this interface.
@internalComponent
@released
*/
CRemConStatusApiController::CRemConStatusApiController(CRemConInterfaceSelector& aInterfaceSelector, 
							   MRemConStatusApiControllerObserver& aObserver)
:	CRemConInterfaceBase(TUid::Uid(KRemConStatusApiUid), 
						 KRemConStatusApiMaxOperationSpecificDataSize, 
						 aInterfaceSelector,
						 ERemConClientTypeController),
	iObserver(aObserver)
	{
	iOutData.Assign(NULL);
	}

/** Destructor.
*/
EXPORT_C CRemConStatusApiController::~CRemConStatusApiController()
	{
	}

/** Gets a pointer to a specific interface version.

@return A pointer to the interface, NULL if not supported.
@internalComponent
@released
*/
TAny* CRemConStatusApiController::GetInterfaceIf(TUid aUid)
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

/** New message from RemCon.

@internalComponent
@released
*/
void CRemConStatusApiController::MrcibNewMessage(TUint aOperationId, const TDesC8& aData)
	{
	LOG_FUNC

	switch ( aOperationId )
		{
	case ERemConStatusApiUnitInfo:
		{
		TInt vendorId, unit, unitType, extendedUnitType;
		SetUnitInfoResponseData(aData, vendorId, unit, unitType, extendedUnitType);
		
		iObserver.MrcsacoUnitInfoResponse(vendorId, unit, unitType, extendedUnitType);
		break;
		}
	case ERemConStatusApiSubunitInfo:
		{
		TInt page, extension;
		TPtrC8 pageData;
		SetSubunitInfoResponseData(aData, page, extension, pageData);
		
		iObserver.MrcsacoSubunitInfoResponse(page, extension, pageData);
		break;
		}
	default:
		break;
		}
	}

/** Parses command data from the buffer.

@param aCommandData The buffer from which to read the data.
@param aVendorId On return, the vendor id
@param aUnit On return, the unit
@param aUnitType On return, the unit type 
@param aExtendedUnitType On return, the extended unit type

@internalComponent
@released
*/
void CRemConStatusApiController::SetUnitInfoResponseData(const TDesC8& aCommandData,
	TInt& aVendorId, TInt& aUnit, TInt& aUnitType, TInt& aExtendedUnitType)
	{
	if(aCommandData.Length())
		{
		ReadCommandDataToInt(aCommandData, KRemConStatusApiVendorIdOffset, 
			KRemConStatusApiVendorIdLength, aVendorId);
		ReadCommandDataToInt(aCommandData, KRemConStatusApiUnitOffset, 
			KRemConStatusApiUnitLength, aUnit);
		ReadCommandDataToInt(aCommandData, KRemConStatusApiUnitTypeOffset, 
			KRemConStatusApiUnitTypeLength, aUnitType);
		ReadCommandDataToInt(aCommandData, KRemConStatusApiExtendedUnitTypeOffset, 
			KRemConStatusApiExtendedUnitTypeLength, aExtendedUnitType);
		}
	else
		{
		// On error set all fields to 0
		aVendorId = aUnit = aUnitType = aExtendedUnitType = 0;
		}
	}

/** Parses command data from the buffer.

@param aCommandData The buffer from which to read the data.
@param aPage On return, the page
@param aExtension On return, the extension
@param aPageData On return, the page data

@internalComponent
@released
*/
void CRemConStatusApiController::SetSubunitInfoResponseData(const TDesC8& aCommandData,
	TInt& aPage, TInt& aExtension, TPtrC8& aPageData)
	{
	if(aCommandData.Length())
		{
		ReadCommandDataToInt(aCommandData, KRemConStatusApiPageOffset, 
			KRemConStatusApiPageLength, aPage);
		ReadCommandDataToInt(aCommandData, KRemConStatusApiExtensionOffset, 
			KRemConStatusApiExtensionLength, aExtension);

		aPageData.Set(aCommandData.Mid(KRemConStatusApiPageDataOffset));
		}
	else
		{
		// On error set all fields to 0
		aPage = aExtension = 0;
		aPageData.Set(KNullDesC8());
		}
	}
	
/** Reads command data from the buffer to an int.

@param aCommandData The buffer from which to read the data.
@param aOffset The offset within aCommandData read from.
@param aLength The length of data to read.  This must not be
				more than 4.
@param aValue On return, the value of the specified data section.

@internalComponent
@released
*/	
void CRemConStatusApiController::ReadCommandDataToInt(const TDesC8& aCommandData, 
	TInt aOffset, TInt aLength, TInt& aValue)
	{
	__ASSERT_ALWAYS(aLength <= 4, CRemConStatusApiController::Panic(EStatusApiCommandDataSectionTooLong));
	
	aValue = 0;
	
	for(TInt i = 0 ; i < aLength; i++)
		{
		aValue |= aCommandData[i+aOffset]<<(8*i);
		}
	}

/** Issue UnitInfo command.
Only one command per controller session can be outstanding at any one time.
@param aStatus Status to be completed with the result of issuing this command.
@param aNumRemotes On completion, the number of remotes this command was issued to.
*/
EXPORT_C void CRemConStatusApiController::UnitInfo(TRequestStatus& aStatus, TUint& aNumRemotes)
	{
	LOG_FUNC

	InterfaceSelector().Send(aStatus, 
		TUid::Uid(KRemConStatusApiUid),
		(TUint)ERemConStatusApiUnitInfo, 
		aNumRemotes,
		ERemConCommand,
		iOutData);
	}

/** Issue SubUnitInfo command.
Only one command per controller session can be outstanding at any one time.
@param aStatus Status to be completed with the result of issuing this command.
@param aNumRemotes On completion, the number of remotes this command was issued to.
*/
EXPORT_C void CRemConStatusApiController::SubunitInfo(TRequestStatus& aStatus, TUint& aNumRemotes)
	{
	LOG_FUNC

	InterfaceSelector().Send(aStatus, 
		TUid::Uid(KRemConStatusApiUid), 
		(TUint)ERemConStatusApiSubunitInfo, 
		aNumRemotes,
		ERemConCommand);
	}
	
/** Utility Status Converter panic function.

@param aPanic The panic number.
@internalComponent
@released
*/
void CRemConStatusApiController::Panic(TStatusApiPanic aPanic)
	{
	User::Panic(KStatusApiPanicName, aPanic);
	}

//
// End of file
