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
* Description:  Definitions of Bluetooth Engine constant.
*
*/


#ifndef BTENCONSTANTS_H
#define BTENCONSTANTS_H


/**  Connect status of the service-level connection. */
enum TBTEngConnectionStatus
    {
    EBTEngNotConnected,
    EBTEngConnecting,
    EBTEngConnected,
    EBTEngDisconnecting
    };


/**  Type of disconnect operation for the service-level connection. */
enum TBTDisconnectType
    {
    EBTDiscImmediate,
    EBTDiscGraceful
    };


/**  Enumeration for Bluetooth visibility mode */
enum TBTVisibilityMode
    {
    EBTVisibilityModeNoScans  = 0x00,    //will be used when turning off BT/power off phone
    EBTVisibilityModeInquiryScanOnly = 0x01,
    EBTVisibilityModeHidden    = 0x02,
    EBTVisibilityModeGeneral   = 0x03,
    EBTVisibilityModeTemporary = 0x100
    };


/**  Enumeration for Bluetooth profiles */
enum TBTProfile
    {
    EBTProfileUndefined = 0x0,
    EBTProfileDUN       = 0x1103,
    EBTProfileOPP       = 0x1105,
    EBTProfileFTP       = 0x1106,
    EBTProfileHSP       = 0x1108,
    EBTProfileA2DP      = 0x110D,
    EBTProfileAVRCP     = 0x110E,
    EBTProfileFAX       = 0x1111,
    EBTProfilePANU      = 0x1115,
    EBTProfileNAP       = 0x1116,
    EBTProfileGN        = 0x1117,
    EBTProfileBIP       = 0x111A,
    EBTProfileHFP       = 0x111E,
    EBTProfileBPP       = 0x1122,
    EBTProfileHID       = 0x1124,
    EBTProfileSAP       = 0x112D,
    EBTProfilePBAP      = 0x1130,
    EBTProfileDI        = 0x1200
    };

/**
 * Bits of UI cookie identifiers of a TBTNamelessDevice in BT registry
 */
enum TBTDeviceUiCookie
    {
    // invalid UI cookie
    EBTUiCookieUndefined = 0x0,
    
    // this device is paired with Just Works model under user's awareness.
    //
    // The purpose of this bit is for the paired device view of 
    // Bluetooth application to show devices that have been paired with
    // Just Works model under user's awareness.
    //
    // Pairing device from Bluetooth application is a user aware bonding.
    // Another example is connecting to HFP of a not-paired v2.1 headset via 
    // BluetoothEngine API during which pairing is performed due to the 
    // security setting of HFP AG.
    // Just-Works pairing due to service connection request from a v2.1 device without
    // IO capability is not considered as a user aware pairing.
    // 
    EBTUiCookieJustWorksPaired = 0x01
    };

#endif // BTENCONSTANTS_H
