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

#ifndef REMCONABSOLUTEVOLUMEAPITARGET_H
#define REMCONABSOLUTEVOLUMEAPITARGET_H

#include <remcon/remconinterfacebase.h>
#include <remcon/remconinterfaceif.h>

_LIT(KAbsoluteVolumeTargetPanicName, "AbsoluteVolumeTarget");
enum TAbsoluteVolumeTargetPanics
    {
    ETargetInvalidMaxVolume = 0,
    ETargetVolumeBeyondMaxVolume = 1,
    };

class MRemConAbsoluteVolumeTargetObserver;

/**
Client-instantiable type supporting sending absolute volume API responses for
Set Absolute Volume and Register Absolute Volume Change Notification. This API
should be used in preference to the extapi.
*/
NONSHARABLE_CLASS(CRemConAbsoluteVolumeTarget) : public CRemConInterfaceBase,
                                                 public MRemConInterfaceIf2
	{
public:
	IMPORT_C static CRemConAbsoluteVolumeTarget* NewL(
			CRemConInterfaceSelector& aInterfaceSelector, 
			MRemConAbsoluteVolumeTargetObserver& aObserver,
			TUint32 aVolume, 
			TUint32 aMaxVolume);
	
	IMPORT_C ~CRemConAbsoluteVolumeTarget();

public:
	IMPORT_C void SetAbsoluteVolumeResponse(TUint32 aVolume, 
			TInt aErr);
	
	IMPORT_C void AbsoluteVolumeChanged(TUint32 aVolume);
	
private:
    CRemConAbsoluteVolumeTarget(CRemConInterfaceSelector& aInterfaceSelector,
    		MRemConAbsoluteVolumeTargetObserver& aObserver,
    		TUint32 aVolume, 
    		TUint32 aMaxVolume);

	void ConstructL();

private: // From CRemConInterfaceBase
	TAny* GetInterfaceIf(TUid aUid);

private: // From MRemConInterfaceIf2
	void MrcibNewMessage(TUint aOperationId, 
			const TDesC8& aData, 
			TRemConMessageSubType aMsgSubType);

private: // Utility Funtions
	void SendError(TInt aError, TUint aOperationId);
	void SendNotificationResponse(TRemConMessageSubType aMsgSubType);
	void ProcessGetStatus();
	void ProcessGetStatusAndBeginObserving();
	void ProcessSetAbsoluteVolume(const TDesC8& aData);

private: // Unowned
    MRemConAbsoluteVolumeTargetObserver& iObserver;

private: // Owned
	// ETrue indicates a request for absolute volume change notification has 
	// been received.
	TBool                     iAbsoluteVolumeNotificationRequest;
	RBuf8                     iOutBuf;
	TUint32                   iClientVolume;
	TUint32                   iClientMaxVolume;
	};
#endif // ABSOLUTEVOLUMEAPITARGET_H
