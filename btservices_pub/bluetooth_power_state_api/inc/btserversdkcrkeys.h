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
* Description:  Bluetooth Engine SDK central repository key definitions.
*                The file name is kept as before for compatibility reasons.
*
*/


#ifndef BTSERVER_SDK_CR_KEYS_H
#define BTSERVER_SDK_CR_KEYS_H


/** 
* @file btserversdkcrkeys.h
* 
* The API provides the BT power state central repository key that is updated by
* the S60 BT Server. The key provides up-to-date information on the BT
* HW power state. Any application may monitor to the cenrep key that is
* updated to receive notifications of any change in state.
*
* The key UID is #KCRUidBluetoothPowerState and following key is provided:
* - #KBTPowerState
*/

/**  Bluetooth Power Mode UID */
const TUid KCRUidBluetoothPowerState = { 0x10204DA9 };

/**
 * The cenrep key for storing the Bluetooth power mode.
 *
 * Valid values are defined by the enum #TBTPowerStateValue
 *
 * Possible values are:
 * 0 Bluetooth chip is not powered/in sleep mode
 * 1 Bluetooth chip is powered up and fully functional
 *
 * Default value: 0
 *
 * @see TBTPowerStateValue
 */
const TUint32 KBTPowerState = 0x00000001;


/**  Enumeration for Bluetooth power mode values */
enum TBTPowerStateValue
    {
    EBTPowerOff,
    EBTPowerOn
    };
#endif // BTSERVER_SDK_CR_KEYS_H
