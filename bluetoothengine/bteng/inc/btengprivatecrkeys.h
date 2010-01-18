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
* Description:  Bluetooth Engine private central repository key definitions.
*
*/


#ifndef BTENG_PRIVATE_CR_KEYS_H
#define BTENG_PRIVATE_CR_KEYS_H


#include <btengdomaincrkeys.h>


/**  Bluetooth Discoverability Settings CenRep UID */
const TUid KCRUidBTEngPrivateSettings = { 0x10204DAC };


/**
 * CenRep key for storing Bluetooth visibility settings.
 * Stores the Bluetooth visibility mode.
 *
 * Possible integer values (the value are as published 
 * in the Bluetooth Core Specification):
 * ( 0x00 No scanning (no scans enabled)
 * 0x01 Inquiry scan only (inquiry Scan enabled, page scan disabled) )
 * 0x02 Hidden mode (page scan enabled, inquiry scan disabled)
 * 0x03 General discoverability mode (Page enabled, inquiry scan enabled)
 * 0x100 Temporarily visibile (first value after reserved range)
 *
 * Default value: 3
 *
 */
const TUint32 KBTDiscoverable =	0x00000001;


/**
 * CenRep key for storing Bluetooth local device name settings.
 * Stores the local name status (if the user has changed the 
 * local device name).
 *
 * Possible integer values:
 * 0 User has not changed local device name
 * 1 User has set the local deive name
 *
 * Default value: 0
 */
const TUint32 KBTLocalNameChanged = 0x00000002;


/**  Enumeration for local name changed status */
enum TBTLocalNameStatus
    {
    EBTLocalNameDefault = 0,
    EBTLocalNameSet
    };


/**
 * CenRep key for storing Bluetooth SIM Access Profile settings.
 * Stores the Bluetooth SIM Access Profile state.
 *
 * Possible integer values:
 * 0 SAP disabled
 * 1 SAP enabled
 *
 * Default value: 0
 */
const TUint32 KBTSapEnabled	= 0x00000003;

const TUint32 KBtHidKeyboardLayout = 0x00000004;

/**  Enumeration for Bluetooth SAP setting */
enum TBTSapMode
    {
    EBTSapDisabled = 0,
    EBTSapEnabled
    };


#endif // BTENG_PRIVATE_CR_KEYS_H
