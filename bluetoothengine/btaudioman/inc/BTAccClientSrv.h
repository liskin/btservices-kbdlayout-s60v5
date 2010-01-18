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
*     BT Acc Server op-codes, panic enumerations and other definitions.
*
*/


#ifndef __BTACCCLIENTSRV_H
#define __BTACCCLIENTSRV_H

//  INCLUDES

// CONSTANTS

// Panic definitions
_LIT(KBTAudioManPanic,"BTAudioMan panic");

// reasons for server panic
enum TBTAccSrvPanic
    {
    EBTAccNoPluginsAvailable,
    EBTAccPluginImplementationNotAccessable, 
    EBTAccBadRequest
    };

// Opcodes used in message passing between client and server
enum TBTAccSrvServRequest
    {
    EBTAccSrvConnectToAccessory,
    EBTAccSrvCancelConnectToAccessory,
    EBTAccSrvDisconnectAccessory,
    EBTAccSrvGetConnections,
    EBTAccSrvDisconnectAllGracefully,
    EBTAccSrvNotifyConnectionStatus,
    EBTAccSrvCancelNotifyConnectionStatus,
    EBTAccSrvConnectionStatus,
    EBTAccSrvGetInfoOfConnectedAcc,
    EBTAccSrvAudioToPhone,
    EBTAccSrvAudioToAccessory,
    EBTAccSrvCancelAudioToAccessory,
    EBTAccSrvInvalidIpc,
    };

// server name
_LIT(KBTAudioManName,"btaudioman");

// A version must be specifyed when creating a session with the server
const TUint KBTAccServerMajorVersionNumber=1;
const TUint KBTAccServerMinorVersionNumber=0;
const TUint KBTAccServerBuildVersionNumber=1;

/**
* Utility to panic the client.
*/
void PanicClient(const RMessage2& aMessage, TInt aPanic);

/**
* Utility to panic server.
*/
void PanicServer(TInt aPanic);

#endif // _BTACCCLIENTSRV_H

