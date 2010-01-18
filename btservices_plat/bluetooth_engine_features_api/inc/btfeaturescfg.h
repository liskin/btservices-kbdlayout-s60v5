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


#ifndef BTFEATURES_H
#define BTFEATURES_H

#include <e32base.h>



/** Namespace supporting Bluetooth features. */
namespace BluetoothFeatures
	{
	/** Enumeration for the different Enterprise IT Bluetooth enablement types. */
	enum TEnterpriseEnablementMode
		{
		/** Bluetooth is disabled entirely. */
		EDisabled,
		/** Only certain profiles are allowed: HSP, HFP, AVRCP, A2DP, HID. */
		EDataProfilesDisabled,
		/* Bluetooth is enabled as normal, i.e. it is subject to usual user 
		control via the UI. */
		EEnabled
		};

	/** Obtains the current Enterprise IT enablement setting. Used by mw 
	software to control the creation of listening services and to police 
	outgoing services. 
	@return The current setting.
	*/
	IMPORT_C TEnterpriseEnablementMode EnterpriseEnablementL();
	
	/** Sets the current Enterprise IT enablement setting. 
	This function is provided only for use by the Bluetooth DCMO plugin. 
	No other software should use this to control Bluetooth availability.
	Changes only take effect on reboot.
	@param aEnablement The desired setting.
	*/
	IMPORT_C void SetEnterpriseEnablementL(TEnterpriseEnablementMode aEnablement);
	}

#endif  // BTFEATURES_H
