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
* Description:  Definitions for BTEng client-server interface.
*
*/



#ifndef BTENGCLIENTSERVER_H
#define BTENGCLIENTSERVER_H

#include <btdevice.h>
#include <bttypes.h>
#include "btengconstants.h"

/**  BTEngServer process name */
_LIT( KBTEngServerName, "btengsrv" );// Temp "bteng" );

/**  BTEng Uid3 for creating the server process */
const TUid KBTEngServerUid3 = { 0x10005950 };

/**  
 *   BTEngServer version numbers; 
 *   Major version number is 2 to distinguish from old BTServer architecture.
 */
const TInt KBTEngServerVersionMajor = 2;
const TInt KBTEngServerVersionMinor = 0;
const TInt KBTEngServerVersionBuild = 0;

/**  
 *   Opcodes used in the client-server interface 
 *   for identifying the requested function.
 */
enum TBTEngServerRequest
    {
    EBTEngSetPowerState= 0x21,          // 33
    EBTEngSetVisibilityMode,            // 34
    EBTEngConnectDevice,                // 35
    EBTEngCancelConnectDevice,          // 36
    EBTEngDisconnectDevice,             // 37
    EBTEngIsDeviceConnected,            // 38
    EBTEngIsDeviceConnectable,          // 39
    EBTEngGetConnectedAddresses,        // 40
    EBTEngNotifyConnectionEvents,       // 41
    EBTEngCancelEventNotifier,          // 42
    EBTEngPrepareDiscovery,             // 43
    EBTEngSetPairingObserver,           // 44
    EBTEngPairDevice,                   // 45
    EBTEngCancelPairDevice,             // 46
    };

/**  TBTDevAddr class size */
const TInt KTBTDevAddrSize = 8;

/**  Maximum conflicts array size for client-server interface. */
const TInt KBTEngMaxConflictsArraySize = 3 * KTBTDevAddrSize;

/**  
 *   Data structure for passing connection status events 
 *   from server to client.
 */
struct TBTEngEventMsg
    {
    TBTDevAddr iAddr;
    TBTEngConnectionStatus iConnEvent;
    TBuf8<KBTEngMaxConflictsArraySize> iConflictsBuf;
    };


/**  Package buffer for transferring parameters between client and server. */
typedef TPckgBuf<TInt> TBTEngParamPkg;

/**  Package buffer to pass the device class between client and server. */
typedef TPckgBuf<TUint32> TBTEngDevClassPkg;

/**  Package buffer for transferring the connection event message. */
typedef TPckgBuf<TBTEngEventMsg> TBTEngEventPkg;


#endif // BTENGCLIENTSERVER_H
