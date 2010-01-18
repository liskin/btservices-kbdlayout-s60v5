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
* Description:  Bluetooth Engine private publish & subscribe key definitions.
*
*/


#ifndef BTENG_PRIVATE_PS_KEYS_H
#define BTENG_PRIVATE_PS_KEYS_H


#include <btengdomainpskeys.h>
#include <bttypes.h>

/**  Bluetooth private category publish and subscribe UID */
const TUid KPSUidBluetoothEnginePrivateCategory = { 0x101FFE47 };


/**
 * Publish and Subscribe key for passing serialized BT AT Codec 
 * between HFP RemCon Bearer and Command Handler.
 *
 * Default value (in binary format): ""
 */
const TUint KBTATCodec = 0x01;


/**
 * Publish and Subscribe key for storing the last paired device 
 * that was denied to connect by the user.
 *
 * Default value (in string format): ""
 */
const TUint KBTBlockDevAddr = 0x02;


/**
* Publish and Subscribe key for storing info of last audio device
* that tried to connect. Used by BT notifier only
*
* The key will contain the following information in 8 bit descriptor
* A;B;C
* Where:
* A is 0/1 depending if the last audio connection attempt was denied(0) or accepted(1)
* B is the bt device adress as exacly 12 hex digits
* C is the time the attempt was accepted or denied as 64 bit integer.

* Example:
* 0;0015a00f42dd;63304481516638125
*/

const TUint KBTAuthorInfoPerDevice = 0x03;

/**
 * Publishes outgoing pairing information for purposes of:
 *     - Ensure a single ongoing outgoing pair request in Bluetooth Engine;
 *     - BTNotif decides if auto pair with default pin shall be used or not;
 */
const TUint KBTOutgoingPairing = 0x04;

enum TBTOutgoingPairMode
    {
    /**
     * not outgoing pairing operation
     */
    EBTOutgoingPairNone = 0,
    
    /**
     * outgoing automatic 0000 pin pairing with headset in progress 
     */
    EBTOutgoingHeadsetAutoPairing,
    
    /**
     * outgoing manual pairing with headset in progress 
     */
    EBTOutgoingHeadsetManualPairing,
    
    /**
     * outgoing pairing with non-headset device in progress 
     */
    EBTOutgoingNoneHeadsetPairing,
    };

class TBTOutgoingPairProperty
    {
public:
    TBTOutgoingPairMode iMode;
    TBTDevAddr iAddr;
    };

/*
* PS Key to indicate the status of bluetooth audio link. Value is of type TBTAudioLinkInfo
* where iState == open / closed
*		iProfile == mono (0) or Stereo (1)
*		iAddr == bluetooth device address
*/
const TUint KBTAudioLinkStatus = 0x05;

enum TBTAudioLinkState {
	EAudioLinkOpen = 0,
	EAudioLinkClosed = 1
};

class TBTAudioLinkInfo
    {
public:    
    TBTAudioLinkState iState;
    TUint iProfile; // mono 0, stereo 1
    TBTDevAddr iAddr;
    };

/**
 * Publish and Subscribe to save last connection attemmp time.
 *
 */
const TUint KBTConnectionTimeStamp = 0x06;

/**
 * Publish and Subscribe to store status for is query 
 * to turn bt off is showing or not.
 */
const TUint KBTTurnBTOffQueryOn = 0x07;

enum TBTBtQueryValue
    {
    EBTQueryOff,
    EBTQueryOn
    };
    
/**
 * P&S to publish ongoing notifier operations with BT devices.
 * 
 * Data type: binary. 
 * 
 * Set and get the data via class TBTNotifLockPublish
 * from btnotiflock.h.
 * 
 * This info is used to lock some operations in btui and btnotifer:
 * For a heaset which was paired from phone, connecting operation
 * shall wait until trusting device has been asked from user and 
 * user selection has been performed;
 * For a device which initiated pairing with phone, query of 
 * accepting connection request from this device shall wait until
 * pairing note has been shown and trusting device has 
 * been asked from user and user selection has been performed.
 */
const TUint KBTNotifierLocks = 0x08;

#endif // BTENG_PRIVATE_PS_KEYS_H
