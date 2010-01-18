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

#ifndef REMCONABSOLUTEVOLUMEAPICONTROLLER_H_
#define REMCONABSOLUTEVOLUMEAPICONTROLLER_H_

#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>
#include <absolutevolumeapi.h>

_LIT(KAbsoluteVolumeControllerPanicName, "AbsoluteVolumeController");
enum TAbsoluteVolumeControllerPanics
	{
	EControllerInvalidMaxVolume = 0,
	EControllerVolumeBeyondMaxVolume = 1,
	EAbsoluteVolumeNotificationAlreadyRegistered = 2,
	EMultipleSetAbsoluteVolumes = 3,
	EMultipleNotifies = 4,
	};

class MRemConAbsoluteVolumeControllerObserver;
class CAbsVolSender;

/**
@internalComponent
*/
class MAbsVolSenderObserver
	{
public:
	virtual void MavsoSendComplete(TInt aResult) = 0;
	};

/**
This class is used to set and observer the volume level on a remote device.

An active scheduler is required in order to use this class.
*/
NONSHARABLE_CLASS(CRemConAbsoluteVolumeController) 
: public CRemConInterfaceBase, public MRemConInterfaceIf2, public MAbsVolSenderObserver
	{
public:    
	IMPORT_C static CRemConAbsoluteVolumeController* NewL(
			CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConAbsoluteVolumeControllerObserver& aObserver,
			TUint32 aMaxVolume);
	
	IMPORT_C ~CRemConAbsoluteVolumeController();

public:    
	IMPORT_C void SetAbsoluteVolume(TRequestStatus& aStatus, 
			TUint32 aVolume, 
			TUint& aNumRemotes); 
	IMPORT_C void CancelSetAbsoluteVolume();
	
	IMPORT_C void RegisterAbsoluteVolumeNotification();    
	IMPORT_C void CancelAbsoluteVolumeNotification();
    
private:    
	CRemConAbsoluteVolumeController(
			CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConAbsoluteVolumeControllerObserver& aObserver,
			TUint32 aMaxVolume); 
    void ConstructL();

private: // Utility functions    
	void HandleNotify(const TDesC8& aData, 
			TRemConMessageSubType aMessageSubType);  
    
    void HandleSetAbsoluteVolumeResponse( const TDesC8& aData );
    static TInt AsyncErrorCallBack(TAny* aObserver);
    void KickOffSendIfNeeded();
    void SetAbsoluteVolumeSendComplete(TInt aResult);
    void RegisterNotifySendComplete();
    void SendSetAbsoluteVolume();
    void SendNotify();
    
    void VolumeUpdate(TUint32 aVolume, TUint32 aMaxVolume);

private: // From CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);
    
private: // From MRemConInterfaceIf2
	void MrcibNewMessage(TUint aOperationId, 
			const TDesC8& aData, 
			TRemConMessageSubType 
			aMessageSubType);
    
private: // From MAbsVolSenderObserver
		void MavsoSendComplete(TInt aResult);
	
private: // Unowned
	MRemConAbsoluteVolumeControllerObserver& iObserver;
	  
private: // Owned
	// For operation-specific data.
	TBuf8<KAbsoluteVolumeRequestDataSize> iNotifyData;
	TBuf8<KAbsoluteVolumeRequestDataSize> iSetData;
	
	// Records the current absolute volume value.
	TUint32 iCurrentVolume;  
	TUint32 iCurrentMaxVolume;
	
	// Used when sending SetAbsoluteVolume commands
	TUint32 iClientMaxVolume;
	
	CAbsVolSender* iAbsVolSender;
	
	TBool iNotificationRequested;

	// Only used for abs vol sends.
	TRequestStatus* iClientStatus;
	TUint* iClientNumRemotes;

	enum TRequestState
		{
	// There is no request outstanding
	ENotInUse = 0,
	// We're currently sending this request.
	ESending,
	// This request is waiting to be sent (possibly because we were busy sending a previous request when this one arose).
	EPending,
		};
	// The two requests we support.
	TRequestState iAbsVolRequest;
	TRequestState iNotifyRequest;
	};

#endif /*REMCONABSOLUTEVOLUMEAPICONTROLLER_H_*/
