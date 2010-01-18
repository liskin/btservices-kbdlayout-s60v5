/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine domain publish & subscribe key definitions.
*
*/


#ifndef BTENG_DOMAIN_PS_KEYS_H
#define BTENG_DOMAIN_PS_KEYS_H

const TUint32 KBTBluetoothTestingMode = 0x101FFE48;

/**  Bluetooth Testing mode publish and subscribe UID */
const TUid KPSUidBluetoothTestingMode = {KBTBluetoothTestingMode};

/**  Bluetooth DUT mode - deprecated and replaced by Bluetooth Testing mode */
const TUid KPSUidBluetoothDutMode = {KBTBluetoothTestingMode};


/**
 * Publish and Subscribe key defining the Bluetooth
 * Device Under Test mode.
 *
 * Possible integer values:
 * 0 Device not in test mode
 * 1 Device in test mode
 *
 * Default value: 0
 */
const TUint KBTDutEnabled = 0x01;


/**  Enumeration for Bluetooth Device Under Test mode */
enum TBTDutModeValue
    {
    EBTDutOff,
    EBTDutOn
    };


/**
 * Publish and Subscribe key defining the Bluetooth
 * Secure Simple Pairing debug mode.
 *
 * Possible integer values:
 * 0 Device not in debug mode
 * 1 Device in debug mode
 *
 * Default value: 0
 */
const TUint KBTSspDebugmode = 0x02;


/**  Enumeration for Bluetooth Device Under Test mode */
enum TBTSspDebugModeValue
    {
    EBTSspDebugModeOff,
    EBTSspDebugModeOn
    };


#endif // BTENG_DOMAIN_PS_KEYS_H
