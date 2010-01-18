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
// This file contains the events part of the PlayerInformation API.
// 
//

/**
 @file
 @publishedAll
 @released
*/

#include <bluetooth/logger.h>
#include <remconinterfaceselector.h>
#include <remconbeareravrcp.h>
#include <playerinformationtarget.h>
#include <playerinformationtargetobserver.h>

#include "playerapplicationsetting.h"
#include "eventsmask.h"
#include "playereventsutils.h"
#include "playerinformation.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_PLAYER_INFO);
#endif

EXPORT_C void MPlayerEventsObserver::PlaybackStatusChanged(TPlaybackStatus aStatus)
	{
	DoPlaybackStatusChanged(aStatus);
	}

EXPORT_C void MPlayerEventsObserver::TrackChanged(TUint64 aTrackId, TUint32 aLengthInMilliseconds) 
	{
	DoTrackChanged(aTrackId, aLengthInMilliseconds); 
	}

EXPORT_C void MPlayerEventsObserver::TrackReachedEnd() 
	{
	DoTrackReachedEnd();
	}

EXPORT_C void MPlayerEventsObserver::TrackReachedStart()
	{
	DoTrackReachedStart();
	}

EXPORT_C void MPlayerEventsObserver::SetPlaybackPosition(TUint32 aMilliseconds) 
	{
	DoSetPlaybackPosition(aMilliseconds); 
	}
	
EXPORT_C void MPlayerEventsObserver::SetBatteryStatus(TTargetBatteryStatus aBatteryStatus) 
	{
	DoSetBatteryStatus(aBatteryStatus); 
	}

void CPlayerInfoTarget::ProcessGetStatusAndBeginObserving( const TUint aOperationId, TRegisterNotificationEvent aEventId, const TDesC8& aData)
	{
	// check the event is in the supported list
	if (!iSupportedNotificationEventList->Find(aEventId))
		{
		// Not supported so return error
		SendError(KErrAvrcpMetadataInvalidParameter, aOperationId, ERemConNotifyResponseInterim);
		return;
		}
		
	// if the event is EPlaybackPosChanged then the timeinterval is included in the RegisterNofication
	if (aEventId == ERegisterNotificationPlaybackPosChanged)
		{
		// decode the playback interval from aData
		TInt error = 0;
		RRemConPlayerInformation32BitResponse request;
		TRAP(error, request.ReadL(aData));
		
		if (error != KErrNone)
			{
			// Not supported so return error
			SendError(KErrAvrcpMetadataParameterNotFound, aOperationId, ERemConNotifyResponseInterim);
			return;
			}
		
		// save the playback interval
		iPlayBackIntervalInMilliseconds = request.iValue * 1000;
		
		// and the current position
		iLastNotifiedPlaybackPositionInMilliseconds = iPlaybackPositionInMilliseconds;
		}
		
	// and request another notification (if there is not one already pending)
	// on the next state change  
	if (KErrNotFound ==iPendingNotificationEventList.Find(aEventId))
		{
		if (iPendingNotificationEventList.Append(aEventId) != KErrNone)
			{
			return SendError(KErrAvrcpMetadataInternalError, aOperationId);   // Try to send internal error if OOM
			}
		}
		
	// send the current status
	SendNotificationResponse(aEventId, ERemConNotifyResponseInterim );
	}

void CPlayerInfoTarget::ProcessGetStatus( const TUint aOperationId, TRegisterNotificationEvent aEventId)
	{
	// send the current value
	if (iSupportedNotificationEventList->Find(aEventId))
		{
		SendNotificationResponse(aEventId, ERemConNotifyResponseChanged );
		}
	else
		{
		// Not found so return error
		SendError(KErrAvrcpMetadataInvalidParameter, aOperationId, ERemConNotifyResponseChanged);
		}
	}
		
/* ProcessGetPlayStatus This returns the current values of SongLength, 
   SongPosition and PlayStatus.
   Note if SongLength and SongPosition are not supported KPlaybackPositionUnknown is returned
 */
void CPlayerInfoTarget::ProcessGetPlayStatus()
	{
	LOG_FUNC

	TInt error =0;

	// format the response in a RRemConPlayerInformationGetPlayStatusResponse
	RRemConPlayerInformationGetPlayStatusResponse response;
	response.iTrackLength = iLengthInMilliseconds;
	response.iPlayPos = iPlaybackPositionInMilliseconds;
	response.iStatus = iPlayBackStatus;
	
	TRAP(error, response.WriteL(iOutBuf));
	if (error == KErrNone)
		{
		// send the response back to the CT
		error = InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
												EGetPlayStatus, ERemConResponse, iOutBuf );
		}
	}

/* ProcessGetPlayStatusUpdate returns the current play status if it has changed
 * relative to the provided play status, otherwise the request is queued until
 * the play status changes.
 */
void CPlayerInfoTarget::ProcessGetPlayStatusUpdate(const TDesC8& aData)
	{
	LOG_FUNC

	// Bearer should never send us more than one of these in parallel
	__ASSERT_DEBUG(!iPlaybackStatusUpdatePending, PlayerEventsUtils::Panic(ETwoGetPlayStatusUpdatesQueued));
	
	TInt error =0;
	RRemConPlayerInformationGetPlayStatusUpdateRequest request;
	TRAP(error, request.ReadL(aData));
	__ASSERT_DEBUG(error == KErrNone, PlayerEventsUtils::Panic(EBadlyFormattedInternalData));

	if(request.iStatus != iPlayBackStatus)
		{
		// format the response in a RRemConPlayerInformationGetPlayStatusResponse
		RRemConPlayerInformationGetPlayStatusUpdateResponse response;
		response.iStatus = iPlayBackStatus;
		
		TRAP(error, response.WriteL(iOutBuf));
		if (error == KErrNone)
			{
			// send the response back to the CT
			error = InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
													EGetPlayStatusUpdate, ERemConResponse, iOutBuf );
			if(error != KErrNone)
				{
				// We will try and send the response again next time we get an update
				iPlaybackStatusUpdatePending = ETrue;
				}
			}
		}
	else
		{
		iPlaybackStatusUpdatePending = ETrue;
		}
	}

// Send a Notification message for aEventID 
void CPlayerInfoTarget::SendNotificationResponse( TRegisterNotificationEvent aEventId, TRemConMessageSubType aMsgSubType )
	{
	LOG_FUNC

	TInt error = 0;
	iOutBuf.Zero();

	if (!iSupportedNotificationEventList->Find(aEventId))
		{
		return SendError(KErrAvrcpMetadataInvalidParameter, 
						 RAvrcpIPC::SetIPCOperationIdFromEventId(aEventId), aMsgSubType);
		}
		
	switch (aEventId)
		{
		case ERegisterNotificationTrackReachedEnd:
		case ERegisterNotificationTrackReachedStart:
			{
			// no extra data for reached start or end 
			RAvrcpIPCError response;
			response.iError = KErrNone;
			TRAP(error, response.WriteL(iOutBuf));
			break;
			}

		case ERegisterNotificationPlaybackStatusChanged:
			{
			// 8bit response -- current playback status 
			RRemConPlayerInformation8BitResponse response;
			response.iValue = iPlayBackStatus;
			TRAP(error, response.WriteL(iOutBuf));
			break;
			}

		case ERegisterNotificationBatteryStatusChanged:
			{
			// 8bit response -- current battery status
			RRemConPlayerInformation8BitResponse response;
			response.iValue = iBatteryStatus;
			TRAP(error, response.WriteL(iOutBuf));
			break;
			}

		case ERegisterNotificationPlaybackPosChanged:
			{
			// 32bit response -- current playback position in millisecond 
			RRemConPlayerInformation32BitResponse response;
			response.iValue = iPlaybackPositionInMilliseconds;
			TRAP(error, response.WriteL(iOutBuf));
			break;
			}

		case ERegisterNotificationTrackChanged:
			{
			// 64bit response -- index of the current track 
			RRemConPlayerInformation64BitResponse response;
			response.iValue = iTrackId;
			TRAP(error, response.WriteL(iOutBuf));
			break;
			}

		case ERegisterNotificationPlayerApplicationSettingChanged:
			{
			// Send a list of the settings that have changed followed by their value 
			// starting with the number of attributes to be sent 
			RRemConPlayerAttributeIdsAndValues response;
			response.iNumberAttributes = 0;
		
			// for every entry in the list
			THashMapIter<TInt, CPlayerApplicationSettings*> iter(iPlayerApplicationSettings);
			CPlayerApplicationSettings* const* setting = iter.NextValue();
			while ( setting != NULL )
				{
				TUint8 value = (*setting)->GetCurrentValue();
				TInt ret1 = response.iAttributeValue.Append(value);
				TInt ret2 = response.iAttributeId.Append((*setting)->GetAttributeID());
				if (ret1 != KErrNone || ret2 != KErrNone)
					{
					response.Close();
					return SendError(KErrAvrcpMetadataInternalError, 
									 RAvrcpIPC::SetIPCOperationIdFromEventId(aEventId),
									 aMsgSubType);   // Try to send internal error if OOM
					}
				response.iNumberAttributes++;
				setting = iter.NextValue();
				}
				
			TRAP(error, response.WriteL(iOutBuf));
			response.Close();
			break;
			}
		default:
			return;
		}
	
	if (error == KErrNone)
		{
		// send the response back to the CT
		TUint operationId = RAvrcpIPC::SetIPCOperationIdFromEventId(aEventId);
		error = InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
												operationId, ERemConResponse, aMsgSubType, iOutBuf);
		}
	}
	 
// from MPlayerEventsObserver
 void CPlayerInfoTarget::DoPlaybackStatusChanged(TPlaybackStatus  aStatus)
	{
	LOG_FUNC

	iPlayBackStatus = aStatus;
	
	// if the playback status is in the current event list
	TInt pos = iPendingNotificationEventList.Find( ERegisterNotificationPlaybackStatusChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		SendNotificationResponse( ERegisterNotificationPlaybackStatusChanged, ERemConNotifyResponseChanged);
		}
	
	pos = iPendingNotificationEventList.Find( ERegisterNotificationPlaybackPosChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		SendNotificationResponse(ERegisterNotificationPlaybackPosChanged, ERemConNotifyResponseChanged);
		}
	
	if(iPlaybackStatusUpdatePending)
		{
		// format the response in a RRemConPlayerInformationGetPlayStatusResponse
		RRemConPlayerInformationGetPlayStatusUpdateResponse response;
		response.iStatus = iPlayBackStatus;
		
		TRAPD(error, response.WriteL(iOutBuf));
		if (error == KErrNone)
			{
			// send the response back to the CT
			error = InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
													EGetPlayStatusUpdate, ERemConResponse, iOutBuf );
			if(error == KErrNone)
				{
				iPlaybackStatusUpdatePending = EFalse;
				}
			// if we did error we will try and respond again next time the client's play status
			// changes
			}
		}
	}
	
void CPlayerInfoTarget::DoTrackChanged(TUint64 aTrackId, TUint32 aLengthInMilliseconds)
	{
	LOG_FUNC

	iTrackId = aTrackId;
	iLengthInMilliseconds = aLengthInMilliseconds;

	// if the playback status is in the current event list 
	TInt pos = iPendingNotificationEventList.Find( ERegisterNotificationTrackChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		SendNotificationResponse(ERegisterNotificationTrackChanged, ERemConNotifyResponseChanged);
		}

	pos = iPendingNotificationEventList.Find( ERegisterNotificationPlaybackPosChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		iPlaybackPositionInMilliseconds = 0;	//set 0 to current position for new track
		SendNotificationResponse(ERegisterNotificationPlaybackPosChanged, ERemConNotifyResponseChanged);
		}
	}

void CPlayerInfoTarget::DoTrackReachedEnd()
	{
	LOG_FUNC
	
	iTrackPosition = EEnd;

	// if the ETrackReachedEnd status is in the current event list 
	TInt pos = iPendingNotificationEventList.Find( ERegisterNotificationTrackReachedEnd );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		SendNotificationResponse(ERegisterNotificationTrackReachedEnd, ERemConNotifyResponseChanged);
		}
	
	pos = iPendingNotificationEventList.Find( ERegisterNotificationPlaybackPosChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		iPlaybackPositionInMilliseconds = iLengthInMilliseconds;
		SendNotificationResponse(ERegisterNotificationPlaybackPosChanged, ERemConNotifyResponseChanged);
		}
	}

void CPlayerInfoTarget::DoTrackReachedStart()
	{
	LOG_FUNC

	iTrackPosition = EStart;

	// if the ETrackReachedStart status is in the current event list 
	TInt pos = iPendingNotificationEventList.Find( ERegisterNotificationTrackReachedStart );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		SendNotificationResponse(ERegisterNotificationTrackReachedStart, ERemConNotifyResponseChanged);
		}
	
	pos = iPendingNotificationEventList.Find( ERegisterNotificationPlaybackPosChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		iPlaybackPositionInMilliseconds = 0;
		SendNotificationResponse(ERegisterNotificationPlaybackPosChanged, ERemConNotifyResponseChanged);
		}
	}

void CPlayerInfoTarget::DoSetPlaybackPosition(TUint32 aMilliseconds)		
	{
	LOG_FUNC
	
	iPlaybackPositionInMilliseconds = aMilliseconds;

	// if the playback position is in the current Notification event list 
	TInt pos = iPendingNotificationEventList.Find( ERegisterNotificationPlaybackPosChanged );
	if (pos != KErrNotFound)
		{
		// a notification has been requested, now check if it is due

		TUint32 difference = (iPlaybackPositionInMilliseconds > iLastNotifiedPlaybackPositionInMilliseconds) ?
        	iPlaybackPositionInMilliseconds - iLastNotifiedPlaybackPositionInMilliseconds:
        	iLastNotifiedPlaybackPositionInMilliseconds - iPlaybackPositionInMilliseconds;

		if (difference >= iPlayBackIntervalInMilliseconds)
			{
			// Due now so send
			iPendingNotificationEventList.Remove( pos );
			SendNotificationResponse(ERegisterNotificationPlaybackPosChanged, ERemConNotifyResponseChanged);			
			}	
		}
	}

void CPlayerInfoTarget::DoSetBatteryStatus(TTargetBatteryStatus aBatteryStatus)
	{
	LOG_FUNC

	TBool validStatus = ETrue;
	if (aBatteryStatus < MPlayerEventsObserver::ENormal || 
		aBatteryStatus > MPlayerEventsObserver::EFullCharge)
		{
		LOG1(_L("Invalid battery status: %d"),aBatteryStatus);
		validStatus = EFalse;
		}
	else
		{
		iBatteryStatus = aBatteryStatus;
		}

	// if the battery status is in the current event list
	TInt pos = iPendingNotificationEventList.Find( ERegisterNotificationBatteryStatusChanged );
	if (pos != KErrNotFound)
		{
		iPendingNotificationEventList.Remove( pos );
		if (validStatus)
			{
			SendNotificationResponse(ERegisterNotificationBatteryStatusChanged, ERemConNotifyResponseChanged);
			}
		}
	}
	
void CPlayerInfoTarget::SendError(TInt aError, TInt aOperationId, TRemConMessageSubType aSubType)
	{
	TInt error = 0;
	RAvrcpIPCError response;
	response.iError = aError;
	TRAP(error, response.WriteL(iOutBuf));   // Don't send error if OOM
	if (error == KErrNone)
		InterfaceSelector().SendUnreliable(TUid::Uid(KRemConPlayerInformationUid),
										aOperationId, ERemConResponse, aSubType, iOutBuf);
	}

