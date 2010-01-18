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
#include <playerinformationtarget.h>
#include <e32property.h> 
#include <hwrmpowerstatesdkpskeys.h>

#include "playerapplicationsetting.h"
#include "eventsmask.h"
#include "playerinformation.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_PLAYER_INFO);
#endif

EXPORT_C CPlayerInfoTarget* CPlayerInfoTarget::NewL(CRemConInterfaceSelector& aInterfaceSelector,
										MPlayerCapabilitiesObserver*& aPlayerCapabilitiesObserver,
										MPlayerApplicationSettingsObserver*& aPlayerApplicationSettingsObserver,
										MPlayerEventsObserver*& aPlayerEventsObserver,
										MPlayerApplicationSettingsNotify& aApplicationSettingNotifier )
	{
	LOG_STATIC_FUNC

	CPlayerInfoTarget* self = new(ELeave) CPlayerInfoTarget(aInterfaceSelector, aApplicationSettingNotifier);
	CleanupStack::PushL(self);
	self->ConstructL( aPlayerCapabilitiesObserver, aPlayerApplicationSettingsObserver, aPlayerEventsObserver );
	CleanupStack::Pop(self);
	return self;
	}
	
/** 
Constructs this interface.
@param aInterfaceSelector An interface selector for use with this interface.
@internalComponent
@released
*/
CPlayerInfoTarget::CPlayerInfoTarget(CRemConInterfaceSelector& aInterfaceSelector, MPlayerApplicationSettingsNotify& aApplicationSettingNotifier)
:	CRemConInterfaceBase(TUid::Uid(KRemConPlayerInformationUid), 
						 KMaxLengthPlayerInformationMsg,
						 aInterfaceSelector,
						 ERemConClientTypeTarget),
	iApplicationSettingNotifier(aApplicationSettingNotifier),
	iPlaybackStatusUpdatePending(EFalse)
	{
	}
	
		
void CPlayerInfoTarget::ConstructL(MPlayerCapabilitiesObserver*& aPlayerCapabilitiesObserver,
										MPlayerApplicationSettingsObserver*& aPlayerApplicationSettingsObserver,
										MPlayerEventsObserver*& aPlayerEventsObserver )
	{
	// reserve two AV/C frames, for this reasoning:
	// GetCapabilities (PDU 0x10) can generate a response
	// which is (255 * 3 bytes) + 2 bytes = 767 total and
	// will fit into two AVC frames. Other PDUs generate
	// less than this, except for PDUs 0x15 and 0x16 which
	// use their own buffer pre-allocated by calling Size()
	iOutBuf.CreateL(KAVCFrameMaxLength * 2);
	
	BaseConstructL();
	
	aPlayerCapabilitiesObserver = this;
	aPlayerApplicationSettingsObserver = this;
	aPlayerEventsObserver = this;
	
	// default values for event notifiers
	iPlayBackStatus = EStopped;
	
	// If no track currently selected
	iTrackId = KNoTrackSelected;
	
	iLengthInMilliseconds = 0;
	iTrackPosition = EStart;
	
	// If no track currently selected, then return KPlaybackPositionUnknown
	iPlaybackPositionInMilliseconds = KPlaybackPositionUnknown;
	iLastNotifiedPlaybackPositionInMilliseconds	= KPlaybackPositionUnknown;

	// The time interval used for playback position notification
	iPlayBackIntervalInMilliseconds = 0;

	// Try to detect battery status from hardware P&S properties	
	iBatteryStatus = DetectBatteryStatus();

	// The Bluetooth SIG vendor id must always be present
	// See AVRCP 1.3 Specification, section 5.5.1, table 5.4
	iCompanyIdList.AppendL(KBluetoothSIGVendorId);
 
	// AVRCP 1.3 specification, Appendix H says that PlaybackStatusChanged
	// and TrackChanged event notification support is mandatory, so add these
	iSupportedNotificationEventList = new (ELeave) TEventsMask();
	iSupportedNotificationEventList->Append(ERegisterNotificationPlaybackStatusChanged);
	iSupportedNotificationEventList->Append(ERegisterNotificationTrackChanged);
	}

EXPORT_C CPlayerInfoTarget::~CPlayerInfoTarget()
	{
	iPendingNotificationEventList.Close();
	iCompanyIdList.Close();
	iOutBuf.Close();

	// Free up all the settings memory
	THashMapIter<TInt, CPlayerApplicationSettings*> iter(iPlayerApplicationSettings);
	CPlayerApplicationSettings* const* setting = iter.NextValue();
	while ( setting != NULL )
		{
		delete *setting;
		setting = iter.NextValue();
		}

	// Now close the hash table itself
	iPlayerApplicationSettings.Close();
	delete iSupportedNotificationEventList;
	}
	
		
TAny* CPlayerInfoTarget::GetInterfaceIf(TUid aUid)
	{
	TAny* ret = NULL;
	if ( aUid == TUid::Uid(KRemConInterfaceIf2) )
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConInterfaceIf2*>(this)
			);
		}

	return ret;	
	}

// from MRemConInterfaceIf
void CPlayerInfoTarget::MrcibNewMessage(TUint aOperationId, const TDesC8& aData, TRemConMessageSubType aMsgSubType)
	{
	LOG(_L("CPlayerInfoTarget::MrcibNewMessage"));
	LOG1(_L("\taOperationId = 0x%02x"), aOperationId);
	LOG1(_L("\taData.Length = %d"), aData.Length());
	LOG1(_L("\taMsgSubType = 0x%02x"), aMsgSubType);

	TMetadataTransferPDU currentOp = RAvrcpIPC::GetPDUIdFromIPCOperationId(aOperationId);
	
	switch (currentOp)
		{
		case EGetCapabilities:
			{
			/* process this in playercapabilities */
			ProcessGetCapabilities( aData );
			break;
			}
		case EListPlayerApplicationSettingAttributes:	
			{
			/* respond with Player Application Attributes*/
			ProcessListPlayerApplicationAttributes( aOperationId );
			break;
			}
		case EGetPlayerApplicationSettingAttributeText:	
			{
			/* respond with Player Application Attribute Text*/
			ProcessGetPlayerApplicationAttributeText( aData, aOperationId );
			break;
			}
		case EListPlayerApplicationSettingValues:
			{
			/* respond with Player Application Values*/
			ProcessListPlayerApplicationValues( aData, aOperationId );
			break;
			}
		case EGetPlayerApplicationSettingValueText:
			{
			/* respond with Application Value Text*/
			ProcessGetPlayerApplicationValueText( aData, aOperationId );
			break;
			}
		case EGetCurrentPlayerApplicationSettingValue:
			{
			/* respond with Current Player Application Value*/
			ProcessGetCurrentPlayerApplicationValue( aData, aOperationId );
			break;
			}
		case ESetPlayerApplicationSettingValue:
			{
			/* Set Player Application Value*/
			ProcessSetPlayerApplicationValue( aData, aOperationId );
			break;
			}
		case EGetPlayStatus:
			{
			// process in playerevents
			ProcessGetPlayStatus();
			break;
			}
		case ERegisterNotification:
			{
			// obtain eventId from aOperationId
			TRegisterNotificationEvent eventId = RAvrcpIPC::GetEventIdFromIPCOperationId(aOperationId);
			
			// register for Notifications
			if (aMsgSubType == ERemConNotifyCommandAwaitingInterim)
				{
				ProcessGetStatusAndBeginObserving(aOperationId, eventId, aData);
				}
			else if (aMsgSubType == ERemConNotifyCommandAwaitingChanged)
				{
				ProcessGetStatus(aOperationId, eventId);
				}
		 	break;
		 	}
		case EGetPlayStatusUpdate:
			{
			// process in playerevents
			ProcessGetPlayStatusUpdate(aData);
			break;
			}
		default:
			break;
		}
		
	}

MPlayerEventsObserver::TTargetBatteryStatus CPlayerInfoTarget::DetectBatteryStatus()
	{
	MPlayerEventsObserver::TTargetBatteryStatus status = ECritical;
	TInt err = KErrNone;
	TInt batteryStatus = EBatteryStatusUnknown;
	TInt chargingStatus = EChargingStatusNotConnected;
	err = RProperty::Get(KPSUidHWRMPowerState,KHWRMBatteryStatus,batteryStatus);
	if (!err)
		{
		err = RProperty::Get(KPSUidHWRMPowerState,KHWRMChargingStatus,chargingStatus);
		}

	switch (batteryStatus)
		{
		// Normal or Charging
		case EBatteryStatusOk:
			{
			if (chargingStatus == EChargingStatusCharging ||
				chargingStatus == EChargingStatusAlmostComplete ||
				chargingStatus == EChargingStatusChargingContinued)
				{
				iBatteryStatus = EExternal;
				}
			else
				{
				if (chargingStatus == EChargingStatusChargingComplete)
					{
					iBatteryStatus = EFullCharge;
					}
				else
					{
					iBatteryStatus = ENormal;
					}
				}
			}
			break;
		// Warning
		case EBatteryStatusLow:
			{
			iBatteryStatus = EWarning;
			}
			break;
		// Critical
		case EBatteryStatusEmpty:
			{
			iBatteryStatus = ECritical;
			}
			break;
		// Unknown state, set as ECritical
		case EBatteryStatusUnknown:
			{
			iBatteryStatus = ECritical;
			}
			break;
		}
	return status;
	}
