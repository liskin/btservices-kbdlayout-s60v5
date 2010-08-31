/*
* ============================================================================
*  Name        : btnotifserversecpolicy.h
*  Part of     : bluetoothengine / btnotif       *** Info from the SWAD
*  Description : Security policy for btnotif client-server interface.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.2
*/

#ifndef BTNOTIFSRVSECPOLICY_H
#define BTNOTIFSRVSECPOLICY_H

/**  Number of ranges in btnotif security policy */
const TInt KBTNotifRangeCount = 4;

/**  Ranges of btnotif command opcodes */
const TInt KBTNotifRanges[ KBTNotifRangeCount ] = 
    {
    0,  // Range 0: 0 - EBTNotifMinValue (Out of range)
    10, // Range 1: All commands requiring no access control
    30, // Range 2: All commands requiring local services
        // More ranges may be added here
    50  // Range 3: Out of range
    };

/**  Mapping of ranges to policies */
const TUint8 KBTNotifElementsIndex[ KBTNotifRangeCount ] = 
    {
    CPolicyServer::ENotSupported,   // applies to 0th range (out of range)
    CPolicyServer::EAlwaysPass,     // no policy for the 1st range
    0,                              // policy 0 applies to 2nd range
    CPolicyServer::ENotSupported    // applies to xth range(out of range)
    };

/**  BTEng security policies, applied on a range of commands */
const CPolicyServer::TPolicyElement KBTNotifPolicyElements[] = 
    {
        { _INIT_SECURITY_POLICY_C1( ECapabilityLocalServices ), 
          CPolicyServer::EFailClient
        } //policy 0, for now on all operations
        // More policies may be added here
    };

/**  BTNotif security policy package (combining all the above) */
const CPolicyServer::TPolicy KBTNotifServerPolicy = 
    {
    CPolicyServer::EAlwaysPass, // Connection attempts may pass
    KBTNotifRangeCount,         // Number of ranges
    KBTNotifRanges,             // Array of ranges
    KBTNotifElementsIndex,      // Mapping of ranges to policies
    KBTNotifPolicyElements      // Array of policies
    };

#endif // BTNOTIFSRVSECPOLICY_H
