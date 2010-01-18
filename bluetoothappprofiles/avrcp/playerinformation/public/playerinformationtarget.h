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

#ifndef REMCONPLAYERINFORMATIONTARGET_H
#define REMCONPLAYERINFORMATIONTARGET_H

#include <e32base.h>
#include <s32mem.h>
#include <e32hashtab.h>

#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>
#include <remcon/avrcpspec.h>
#include <playerinformationtargetobserver.h>

class CRemConInterfaceSelector;
class CPlayerApplicationSettings;
class TEventsMask;

// This is the maximum length of the player application setting attribute and value strings
const TInt KMaxPlayerApplicationSettingsValue = 255 ;
// This is the minimum length of the player application setting attribute and value strings
const TInt KMinPlayerApplicationSettingsValue = 1 ;

/**
Client-instantiable class supporting player information responses.
This is the container class for the three interface parts
*/
NONSHARABLE_CLASS(CPlayerInfoTarget) : 	public CRemConInterfaceBase,
								       	public MRemConInterfaceIf2, 
								       	private MPlayerCapabilitiesObserver,
										private MPlayerApplicationSettingsObserver,
										private MPlayerEventsObserver
								           
	{
public:
	/**
	Factory function.
	@param aInterfaceSelector The interface selector. The client must have 
	created one of these first.
	@param aPlayerCapabilitiesObserver Returns a pointer to a MPlayerCapabilitiesObserver for this interface, owned by the CPlayerInfoTarget.
	@param aPlayerApplicationSettingsObserver Returns a pointer to a MPlayerApplicationSettingsObserver for this interface, owned by the CPlayerInfoTarget
	@param aPlayerEventsObserver Returns a pointer to a MPlayerEventsObserver for this interface, owned by the CPlayerInfoTarget
	@return A new CPlayerInfoTarget, owned by the interface selector.
	*/
	IMPORT_C static CPlayerInfoTarget* NewL(CRemConInterfaceSelector& aInterfaceSelector,
								       	MPlayerCapabilitiesObserver*& aPlayerCapabilitiesObserver,
										MPlayerApplicationSettingsObserver*& aPlayerApplicationSettingsObserver,
										MPlayerEventsObserver*& aPlayerEventsObserver,
										MPlayerApplicationSettingsNotify& aApplicationSettingNotifier );
	
	/** Destructor */
	IMPORT_C ~CPlayerInfoTarget();



private:
	/** 
	Constructor.
	@param aInterfaceSelector The interface selector.
	*/
	CPlayerInfoTarget(CRemConInterfaceSelector& aInterfaceSelector, MPlayerApplicationSettingsNotify& aApplicationSettingNotifier );

	void ConstructL(MPlayerCapabilitiesObserver*& aPlayerCapabilitiesObserver,
							MPlayerApplicationSettingsObserver*& aPlayerApplicationSettingsObserver,
							MPlayerEventsObserver*& aPlayerEventsObserver );
	
private: // from CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);

private: // from MRemConInterfaceIf
	void MrcibNewMessage(TUint aOperationId, const TDesC8& aData, TRemConMessageSubType aMsgSubType);

private: 
	// Bearer attributes (settings) processing
	void ProcessListPlayerApplicationAttributes( TInt aOperationId );
	void ProcessGetPlayerApplicationAttributeText( const TDesC8& aData, TInt aOperationId );
	void ProcessListPlayerApplicationValues( const TDesC8& aData, TInt aOperationId );
	void ProcessGetPlayerApplicationValueText( const TDesC8& aData, TInt aOperationId );
	void ProcessGetCurrentPlayerApplicationValue( const TDesC8& aData, TInt aOperationId );
	void ProcessSetPlayerApplicationValue( const TDesC8& aData, TInt aOperationId );

	// Bearer attributes (settings) API implementation from MPlayerApplicationSettingsObserver
	void DoDefineAttributeL(TUint aAttributeID, TDesC8& aAttributeText, RArray<TUint> &aValues, RArray<TPtrC8> &aValueTexts, TUint aInitialValue);
	void DoSetAttributeL(TUint aAttributeID, TUint aValue);

	// Notifier Events API implementation from MPlayerEventsObserver
	void DoPlaybackStatusChanged(TPlaybackStatus aStatus);
	void DoTrackChanged(TUint64 aTrackId, TUint32 aLengthInMilliseconds);
	void DoTrackReachedEnd();
	void DoTrackReachedStart();
	void DoSetPlaybackPosition(TUint32 aMilliseconds);	
	void DoSetBatteryStatus(TTargetBatteryStatus aBatteryStatus);

	// Notifier Events bearer processing  
	void ProcessGetPlayStatus();
	void ProcessGetStatusAndBeginObserving(TUint aOperationId, TRegisterNotificationEvent aEventId, const TDesC8& aData);
	void ProcessGetStatus(TUint aOperationId, TRegisterNotificationEvent aEventId);
	void SendNotificationResponse(TRegisterNotificationEvent aEventId, TRemConMessageSubType aMsgSubType);
	void ProcessGetPlayStatusUpdate(const TDesC8& aData);
	
	// Capabilities API implementation from MPlayerCapabilitiesObserver
	void DoClearEvents();
	TInt DoAddEvent(TRegisterNotificationEvent aEvent);
	TInt DoRemoveEvent(TRegisterNotificationEvent aEvent);
	void DoClearCompanyIds();
	TInt DoAddCompanyId(TInt aID);
	TInt DoRemoveCompanyID(TInt aID);

	// Capabilities bearer processing 
	void ProcessGetCapabilities( const TDesC8& aData );
	
	// Miscellaneous
	TBool AttributeSettingExists(TUint anAttribute);
	TBool IsValidAttributeValue(TUint anAttributeId, TUint aAttributeValue);
	TBool AttributeValueCanBeSet(TUint anAttributeId, TUint aAttributeValue);
	void SendError(TInt, TInt);
	void SendError(TInt, TInt, TRemConMessageSubType aSubType);
	CPlayerApplicationSettings* GetSetting(TUint anAttribute);
	MPlayerEventsObserver::TTargetBatteryStatus DetectBatteryStatus();

private: // owned
	RBuf8								iOutBuf;

	MPlayerApplicationSettingsNotify& 	iApplicationSettingNotifier;
		
	// data for MPlayerEventsObserver
	TPlaybackStatus 					iPlayBackStatus;
	TUint64 							iTrackId;
	TUint32 							iLengthInMilliseconds;
	TPlayPosition						iTrackPosition;
	
	// The time interval (received as part if the RegisterNotification) 
	// in which the change in playback position will be notified
	TUint32   							iPlayBackIntervalInMilliseconds;

	TTargetBatteryStatus				iBatteryStatus;
	TUint32 							iPlaybackPositionInMilliseconds;
	TUint32								iLastNotifiedPlaybackPositionInMilliseconds;
	
	// data for MPlayerCapabilitiesObserver
	TEventsMask*						iSupportedNotificationEventList;
	RArray<TInt> 						iCompanyIdList;

	// list of events for which notifications have been request 
	RArray<TRegisterNotificationEvent> 			iPendingNotificationEventList;
	
	// Whether we've been asked for an update when the play status changes
	TBool								iPlaybackStatusUpdatePending;

	// Attribute (settings) data MPlayerApplicationSettingsObserver
	RHashMap<TInt, CPlayerApplicationSettings*>		iPlayerApplicationSettings;

	};


#endif // REMCONPLAYERINFORMATIONTARGET_H
