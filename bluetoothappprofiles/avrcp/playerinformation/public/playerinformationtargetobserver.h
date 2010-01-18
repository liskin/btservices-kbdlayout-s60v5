// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef REMCONPLAYERINFORMATIONTARGETOBSERVER_H
#define REMCONPLAYERINFORMATIONTARGETOBSERVER_H

#include <e32base.h>
#include <remcon/avrcpspec.h>
#include <barsc2.h>


/**
This class is used to define the capabilities of the current application,
and the implementation will generate AVRCP responses to the controller.
For the event part of the API, it is recommended to add all events which make
sense for the media player. In general, this will be everything except
ESystemStatusChanged. The company ID part of the API is only needed if
the media player needs to support vendor extensions other than bluetooth SIG.
The bluetooth SIG company ID is always included in responses to a COMPANY_ID request.
*/
class MPlayerCapabilitiesObserver 
	{
	public:
		/**
		Remove all notification events from the supported list
		*/
		IMPORT_C void ClearEvents();
		/**
		Add a notification event to the supported list of events
		The AVRCP 1.3 specification mandates that PlaybackStatusChanged
		and TrackChanged events must be supported; KErrAlreadyExists will
		be returned if AddEvent() is called with either of these events.
		@param aEvent the event to add
		@return KErrAlreadyExists if the event is already present.
		KErrNotSupported if the event isn't supported by the implementation, e.g.. ESystemStatusChanged
		*/
		IMPORT_C TInt AddEvent(TRegisterNotificationEvent aEvent);
		/**
		Remove a notification event from the supported list of events
		The AVRCP 1.3 specification mandates that PlaybackStatusChanged
		and TrackChanged events must be supported; KErrNotSupported will
		be returned if RemoveEvent() is called with either of these events.
		@param aEvent the event to remove
		@return KErrNone if this completes successfully, KErrNotFound if aID
		was not in the list, or any other system wide error code.
		*/
		IMPORT_C TInt RemoveEvent(TRegisterNotificationEvent aEvent);

		const static TInt KMaxCompanyID = 0xFFFFFF;
		const static TInt KMaxNrOfCompanyIDs = 255;
		
		/**
		Remove all additional company IDs from the supported list
		*/
		IMPORT_C void ClearCompanyIds();
		/**
		Add a company id to the supported list of company ids.
		The AVRCP 1.3 specification mandates that the Bluetooth SIG vendor id 
		must be supported; KErrAlreadyExists will be returned if AddCompanyId()
		is called with this company id.
		@param aID the id to add
		@return KErrNone if this completes successfully,
		KErrAlreadyExists if aID is already present,
		KErrOverflow if the maximum number of company ids are already listed,
		or any other system wide error code.
		*/
		IMPORT_C TInt AddCompanyId(TInt aID);
		/**
		Remove a company id from the list of supported vendor extensions.
		The Bluetooth SIG id can't be removed, as this must always be supported
		@param aID the id to remove
		@return KErrNone if this completes successfully, KErrNotFound if aID
		was not in the list, or any other system wide error code.
		*/
		IMPORT_C TInt RemoveCompanyID(TInt aID);
		
	private:
		virtual void DoClearEvents() = 0;
		virtual TInt DoAddEvent(TRegisterNotificationEvent aEvent) = 0;
		virtual TInt DoRemoveEvent(TRegisterNotificationEvent aEvent) = 0;
		virtual void DoClearCompanyIds() = 0;
		virtual TInt DoAddCompanyId(TInt aID) = 0;
		virtual TInt DoRemoveCompanyID(TInt aID) = 0;
	};

/**
This class is for supporting the player application settings PDUs in AVRCP1.3
specification section 5.2. The RegisterNotification PDU for
EVENT_PLAYER_APPLICATION_SETTING_CHANGED is also supported through this API.

The media player should first define all the attributes it supports, using
DefineAttributeL. When an attribute's value is changed by the media player,
it should call SetAttributeL to inform the controller. When the controller
changes a setting, the media player application receives a callback via the
MPlayerApplicationSettingsNotify interface
*/
class MPlayerApplicationSettingsObserver 
	{
	public:
		/**
		Define an attribute supported by this player.
		It will be included in future responses to the following PDUs:
			ListPlayerApplicationSettingAttributes,
			ListPlayerApplicationSettingValues,
			GetCurrentPlayerApplicationSettingValue,
			GetPlayerApplicationSettingAttributeText,
			GetPlayerApplicationSettingValueText,
		@param aAttributeID The specification or player defined attribute
		@param aAttributeText The UTF8 text name of the attribute(allowed text length is 1-255) - the API will take a copy
		@param aValues The list of defined values
		@param aValueTexts The UTF8 text for each defined value(allowed text length is 1-255) - The API will make copies.
		@param aInitialValue The initial value for this attribute
		@leave KErrNoMemory if memory could not be allocated to store the copies of aAttributeID and relative settings
		@leave KErrNotSupported if attribute or value is out of specification defined range,
			or aValueTexts is not equal length to aValues
		*/
		IMPORT_C void DefineAttributeL(TUint aAttributeID,
										TDesC8& aAttributeText,
										RArray<TUint> &aValues,
										RArray<TPtrC8> &aValueTexts,
										TUint aInitialValue);

		/**
		Set the current value of a previously defined attribute
		This updates the cache and will cause completion of a 
		pending EVENT_PLAYER_APPLICATION_SETTING_CHANGED notification PDU
		@param aAttributeID The specification or player defined attribute
		@param aValue The current value
		@leave KErrNotFound if the attribute is not defined,
			KErrArgument if the value is not valid according to the definition
		*/
		IMPORT_C void SetAttributeL(TUint aAttributeID, TUint aValue);

	private:
		virtual void DoDefineAttributeL(TUint aAttributeID,
										TDesC8& aAttributeText,
										RArray<TUint> &aValues,
										RArray<TPtrC8> &aValueTexts,
										TUint aInitialValue) = 0;
		virtual void DoSetAttributeL(TUint aAttributeID, TUint aValue) = 0;
	
	};

/**
This is a helper API allowing CPlayerApplicationSettings to be initialised
via a resource file. Using a resource file may help to provide localised text
for the attributes and values, according to current language setting of the phone.
*/
class PlayerApplicationSettingsResourceInit
	{
	public:
	/**
	Defines multiple attributes supported by this player, which are listed in a resource file.
	@param aSettings The CPlayerApplicationSettings object on which the attributes should be defined
	@param aResource A fully constructed CResourceFile
	@leave KErrNoMemory, or leave from CResourceFile functions
	*/
	IMPORT_C static void DefineAttributesL(MPlayerApplicationSettingsObserver& aSettings, CResourceFile &aResource);
	};

/**
This class supports the notification PDUs in AVRCP1.3 specification section 5.4,
with the following exceptions:
   EVENT_SYSTEM_STATUS_CHANGED is not supported, it is only for adaptors that plug into a media player
   EVENT_PLAYER_APPLICATION_SETTING_CHANGED is supported through the CPlayerApplicationSettings API

Events are pushed by the media player calling functions in this API, where they are
cached until the controller pulls them via a GetPlayStatus or RegisterNotification PDU
@see CPlayerApplicationSettings
*/
class MPlayerEventsObserver 
	{
	public:
		enum TTargetBatteryStatus
			{
			ENormal     = 0,
			EWarning    = 1,
			ECritical   = 2,
			EExternal   = 3,
			EFullCharge = 4,
			EUnknown	= 5
			};	
			
		enum TPlaybackStatus
			{
			EStopped = 0,
			EPlaying = 1,
			EPaused = 2,
			EFwdSeek = 3,
			ERevSeek = 4,
			EError = 0xFF
			};

		enum TPlayPosition
			{
			EStart = 0,
			EMiddle = 1,
			EEnd= 2,
			};

		static const TUint64 KNoTrackSelected = KMaxTUint64;

		static const TUint32 KPlaybackPositionUnknown = 0xFFFFFFFF;
		
		/**
		Call this function whenever the playback status changes.
		It will be used to complete pending EVENT_PLAYBACK_STATUS_CHANGED
		and EVENT_PLAYBACK_POS_CHANGED notifications.
		The value is used to generate the response to a GetPlayStatus PDU.
		@param aStatus The specification defined playback status
		*/
		IMPORT_C void PlaybackStatusChanged(TPlaybackStatus aStatus);
		/**
		Call this function whenever the current media track is changed.
		use KNoTrackSelected to indicate that there is no media selected. 
		This is the default value on construction. It will be used to
		complete pending EVENT_TRACK_CHANGED and EVENT_PLAYBACK_POS_CHANGED notifications.
		The values are used to generate the response to a GetPlayStatus PDU.
		@param aTrackId A handle to the current track.
		@param aLengthInMilliseconds The length of the current track.
		*/
		IMPORT_C void TrackChanged(TUint64 aTrackId, TUint32 aLengthInMilliseconds);
		/**
		Call this function whenever the current track reaches the end position,
		e.g. due to playback or forward seek. It will be used to complete
		pending EVENT_TRACK_REACHED_END and EVENT_PLAYBACK_POS_CHANGED notifications.
		*/
		IMPORT_C void TrackReachedEnd();
		/**
		Call this function whenever the current track reaches the start position,
		e.g. due to reverse seek (rewind). It will be used to complete pending
		EVENT_TRACK_REACHED_START and EVENT_PLAYBACK_POS_CHANGED notifications.
		*/
		IMPORT_C void TrackReachedStart();
		/**
		Call this function during playback or seek operations, to indicate the
		current position within the track. It will be used to complete a pending
		EVENT_PLAYBACK_POS_CHANGED notification. The value is used to generate the
		response to a GetPlayStatus PDU.
		@param aMilliseconds The current playback position. It is recommended to call
		with a resolution <=1000ms to satisfy the 1s resolution of the notification
		playback interval.
		*/
		IMPORT_C void SetPlaybackPosition(TUint32 aMilliseconds);

		/**
		Call this function to report the current battery status
		@param aBatteryStatus The current battery status.
		*/
		IMPORT_C void SetBatteryStatus(TTargetBatteryStatus aBatteryStatus);
		
	private:
		virtual void DoPlaybackStatusChanged(TPlaybackStatus aStatus) = 0;
		virtual void DoTrackChanged(TUint64 aTrackId, TUint32 aLengthInMilliseconds) = 0;
		virtual void DoTrackReachedEnd() = 0;
		virtual void DoTrackReachedStart() = 0;
		virtual void DoSetPlaybackPosition(TUint32 aMilliseconds) = 0;
		virtual void DoSetBatteryStatus(TTargetBatteryStatus aBatteryStatus) = 0;
	
		
	};

/**
Clients must implement this interface if they require callbacks 
when the controller has changed the settings via a SetPASValue message
*/
class MPlayerApplicationSettingsNotify  
	{
	public:
		/**
		This is called when the controller has changed a setting
		@param aAttributeID A list of attribute IDs whose value has changed.
		@param aAttributeValue A list of new values for the attributes listed in aAttributeID.
		*/
		virtual void MpasnSetPlayerApplicationValueL(const RArray<TInt>& aAttributeID,
													const RArray<TInt>& aAttributeValue)=0;
	};
	

#endif // REMCONPLAYERINFORMATIONTARGETOBSERVER_H
