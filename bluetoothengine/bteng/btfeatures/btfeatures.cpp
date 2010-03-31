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

#include <centralrepository.h> 

#include "btengprivatecrkeys.h"


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
	
	// get ready to use the central repository.
	CRepository* cenRep = CRepository::NewL(KCRUidBTEngPrivateSettings);
	CleanupStack::PushL(cenRep);
	
	switch(aMode)
		{
	case EDisabled:
		User::LeaveIfError(cenRep->Set(KBluetoothEnterpriseState, EBluetoothEnterpriseDisabled));
		break;
	case EDataProfilesDisabled:
		User::LeaveIfError(cenRep->Set(KBluetoothEnterpriseState, EBluetoothEnterpriseDataProfilesDisabled));
		break;
	case EEnabled:
		User::LeaveIfError(cenRep->Set(KBluetoothEnterpriseState, EBluetoothEnterpriseEnabled));
		break;
	default:
		User::Leave(KErrArgument);
		break;
		}
	
	CleanupStack::PopAndDestroy(cenRep);
	}

EXPORT_C BluetoothFeatures::TEnterpriseEnablementMode BluetoothFeatures::EnterpriseEnablementL()
	{
	TEnterpriseEnablementMode mode = EEnabled;
	TInt setting = EBluetoothEnterpriseEnabled;
	
	// get the value out of the repository.
	CRepository* cenRep = CRepository::NewL(KCRUidBTEngPrivateSettings);
	CleanupStack::PushL(cenRep);
	
	TInt err = cenRep->Get(KBluetoothEnterpriseState, setting);
	if(err == KErrNotFound)
		{
		// not found in repository, so assume enabled by default
		mode = EEnabled;
		}
	else if(err == KErrNone)
		{
		// Got a value from repository, convert it as appropriate.
		switch(setting)
			{
		case EBluetoothEnterpriseDisabled:
			mode = EDisabled;
			break;
		case EBluetoothEnterpriseDataProfilesDisabled:
			mode = EDataProfilesDisabled;
			break;
		case EBluetoothEnterpriseEnabled:
			mode = EEnabled;
			break;
		default:
			User::Leave(KErrUnknown);
			break;
			}
		}
	else
		{
		// Some other error accessing the central repository.
		User::Leave(err);
		}
	
	CleanupStack::PopAndDestroy(cenRep);
	
	return mode;
	}
