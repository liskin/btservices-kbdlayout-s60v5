/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT HID client implementation
 *
*/


#include <btmanclient.h>
#include "bthidclient.h"
#include "bthidclientsrv.h"
#include "debug.h"
#include "hiduids.h"

static TInt StartServer();

// ---------------------------------------------------------
// Connect
// Handles connection to server(creates session)
// ---------------------------------------------------------
//
EXPORT_C TInt RBTHidClient::Connect()
    {
        TRACE_FUNC

    TInt retVal = CreateSession(KBTHidSrvName, Version());
    if (retVal != KErrNone && retVal != KErrAlreadyExists)
        {
        retVal = StartServer();
        if (retVal == KErrNone || retVal == KErrAlreadyExists)
            {
            retVal = CreateSession(KBTHidSrvName, Version());
            }
        }
        TRACE_INFO((_L("[BTHID] ret %d "), retVal))

    if (retVal != KErrNone)
        {
        Close();
        }

    return retVal;
    }

// ---------------------------------------------------------
// Version
// Defines server version number
// ---------------------------------------------------------
//
TVersion RBTHidClient::Version() const
    {
    return (TVersion(KBTHIDServMajorVersionNumber,
            KBTHIDServMinorVersionNumber, KBTHIDServBuildVersionNumber));
    }

// ---------------------------------------------------------
// ConnectToHID
// Establishes a Bluetooth HID connection
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::ConnectDevice(const TBTDevAddrPckgBuf& aAddrBuf,
        TDes8& aDiagnostic, TRequestStatus& aStatus)
    {
    SendReceive(EBTHIDServConnectDevice, TIpcArgs(&aAddrBuf, &aDiagnostic),
            aStatus);
    }

// ---------------------------------------------------------
// CancelConnectDevice
// Cancel Connection attempt
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::CancelConnectDevice() const
    {
    SendReceive(EBTHIDServCancelConnect);
    }

// ---------------------------------------------------------
// NotifyStatusChange
// Request for a notify for change of connection status
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::NotifyStatusChange(
        THIDStateUpdateBuf& aUpdateParams, TDes8& aDiagnostic,
        TRequestStatus& aStatus)
    {
    SendReceive(EBTHIDServNotifyConnectionChange, TIpcArgs(&aUpdateParams,
            &aDiagnostic), aStatus);
    }

// ---------------------------------------------------------
// CancelNotifyStatusChange
// Cancel a outstanding request for a notify for change of connection status
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::CancelNotifyStatusChange() const
    {
    SendReceive(EBTHIDServCancelNotify);
    }

// ---------------------------------------------------------
// DisconnectDevice
// Disconnect from the selected HID device
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::DisconnectDevice(
        const TBTDevAddrPckgBuf& aAddrBuf, TBTDisconnectType aDiscType,
        TRequestStatus& aStatus)
    {
    TBTHidParamPkg discPkg(aDiscType);
    SendReceive(EBTHIDServDisconnectDevice, TIpcArgs(&aAddrBuf, &discPkg),
            aStatus);
    }

// ---------------------------------------------------------
// DisconnectAllGracefully
// Disconnect from the all HID devices
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::DisconnectAllGracefully(TRequestStatus& aStatus)
    {
        TRACE_FUNC
    SendReceive(EBTHidSrvDisconnectAllGracefully, aStatus);
    }

// ---------------------------------------------------------
// IsConnected
// Request for the connection status for a specified device
// ---------------------------------------------------------
//
EXPORT_C TInt RBTHidClient::IsConnected(const TBTDevAddr& aBTaddr)
    {
    TPckgC<TBTDevAddr> addrPck(aBTaddr);
    return SendReceive(EBTHIDServIsConnected, TIpcArgs(&addrPck));
    }

// ---------------------------------------------------------
// GetConnections
// Request for a list of current connected HID device addresses
// ---------------------------------------------------------
//
EXPORT_C void RBTHidClient::GetConnections(TDes8& aAddrs, TInt aProfile)
    {
        TRACE_FUNC
    SendReceive(EBTHIDGetConnections, TIpcArgs(&aAddrs, aProfile));
    }

TInt StartServer()
    {
        TRACE_STATIC_FUNC
    const TUid KKBTHIDSrvUid3 =
        {
        BTHID_SRV_UID
        };
    const TUidType serverUid(KNullUid,KNullUid, KKBTHIDSrvUid3);
    RProcess server;
    TInt r = server.Create(KBTHidServerFilename, KNullDesC, serverUid);
    if (r != KErrNone)
        return r;
    server.SetPriority(EPriorityHigh);

    TRequestStatus stat;
    server.Rendezvous(stat);
    if (stat != KRequestPending)
        {
        server.Kill(0);
        }
    else
        {
        server.Resume();
        }

    User::WaitForRequest(stat);
        TRACE_INFO((_L("[BTHID] Server started, code %d"), stat.Int()))
    r = (server.ExitType() == EExitPanic) ? KErrGeneral : stat.Int();
    server.Close();
    return r;
    }
// End of File
