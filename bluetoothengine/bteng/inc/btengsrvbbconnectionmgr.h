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
* Description:  Helper class for handling Bluetooth Baseband-related 
*                connection management.
*
*/



#ifndef BTENGSRVBBCONNECTIONMGR_H
#define BTENGSRVBBCONNECTIONMGR_H


#include <bt_sock.h>

#include "btengactive.h"

class CBTEngServer;

/**  ?description */
//const ?type ?constant_var = ?constant;


/**
 *  ?one_line_short_description
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSrvBBConnMgr ) : public CBase, 
                                          public MBluetoothPhysicalLinksNotifier, 
                                          public MBTEngActiveObserver
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngSrvBBConnMgr* NewL(CBTEngServer* aServer, RSocketServ& aSockServ);

    /**
     * Destructor
     */
    virtual ~CBTEngSrvBBConnMgr();

    /**
     * Subscribes to relevant events from BT Baseband, and also WLAN events.
     *
     * @since S60 v3.2
     */
    void Subscribe();

    /**
     * Stops listening to BT Baseband and WLAN events.
     *
     * @since S60 v3.2
     */
    void Unsubscribe();

    /**
     * Attempts to re-arrange the topology into a single piconet if necessary.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    TInt ManageTopology( TBool aPrepDiscovery );

    /**
     * Gets the addresses of all connected Bluetooth devices.
     *
     * @since S60 v3.2
     * @param On return, holds the addresses of connected BT devices.
     */
    void GetConnectedAddressesL( RBTDevAddrArray& aAddrArray );

    /**
     * Disconnect all Bluetooth baseband connections.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void DisconnectAllLinksL( TCallBack& aCallBack );

    void DisconnectAllLinksForPowerOffL( TCallBack& aCallBack );

    
    /**
     * Indicate to turn off BT when idle.
     *
     * @since S60 v5.0
     * @param aEnable Enable or disable automatic power off.
     * @param aCallBack re-use DisconnectAll callback.
     */
    void SetAutoSwitchOff( TBool aEnable, TCallBack& aCallBack );

// from base class MBluetoothPhysicalLinksNotifier

    /**
     * From MBluetoothPhysicalLinksNotifier.
     * Notification of a requested connection coming up. Not used here.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleCreateConnectionCompleteL( TInt aErr );

    /**
     * From MBluetoothPhysicalLinksNotifier.
     * Notification of a requested disconnection having taken 
     * place. Not used here.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleDisconnectCompleteL( TInt aErr );

    /**
     * From MBluetoothPhysicalLinksNotifier.
     * Notification that all existing connections have been torn down.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleDisconnectAllCompleteL( TInt aErr );

// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 v3.2
     * @param aActive Pointer to the active object that completed.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTEngActive* aActive,
                                     TInt aStatus );

    /**
     * Callback for handling cancelation of an outstanding request.
     *
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );
    
    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an error has occurred in RunL.
     *
     * @since S60 v3.2
     * @param aActive Pointer to the active object that completed.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTEngActive* aActive,
                               TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngSrvBBConnMgr(CBTEngServer* aServer, RSocketServ& aSockServ);

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * Gets the WLAN connection status, if activated.
     *
     * @since S60 v3.2
     * @return ETrue if a WLAN connection is active, otherwise EFalse.
     */
    TBool GetWlanStatus();

private: // data

    /**
     * Flag to indicate if we should monitor WLAN connection status.
     */
    TBool iWlanSupported;

    /**
     * Flag to indicate that BT is to be switched off when active.
     */
    TBool iAutoSwitchOff;

    /**
     * ?description_of_member
     */
    RProperty iLinkCountProperty;

    /**
     * ?description_of_member
     */
    RProperty iWlanStatusProperty;

    /**
     * Session with the socket server. Not own!
     */
    RSocketServ& iSockServ;

    /**
     * BT Link Manager socket.
     * Own.
     */
    CBluetoothPhysicalLinks* iPhyLinks;

    /**
     * Active object for monitoring BT link count.
     * Own.
     */
    CBTEngActive* iLinkCountWatcher;

    /**
     * Active object for monitoring BT link count.
     * Own.
     */
    CBTEngActive* iWlanWatcher;

    /**
     * Callback function to call when disconnecting all links has completed.
     * Not own.
     */
    TCallBack iCallBack;
    
    /**
     * Pointer to our parent.
     * Not own.
     */
    CBTEngServer* iServer;
    
    };


#endif // BTENGSRVBBCONNECTIONMGR_H
