/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     BT Accessory Server security policy definition.
*
*/


#ifndef BTACC_SECURITYPOLICY_H
#define BTACC_SECURITYPOLICY_H

// ----------------------------------------------------------------------------------------
// BTAccSrv's policy
// ----------------------------------------------------------------------------------------
//

//Total number of ranges


//Definition of the ranges of IPC numbers
const TInt KBTAccSrvRanges[] = 
    {
    0,  //EBTAccSrvConnectToAccessory,
        //EBTAccSrvCancelConnectToAccessory,
        //EBTAccSrvDisconnectAccessory,
        //EBTAccSrvDisconnectAllGracefully,
        //EBTAccSrvIsConnected,
        //EBTAccSrvGetInfoOfConnectedAcc,
        //EBTAccSrvAudioToPhone,
        //EBTAccSrvAudioToAccessory,
        //EBTAccSrvCancelAudioToAccessory,
    EBTAccSrvInvalidIpc   // Range 3: Invalid IPC opcode, return ENotSupported
    };

const TUint KBTAccSrvRangeCount = sizeof(KBTAccSrvRanges) / sizeof(TInt);

//Policy to implement for each of the above ranges        
const TUint8 KBTAccSrvElementsIndex[KBTAccSrvRangeCount] = 
    {
    0,                              //policy 0 applies to 0th range
    CPolicyServer::ENotSupported    //applies to 7th range(out of range IPC)
    };

//Specific capability checks
const CPolicyServer::TPolicyElement KBTAccSrvElements[] = 
    {
        {_INIT_SECURITY_POLICY_C1(ECapabilityLocalServices),
         CPolicyServer::EFailClient
        }
    };

//Package all the above together into a policy
const CPolicyServer::TPolicy KBTAccSrvPolicy =
        {
        CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
        KBTAccSrvRangeCount,        //number of ranges                                   
        KBTAccSrvRanges,            //ranges array
        KBTAccSrvElementsIndex,        //elements<->ranges index
        KBTAccSrvElements            //array of elements
        };

#endif // BTACC_SECURITYPOLICY_H

// End of File
