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
* Description:  Bluetooth HID ECom plug-in class declaration.
 *
*/


#ifndef BTHIDPLUGIN_H
#define BTHIDPLUGIN_H

#include <e32base.h>
#include <bttypes.h>
#include <bt_sock.h>
#include <btengplugin.h>
#include <btengconstants.h>
#include "bthidclient.h"
#include "genericactive.h"
#include "bthidclientsrv.h"

/**
 *  BTEng plug-in base class.
 *
 *
 *  @lib 
 *  @since S60 v5.0
 */
class RBTHidClient;

class CBTHidPlugin : public CBTEngPlugin, public MGenericActiveObserver
    {

public:

    /**
     * Constructor; loads the plug-in through the ECom framework.
     *
     * @since S60 v5.0
     * @param aImplementationUid The UID of the interface implementation 
     *                           to be loaded.
     * @return Pointer to the constructed CBTHidPlugin object.
     */
    static CBTHidPlugin* NewL();

    /**
     * Destructor; destroys the plug-in through the ECom framework.
     */
    virtual ~CBTHidPlugin();

    /**
     * Gets the list of profiles implemented by this plug-in.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * @since S60 v5.0
     * @param aProfiles Array of integers identifying the BT profiles 
     *                  implemented by this plug-in.
     */
    void GetSupportedProfiles(RProfileArray& aProfiles);

    /**
     * Sets the observer for receiving connection events through the 
     * MBTEngPluginObserver interface. This method MUST be called 
     * before any command is given to the plug-in
     *
     * @since S60 v5.0
     * @param aObserver The observer of connection events.
     */
    void SetObserver(MBTEngPluginObserver* aObserver);

    /**
     * Indicates if this plug-in implements the specified profiles.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * @since S60 v5.0
     * @param aProfile Integer identifying the BT profile.
     * @return ETrue if the plug-in implements the requested profile, 
     *               otherwise EFalse.
     */
    TBool IsProfileSupported(const TBTProfile aProfile) const;

    /**
     * Instructs the plug-in to create a service-level connection 
     * with the specified Bluetooth address.
     *
     * This is an asynchronous operation; on completion, the plug-in informs 
     * the observer by calling MBTEngPluginObserver::ConnectComplete with 
     * the appropriate error code.
     *
     * @since S60 v5.0
     * @param aAddr The BT device address identifying a remote device.
     * @return KErrNone on success, otherwise an appropriate error code.
     */
    TInt Connect(const TBTDevAddr& aAddr);

    /**
     * Instructs the plug-in to cancel the creation a service-level 
     * connection with the specified Bluetooth address.
     *
     * In addition, as this is part of an asynchronous operation, the plug-in 
     * completes the outstanding asyncronous request status with KErrCancel.
     *
     * @since S60 v5.0
     * @param aAddr The BT device address identifying a remote device.
     */
    void CancelConnect(const TBTDevAddr& aAddr);

    /**
     * Instructs the plug-in to create a service-level connection 
     * with the specified Bluetooth address.
     *
     * This is an asynchronous operation; on completion, the plug-in informs 
     * the observer by calling MBTEngPluginObserver::DisconnectComplete with 
     * the appropriate error code.
     *
     *
     * @since S60 v5.0
     * @param aAddr The BT device address identifying a remote device.
     *              In case the address is the NULL address, the plug-in is
     *              expected to disconnect all its connections.
     * @param aDiscType The type of disconnection; 
     *                  EGraceful for graceful (normal) disconnection, 
     *                  EImmediate for immediate (forced) disconnection.
     * @return KErrNone on success, otherwise an appropriate error code.
     */
    TInt Disconnect(const TBTDevAddr& aAddr, TBTDisconnectType aDiscType);

    /**
     * Gets the addresses of the connected devices for the specified profile.
     *
     * @since S60 v5.0
     * @param aAddrArray On return, holds the Bluetooth device addresses 
     *                   of the connected Bluetooth devices for the 
     *                   requested profile.
     * @param aConnectedProfile The profile for which the existing 
     *                          connections are requested.
     * @return KErrNone if sucessful, otherwise the error code 
     *                  indicating the error situation.
     */
    void GetConnections(RBTDevAddrArray& aAddrArray,
            TBTProfile aConnectedProfile);

    /**
     * Indicates if this plug-in has a service-level connection 
     * with the specified Bluetooth address.
     *
     * @since S60 v5.0
     * @param aAddr The BT device address identifying a remote device.
     * @return The status of the connection according to the TConnectionStatus
     *         enumeration. EConnecting is interpreted that at least one 
     *         incoming connection request has been completed, but that not 
     *         all service-level connections have been fully established.
     */
    TBTEngConnectionStatus IsConnected(const TBTDevAddr& aAddr);

private:
    //From MGenericActiveObserver
    void RequestCompletedL(CGenericActive& aActive);

    void CancelRequest(CGenericActive& aActive);

private:

    CBTHidPlugin();

    void ConstructL();

    //Handle async requests
    TInt HandleAsyncRequest(const TBTDevAddr& aAddr, TInt aRequestId);

    //Report the Profile connection event to BTEng
    void ReportProfileConnectionEvents(const TBTDevAddr& aAddr,
            TBool aConnected);

    //Handle the complete of request for NotifyProfileStatusChange
    void HandleNotifyProfileStatusChange(CGenericActive& aActive);

    //Handle the complete of request for RequestConnect
    void HandelRequestConnectComplete();

    //Handle the complete of request for RequestDisconnect
    void HandelRequestDisconnectComplete();

private:
    // data

    MBTEngPluginObserver* iObserver; // not own

    //Client API for BTHID Server
    RBTHidClient iClient;

    //Active Object for async request Connect/Disconnect
    CGenericActive* iActive4ClientReq;

    //Packagebuf of TBTDevAddr
    TBTDevAddrPckgBuf iBTDevAddrPckgBuf;

    //BT address buf for conflict address, when connecting to HID device
    TBuf8<KBTDevAddrSize * 2> iDiagnostic;

    //BT address buf for conflict address, when HID device reconnect
    TBuf8<KBTDevAddrSize * 2> iDiagnosticAddress;

    //Active Object for async request profile status update
    CGenericActive* iActive4ProfileStatus;

    //Packagebuf of HID status update
    THIDStateUpdateBuf iHIDStateUpdatePckg;
    };

#endif // BTHIDPLUGIN_H
