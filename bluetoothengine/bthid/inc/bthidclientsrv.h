/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Client-Server message passing declarations
 *
*/


#ifndef CLIENTSRV_H
#define CLIENTSRV_H

// Opcodes used in message passing between client and server
enum TBTHidIpc
    {
    EBTHIDServConnectDevice, /*!< Connect New device command */
    EBTHIDServCancelConnect, /*!< Cancel connection attempt */
    EBTHIDServDisconnectDevice, /*!< Disconnect device command */
    EBTHidSrvDisconnectAllGracefully,/*!< Disconnect all device command */
    EBTHIDServDropConnection, /*!< Close Bluetooth Connection */
    EBTHIDServNotifyConnectionChange,/*!< Notify connection status change */
    EBTHIDServCancelNotify, /*!< Cancel outstanding notify command */
    EBTHIDServConnectionCount, /*!< Get connection count command */
    EBTHIDServConnectionStatus, /*!< Get connection details command */
    EBTHIDServFindConnection, /*!< Get connection details command */
    EBTHIDServAuthoriseFlag, /*!< Get Authorisation flag command */
    EBTHIDServSetAuthoriseFlag, /*!< Set Authorisation flag command */
    EBTHIDServIsAlreadyPairedFlag, /*!< Get pairing status for given address */
    EBTHIDServIsTrusted, /*!< Get trust status for given address */
    EBTHIDServIsConnected, /*!< Get Connected status for given address */
    EBTHIDServConnectionDetails, /*!< Get connection details */
    EBTHIDGetConnections, /*!< Get HID connections addresses */
    EInvalidIpc
    };

// server name
_LIT(KBTHidSrvName,"bthidserver");
_LIT(KBTHidServerFilename, "bthidserver.exe");

// UID of the server
const TUid KUidBTHidServer =
    {
    0x2001E301
    };

//the server version. A version must be specified when 
//creating a session with the server
const TUint KBTHIDServMajorVersionNumber = 0;
const TUint KBTHIDServMinorVersionNumber = 1;
const TUint KBTHIDServBuildVersionNumber = 1;

typedef TPckgBuf<TInt> TBTHidParamPkg;

#endif // _BTACCCLIENTSRV_H
