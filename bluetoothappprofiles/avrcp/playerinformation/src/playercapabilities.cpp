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
// This file contains the capabilites part of playerinformation.
// 
//

/**
 @file
 @publishedAll
 @released
*/

#include <bluetooth/logger.h>
#include <remconinterfaceselector.h>
#include <remcon/avrcpspec.h>
#include <playerinformationtarget.h>

#include "eventsmask.h"
#include "playerinformation.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_PLAYER_INFO);
#endif

class TRemConMetadataTransferGetCapabilities 
	{
public:
	TUint8	 				iCapabilityID;
	};

EXPORT_C void MPlayerCapabilitiesObserver::ClearEvents()
	{
	DoClearEvents();
	}

EXPORT_C TInt MPlayerCapabilitiesObserver::AddEvent(TRegisterNotificationEvent aEvent)
	{
	return DoAddEvent(aEvent);
	}
	
EXPORT_C TInt MPlayerCapabilitiesObserver::RemoveEvent(TRegisterNotificationEvent aEvent)
	{
	return DoRemoveEvent(aEvent);
	}

EXPORT_C void MPlayerCapabilitiesObserver::ClearCompanyIds()
	{
	DoClearCompanyIds();
	}

EXPORT_C TInt MPlayerCapabilitiesObserver::AddCompanyId(TInt aID)
	{
	return DoAddCompanyId( aID );
	}

EXPORT_C TInt MPlayerCapabilitiesObserver::RemoveCompanyID(TInt aID)
	{
	return DoRemoveCompanyID( aID );
	}


// from MPlayerCapabilitiesObserver
void CPlayerInfoTarget::DoClearEvents()
	{
	// AVRCP 1.3 specification, Appendix H says that PlaybackStatusChanged
	// and TrackChanged event notification support is mandatory, so add these
	
	iSupportedNotificationEventList->Reset();
	iSupportedNotificationEventList->Append(ERegisterNotificationPlaybackStatusChanged);
	iSupportedNotificationEventList->Append(ERegisterNotificationTrackChanged);
	}


TInt CPlayerInfoTarget::DoAddEvent( TRegisterNotificationEvent aEvent )
	{
	// check the event is supported (note system status is NOT supported)
	if (   aEvent >= ERegisterNotificationReservedLast
		|| aEvent == ERegisterNotificationSystemStatusChanged_NotSupported)
		{
		return KErrNotSupported;
		}
		
	// check if aEvent is already in the list of supported events
	if (!iSupportedNotificationEventList->Find(aEvent))
		{
		// not found so add it to the list
		iSupportedNotificationEventList->Append(aEvent);
		return KErrNone;
		}
	
	// else return already in the list
	return KErrAlreadyExists;
	}


TInt CPlayerInfoTarget::DoRemoveEvent( TRegisterNotificationEvent aEvent )
	{
	// AVRCP 1.3 specification, Appendix H says that PlaybackStatusChanged
	// and TrackChanged event notification support is mandatory, so add these
	TInt err = KErrNone;
	if (   aEvent == ERegisterNotificationPlaybackStatusChanged
		|| aEvent == ERegisterNotificationTrackChanged)
		{
		return KErrNotSupported;
		}
	
	// Find aEvent in the list of supported events
	if (iSupportedNotificationEventList->Find(aEvent))
		{
		// and then remove it
		iSupportedNotificationEventList->Remove(aEvent);
		TInt pendingPos = iPendingNotificationEventList.Find(aEvent);
		if (pendingPos != KErrNotFound)
			{
			iPendingNotificationEventList.Remove( pendingPos );
			
			// Tell anyone waiting for a notification that an event has been
			// removed. Alas, we can't specify _WHICH_ event has been removed
			// since there's no way to indicate this in an INTERNAL_ERROR response
			SendError(KErrAvrcpMetadataInternalError, 
					  RAvrcpIPC::SetIPCOperationIdFromEventId(aEvent), 
					  ERemConNotifyResponseChanged);
			}
		}
	else
		{
		err = KErrNotFound;
		}
	return err;
	}


void CPlayerInfoTarget::DoClearCompanyIds()
	{
	// The Bluetooth SIG vendor id must always be present
	// See AVRCP 1.3 Specification, section 5.5.1, table 5.4
	iCompanyIdList.Reset();
	iCompanyIdList.Append(KBluetoothSIGVendorId);
	}


TInt CPlayerInfoTarget::DoAddCompanyId( TInt aID )
	{
	if ( aID > KMaxCompanyID )
		{
		return KErrNotSupported;
		}
		
	// check if the ID is already in the list
	TInt pos = iCompanyIdList.Find( aID );

	// add only if not present in list to avoid duplicates
	if (pos != KErrNotFound)
		{
		return KErrAlreadyExists;
		}
	
	// make sure the list cannot contain more than 255 items
	if (iCompanyIdList.Count() >= KMaxNrOfCompanyIDs)
		{
		return KErrOverflow;
		}
		
	return iCompanyIdList.Append( aID );
	}


TInt CPlayerInfoTarget::DoRemoveCompanyID( TInt aID )
	{
	// The Bluetooth SIG vendor id must always be present
	// See AVRCP 1.3 Specification, section 5.5.1, table 5.4
	if ( aID == KBluetoothSIGVendorId )
		{
		return KErrNotSupported;
		}
		
	// check if the ID is in the list
	TInt pos = iCompanyIdList.Find( aID );

	// if found remove else do nothing
	if (pos != KErrNotFound)
		{
		iCompanyIdList.Remove( pos );
		return KErrNone;
		}
	return pos;
	}


void CPlayerInfoTarget::ProcessGetCapabilities(const TDesC8& aData)
	{
	LOG_STATIC_FUNC

	/* Decode the get capability message */
	TRemConMetadataTransferGetCapabilities getCapability;
	
	// check there is at least 1 byte of data
	if (!aData.Length())
		{
		// Invalid packet
		return SendError(KErrAvrcpMetadataInvalidParameter, EGetCapabilities);
		}
		
	TPckgC<TRemConMetadataTransferGetCapabilities> data(*reinterpret_cast<const TRemConMetadataTransferGetCapabilities*>(aData.Ptr()));
	getCapability = data();
	TGetCapabilityValues id = (TGetCapabilityValues) getCapability.iCapabilityID;
	
	// format the response in a TRemConGetCapabilitiesResponse
	RRemConGetCapabilitiesResponse response;

	switch ( id )
		{
	case ECapabilityIdCompanyID:
		// respond with ECapabilityIdCompanyID
		response.iCapabilityId = ECapabilityIdCompanyID;

		// followed by number of IDs
		response.iCapabilityCount = iCompanyIdList.Count();

		// and then list of company ids,
		for (TInt i=0; i< iCompanyIdList.Count(); i++ )
			{
			TInt x = iCompanyIdList[i];
			if (response.iCapabilities.Append(x) != KErrNone)
				{
				response.Close();
				return SendError(KErrAvrcpMetadataInternalError, EGetCapabilities);   // Try to send internal error if OOM
				}
			}
		break;
		
	case ECapabilityIdEventsSupported:
		// respond with ECapabilityIdEventsSupported
		response.iCapabilityId = ECapabilityIdEventsSupported;
		
		// followed by number of supported events, and then list of events
		response.iCapabilityCount = 0;
		iSupportedNotificationEventList->Begin();
		while(iSupportedNotificationEventList->Next())
			{
			if (KErrNone != response.iCapabilities.Append(iSupportedNotificationEventList->Get()))
				{
				response.Close();
				return SendError(KErrAvrcpMetadataInternalError, EGetCapabilities);   // Try to send internal error if OOM
				}
			response.iCapabilityCount++;
			}
		break;	
		
	default:
		/* other IDs are reserved */
		return SendError(KErrAvrcpMetadataInvalidParameter, EGetCapabilities);  // Invalid packet
		}

	TInt error = 0;
	TRAP(error, response.WriteL(iOutBuf));   // Don't send error if OOM
	response.Close();
	if (error == KErrNone)
		// send the result back to the CT
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
											EGetCapabilities, ERemConResponse, iOutBuf );
	}

