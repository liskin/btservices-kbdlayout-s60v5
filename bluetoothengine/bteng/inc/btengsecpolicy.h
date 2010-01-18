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
* Description:  Security policy for BTEng client-server interface.
*
*/



#ifndef BTENGSECPOLICY_H
#define BTENGSECPOLICY_H


/**  BTEng policy range count */
const TUint KBTEngRangeCount = 5;

/**  Ranges of BTEng opcodes */
const TInt KBTEngRanges[ KBTEngRangeCount ] = 
    {
    0,  // Range 0: 0 - 32 (Out of range)
    33, // Range 1: EBTEngSetPowerState
        //          EBTEngSetScanMode
    35, // Range 2: EBTEngConnectDevice
        //          EBTEngCancelConnectDevice
        //          EBTEngDisconnectDevice
        //          EBTEngIsDeviceConnected
        //          EBTEngIsDeviceConnectable
        //          EBTEngGetConnectedAddresses
        //          EBTEngNotifyConnectionEvents
        //          EBTEngCancelEventNotifier
        //          EBTEngPrepareDiscovery
    44, // Range 3: EBTEngSetPairingObserver
        //          EBTEngPairDevice
        //          EBTEngCancelPairDevice
    47  // Range x: Out of range
    };

/**  Mapping of ranges to policies */
const TUint8 KBTEngElementsIndex[ KBTEngRangeCount ] = 
    {
    CPolicyServer::ENotSupported,   //applies to 0th range (out of range)
    1,                              //policy 1 applies to 1st range
    0,                              //policy 0 applies to 2nd range
    1,                              //policy 1 applies to 3rd range
    CPolicyServer::ENotSupported    //applies to 9th range(out of range IPC)
    };

/**  BTEng security policies */
const CPolicyServer::TPolicyElement KBTEngPolicyElements[] = 
    {
        { _INIT_SECURITY_POLICY_C1( ECapabilityLocalServices ), 
          CPolicyServer::EFailClient
        }, //policy 0, connection management operations
        { _INIT_SECURITY_POLICY_C2( ECapabilityLocalServices, ECapabilityWriteDeviceData ), 
          CPolicyServer::EFailClient
        } //policy 1, write HW settings
    };

/**  BTEng security policy package */
const CPolicyServer::TPolicy KBTEngServerPolicy = 
    {
    CPolicyServer::EAlwaysPass, // Connection attempts may pass
    KBTEngRangeCount,           // Number of ranges
    KBTEngRanges,               // Array of ranges
    KBTEngElementsIndex,        // Mapping of ranges to policies
    KBTEngPolicyElements        // Array of policies
    };


#endif // BTENGSECPOLICY_H
