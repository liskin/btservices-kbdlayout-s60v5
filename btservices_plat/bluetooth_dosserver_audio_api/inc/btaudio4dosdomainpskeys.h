/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth accessory connection state P&S key definition
*               This is only for non- Accessory Framework build since A3.2
*
*/


#ifndef BTAUDIO4DOS_DOMAINPSKEYS_H
#define BTAUDIO4DOS_DOMAINPSKEYS_H

/**
* Notice that the keys declared in this header are ONLY for Dos Server version build since A3.2 release.
* The keys are not available at runtime in Accessory Framework version.
*
* In Dos Server build, the keys are defined when Bluetooth power is ON and deleted when
* Bluetooth power is OFF.
*/

// PubSub Uid of BTAAC state
const TUid KPSUidBluetoothAudioAccessoryState = { 0x101FFE49 };

// Key1 : Bluetooth HW address of the connected BTAA (string value)
// No connected BT audio accessory if this key is not defined or the value is "",
// otherwise, it contains the readable BT address of audio accessory
// 
const TUint KBtaaAddress = 0x00000001;

// Key2 : Is a BTAA (BT Audio Accessory) connected and does it support remote volume control.
// No connected BT audio accessory if this key is not defined.
const TUint KBtaacConnected = 0x00000002;

// Possible integer values for key KBtaacConnected
enum TBtaaConnectionStatus
	{
	EBTaacNoAudioAccConnected,
	EBTaacRemoteVolCtrlCapableAccConnected,
	EBTaacRemoteVolCtrlIncapableAccConnected,
	EBTaacAccTemporarilyDisconnected,
	EBTaacAccTemporarilyUnavailable 		
	};

#endif

