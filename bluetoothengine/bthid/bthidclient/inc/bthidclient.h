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
* Description:  BTHID Server client API
 *
*/


#ifndef BTHIDCLIENT_H
#define BTHIDCLIENT_H

#include <e32base.h>
#include <btdevice.h>
#include <btmanclient.h>
#include "bthidtypes.h"
#include "btengconstants.h"

/*!
 State changes of a Bluetooth HID connection
 */
enum TBTHidConnState
    {
    EBTDeviceConnected, /*!< Device connected */
    EBTDeviceConnectedFromRemote, /*!< Device connected from remote HID device */
    EBTDeviceLinkLost, /*!< Device link lost */
    EBTDeviceLinkRestored, /*!< Device link restored */
    EBTDeviceDisconnected, /*!< Device disconnected */
    EBTDeviceUnplugged, /*!< Device unplugged */
    EBTDeviceAnotherExist
    /*!< Another Device Connection already exist */
    };

/*!
 BT HID Device connection state details.
 */
class THIDStateUpdate
    {
public:
    /*! HID Device Bluetooth Address. */
    TBTDevAddr iDeviceAddress;

    /*! New Connection State. */
    TBTHidConnState iState;
    };
typedef TPckgBuf<THIDStateUpdate> THIDStateUpdateBuf;

/*!
 Provides the client-side interface to the server session.
 */

NONSHARABLE_CLASS( RBTHidClient ) : public RSessionBase
    {
public:
    /*!
     Connect to the server and create a session.
     @result System error code.
     */
    IMPORT_C TInt Connect();

    /*!
     Get the version number.
     @result The client version.
     */
    IMPORT_C TVersion Version() const;

    /*!
     Issue a connect request to the server.
     @param aParams The connection parameters.
     @param aDiagnostic Returned BT device address in case of conflict.
     @param aStatus A TRequestStatus object to be used for async comms.
     */
    IMPORT_C void ConnectDevice(const TBTDevAddrPckgBuf& aAddrBuf,
            TDes8& aDiagnostic, TRequestStatus& aStatus);

    /*!
     Issue an update request to the server.
     @param aUpdateParams Update buffer to be filled by the server.
     @param aDiagnostic Returned BT device address in case of conflict.
     @param aStatus A TRequestStatus object to be used for async comms.
     */
    IMPORT_C void NotifyStatusChange(THIDStateUpdateBuf& aUpdateParams,
            TDes8& aDiagnostic, TRequestStatus& aStatus);

    /*!
     Issue a disconnect request to the server.
     @param aAddress The BT address of the device to disconnect.
     @param aStatus A TRequestStatus object to be used for async comms.
     */
    IMPORT_C void DisconnectDevice(const TBTDevAddrPckgBuf& aAddrBuf,
            TBTDisconnectType aDiscType, TRequestStatus& aStatus);

    /*!
     Cancels an outstanding connect request with the server.
     */
    IMPORT_C void CancelConnectDevice() const;

    /*!
     Cancels an outstanding update request with the server.
     */
    IMPORT_C void CancelNotifyStatusChange() const;

    /*!
     Issue a disconnect request to the server.
     Release all the HID connections.
     @param aStatus A TRequestStatus object to be used for async comms.
     */
    IMPORT_C void DisconnectAllGracefully(TRequestStatus& aStatus);

    /*!
     Return HID device connection status in service level
     @param aBDaddr BT address of the device to retrieve.
     */
    IMPORT_C TInt IsConnected(const TBTDevAddr& aBDaddr);

    /*!
     Return HID device connection status in service level
     @param aAddrs Descriptor of BT addresses of connected devices
     @param aProfile Profile for connection  
     */
    IMPORT_C void GetConnections(TDes8& aAddrs, TInt aProfile);

    };

#endif  // BTHIDCLIENT_H
// End of File
