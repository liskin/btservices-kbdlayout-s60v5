/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine domain central repository key definitions.
*
*/


#ifndef BTENG_DOMAIN_CR_KEYS_H
#define BTENG_DOMAIN_CR_KEYS_H


#include <btserversdkcrkeys.h>


/**  Bluetooth Engine CenRep Uid */
const TUid KCRUidBluetoothEngine = { 0x10204DAB };


/**
 * CenRep key for storing Bluetooth feature settings.
 * Indicates if Bluetooth Headset Profile is supported or not.
 *
 * Possible integer values:
 * 0 Headset Profile not supported
 * 1 Headset Profile supported
 *
 * Default value: 1 
 */
const TUint32 KBTHspSupported = 0x00000001;


/**  Enumeration for Headset profile support */
enum TBTHspSupported
    {
    EBTHspNotSupported = 0,
    EBTHspSupported
    };


/**
 * CenRep key for storing Bluetooth feature settings.
 * Product specific settings for activating BT in offline mode.
 *
 * Possible integer values:
 * 0 BT activation disabled in offline mode
 * 1 BT activation enabled in offline mode
 *
 * Default value: 1
 */
const TUint32 KBTEnabledInOffline = 0x00000002;


/**  Enumeration for Bluetooth activation in offline mode */
enum TBTEnabledInOfflineMode
    {
    EBTDisabledInOfflineMode = 0,
    EBTEnabledInOfflineMode
    };


/**
 * CenRep key for storing Bluetooth feature settings.
 * Indicates if eSCO is supported.
 *
 * Possible integer values:
 * 0 eSCO not supported
 * 1 eSCO not supported
 *
 * Default value: 0
 */
const TUint32 KBTEScoSupportedLV = 0x00000003;


/**  Enumeration for eSCO support */
enum TBTEScoSupported
    {
    EBTEScoNotSupported = 0,
    EBTEScoSupported
    };


/**
 * CenRep key for storing Bluetooth feature settings.
 * Indicates if device selection/passkey setting by means 
 * other than user input is enabled.
 *
 * Possible integer values:
 * 0 Out-of-band setting is disabled
 * 1 Out-of-band setting is enabled
 *
 * Default value: 0
 */
const TUint32 KBTOutbandDeviceSelectionLV = 0x00000004;


/**  Enumeration for out-of-band selection mode */
enum TBTOutbandSelection
    {
    EBTOutbandDisabled = 0,
    EBTOutbandEnabled
    };


/**
 * CenRep key for storing Bluetooth feature settings.
 * Stores the Bluetooth Vendor ID.
 *
 * The integer value is specified by the Bluetooth SIG, and used for the 
 * Device Identification Profile. It needs to be pre-set by each product.
 */
const TUint32 KBTVendorID = 0x00000005;


/**
 * CenRep key for storing Bluetooth feature settings.
 * Stores the Bluetooth Product ID.
 *
 * The integer value is used for the Device Identification Profile. It is 
 * product-specific, and the values are managed by the company 
 * (as identified by the Vendor ID) It needs to be pre-set by each product.
 */
const TUint32 KBTProductID = 0x00000006;

/**
 * CenRep key for storing Bluetooth feature settings.
 * Indicates if supports remote volume control over AVRCP Controller.
 *
 * Possible integer values:
 * 0  not supported
 * 1  supported
 *
 * Default value: 1
 */
const TUint32 KBTAvrcpVolCTLV = 0x00000007;

/**  Enumeration for remote volume control AVRCP Controller support */
enum TBTAvrcpVolCTSupported
    {
    EBTAvrcpVolCTNotSupported = 0,
    EBTAvrcpVolCTSupported
    };
    
/**
 * CenRep key for storing Bluetooth feature settings.
 * Indicates if supports legacy remote volume control over AVRCP Controller.
 *
 * Possible integer values:
 * 0  not supported
 * 1  supported
 *
 * Default value: 0
 */
const TUint32 KBTAvrcpLegacyVolCTLV = 0x00000008;

/**  Enumeration for remote volume control AVRCP Controller support */
enum TBTAvrcpLegacyVolCTSupported
    {
    EBTAvrcpLegacyVolCTNotSupported = 0,
    EBTAvrcpLegacyVolCTSupported
    };
    
/**
 * CenRep key for default folder for file receiving.
 * Indicates where receving files will be saved.
 
 *
 * Default value: Received files\\
 */
const TUint32 KLCReceiveFolder = 0x0000000A;

/**
 * CenRep key for storing Bluetooth feature settings.
 * Indicates support of automatic disconnection when audio link opening fails
 *
 * Possible integer values:
 * 0 disabled;
 * 1 enabled.
 *
 * Default value: 1 in S60 3.2, and 0 since S60 5.0
 */
const TUint32 KBTDisconnectIfAudioOpenFailsLV = 0x00000010;

/**  Enumeration for automatic disconnection activation/deactivation */
enum TBTAutoDisconnectIfAudioOpenFails
    {
    EBTNotDisconnectIfAudioOpenFails = 0,
    EBTDisconnectIfAudioOpenFails = 1
    };

/**  Bluetooth Local Device Address CenRep UID */
const TUid KCRUidBluetoothLocalDeviceAddress = { 0x10204DAA };

/**
 * CenRep key for storing the Bluetooth local device address.
 *
 * Default value (in string format): ""
 */
const TUint32 KBTLocalDeviceAddress = 0x00000001;


#endif // BTENG_DOMAIN_CR_KEYS_H
