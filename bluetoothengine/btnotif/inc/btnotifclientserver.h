/*
* ============================================================================
*  Name        : btnotifclientserver.h
*  Part of     : bluetoothengine / btnotif
*  Description : Definitions for the btnotif client-server interface.
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

#ifndef BTNOTIFCLIENTSERVER_H
#define BTNOTIFCLIENTSERVER_H

/**  BTNotif server name */
_LIT( KBTNotifServerName, "btnotifsrv" );

/**  btnotifsrv Uid3 for creating the server process */
const TUid KBTNotifServerUid3 = { 0x20026FED };

/**  
 *   BTNotif server version numbers; 
 *   Major version number is 2 to distinguish 
 *   from old btnotif architecture.
 */
const TInt KBTNotifServerVersionMajor = 2;
const TInt KBTNotifServerVersionMinor = 0;
const TInt KBTNotifServerVersionBuild = 0;

/**  
 *   Opcodes used in the client-server interface 
 *   for identifying the requested command.
 */
enum TBTNotifServerRequest
    {
    EBTNotifMinValue = 9,
    EBTNotifStartSyncNotifier,
    EBTNotifStartAsyncNotifier,
    EBTNotifCancelNotifier,
    EBTNotifUpdateNotifier,
    EBTNotifPrepareDiscovery = 43,
    EBTNotifPairDevice,
    EBTNotifCancelPairDevice,
    };

/**  
 *   Message slots Opcodes used in the client-server interface 
 *   for identifying the requested command.
 */
enum TBTNotifServerRequestSlot
    {
    EBTNotifSrvUidSlot,
    EBTNotifSrvParamSlot,
    EBTNotifSrvReplySlot
    };

#endif // T_BTNOTIFCLIENTSERVER_H
