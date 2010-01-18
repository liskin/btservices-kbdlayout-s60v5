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
#ifndef __BTSAP_DOMAIN_PS_KEYS_H__
#define __BTSAP_DOMAIN_PS_KEYS_H__

const TUid KPSUidBluetoothSapConnectionState = {0x10204DB1};
 
/**
* Information about the state of the BTSap connection
*
* Possible integer values:
* 0 (EBTSapNotConnected) Not connected
* 1 (EBTSapConnecting)	 Connection under negotiation
* 2 (EBTSapConnected)	 Connected
*
* Default value: 0 (EBTSapNotConnected)
*
* (Shared data key: KSapState)
*/
const TUint KBTSapConnectionState = 0x00000001;

// Enumeration that defines the different SAP connection states
enum TBTSapState
	{
	EBTSapNotConnected = 0,
	EBTSapConnecting   = 1,
	EBTSapConnected	   = 2,
	EBTSapAccepted     = 3,
	EBTSapRejected     = 4
	};

#endif // __BTSAP_DOMAIN_PS_KEYS_H__
