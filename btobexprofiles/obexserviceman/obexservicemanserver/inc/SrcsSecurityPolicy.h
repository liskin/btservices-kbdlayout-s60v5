/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Obexservicemanager security policy definition
*
*/


#ifndef SRCS_SECURITYPOLICY_H
#define SRCS_SECURITYPOLICY_H

// ---------------------------------------------------------------------------
// Srcs's policy
// ---------------------------------------------------------------------------
//

// Count of ranges
const TUint KSrcsRangeCount = 2;


// Definition of the ranges of IPC numbers
const TInt KSrcsRanges[KSrcsRangeCount] = 
    {
	0,  // ESrcsBTServicesON to ESrcsGetIrDAState
	7   // Out of IPC range
    };

// Policy to implement for each of the above ranges        
const TUint8 KSrcsElementsIndex[KSrcsRangeCount] = 
    {
    0,	                         // applies to 0th range                          
    CPolicyServer::ENotSupported // out of range Srcs's IPC
    };

//Specific capability checks
const CPolicyServer::TPolicyElement KSrcsElements[] = 
    {
		{
			_INIT_SECURITY_POLICY_C1(ECapabilityLocalServices), CPolicyServer::EFailClient
		} //policy 0.
    };

// Policy structure for Srcs. This is passed to Srcs's constructor
const CPolicyServer::TPolicy KSrcsPolicy =
        {
        CPolicyServer::EAlwaysPass, // Indicates that Connect attempts should be processed without any further checks.
        KSrcsRangeCount,	        // Range count
        KSrcsRanges,	            // ranges array
        KSrcsElementsIndex,	        // elements' index
        KSrcsElements		        // array of elements
        };

#endif // SRCS_SECURITYPOLICY_H

// End of File