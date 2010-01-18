/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
*/


#include <btfeaturescfg.h> 
#include <featmgr/FeatureControl.h>
#include <featmgr/FeatMgr.h>

/**
	Device supports bluetooth data profiles disabled.
*/
const TInt KFeatureIdTempFfBluetoothDataProfilesDisabled = 0x1000001;
	
/**
	Device supports bluetooth disabled.
*/
const TInt KFeatureIdTempFfBluetoothDisabled = 0x1000002;


EXPORT_C void BluetoothFeatures::SetEnterpriseEnablementL(BluetoothFeatures::TEnterpriseEnablementMode aMode)
	{
	// This method is only provided for use by the Bluetooth DCMO plugin. 
	// Hence we should be running in the DCMO server process.
	_LIT_SECURE_ID(KDcmoServerSecureUid, 0x2001FE47);
	RProcess proc;
	if ( proc.SecureId() != KDcmoServerSecureUid )
		{
		User::Leave(KErrPermissionDenied);
		}	

	const TUid KBtDisabledUid = {KFeatureIdTempFfBluetoothDisabled};
	const TUid KBtDataProfilesDisabledUid = {KFeatureIdTempFfBluetoothDataProfilesDisabled};

	RFeatureControl featureControl;
	User::LeaveIfError(featureControl.Connect());
	CleanupClosePushL(featureControl);
	switch ( aMode )
		{
	case EDisabled:
		User::LeaveIfError(featureControl.EnableFeature(KBtDisabledUid));
		User::LeaveIfError(featureControl.DisableFeature(KBtDataProfilesDisabledUid));
		break;
	case EDataProfilesDisabled:
		User::LeaveIfError(featureControl.DisableFeature(KBtDisabledUid));
		User::LeaveIfError(featureControl.EnableFeature(KBtDataProfilesDisabledUid));
		break;
	case EEnabled:
		User::LeaveIfError(featureControl.DisableFeature(KBtDisabledUid));
		User::LeaveIfError(featureControl.DisableFeature(KBtDataProfilesDisabledUid));
		break;
	default:
		User::Leave(KErrArgument);
		break;
		}
	CleanupStack::PopAndDestroy(&featureControl);
	}

EXPORT_C BluetoothFeatures::TEnterpriseEnablementMode BluetoothFeatures::EnterpriseEnablementL()
	{
	TEnterpriseEnablementMode mode = EDisabled;
	
	FeatureManager::InitializeLibL();
    const TBool bluetoothDisabled = FeatureManager::FeatureSupported(KFeatureIdTempFfBluetoothDisabled);
    if ( !bluetoothDisabled )
		{
		const TBool dataProfilesDisabled = FeatureManager::FeatureSupported(KFeatureIdTempFfBluetoothDataProfilesDisabled);
		if ( dataProfilesDisabled )
			{
			mode = EDataProfilesDisabled;
			}
		else
			{
			mode = EEnabled;
			}	
		}
    FeatureManager::UnInitializeLib();
    
	return mode;
	}
