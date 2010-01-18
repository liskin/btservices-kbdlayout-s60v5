/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Pairing manager header definition
*
*/

#ifndef BTENGPAIRMANAGER_H_
#define BTENGPAIRMANAGER_H_

#include <bttypes.h>
#include <bt_sock.h>
#include <bluetooth/pairing.h>
#include "btengactive.h"
#include "btengconstants.h"

class CBTEngPairBase;
class CBTEngServer;

/**
 *  Class CBTEngPairMan
 *
 *  This class manages pairing with BT devices.
 *  The responsibility of handling incoming and outgoing pairings is
 *  delegated to CBTEngIncPair and CBTEngOtgPair respectively.
 *
 *  @since S60 S60 v5.1
 */
NONSHARABLE_CLASS( CBTEngPairMan ) : public CBase, public MBTEngActiveObserver
    {
public:

    /**
     * Two-phase constructor
     */
    static CBTEngPairMan* NewL( CBTEngServer& aServer );

    /**
     * Destructor
     */
    ~CBTEngPairMan();

    /**
     * Cancels an outstanding pairing request.
     */
    void CancelOutgoingPair();    
    
    /**
     * Process commands relevant to pairing
     */
    void ProcessCommandL( const RMessage2& aMessage );
    
    /**
     * Cancels an outstanding command.
     * @param aOpcode the identifier of the command to be cancelled.
     */
    void CancelCommand( TInt aOpCode );
    
    /**
     * Handle a change in BTRegistry remote device table.
     *
     * @since S60 v5.1
     */
    void RemoteRegistryChangeDetected();

    /**
     * Gets the instance of pairing server.
     * @return the server instance. NULL if dedicated bonding is unavailable.
     */
    RBluetoothPairingServer* PairingServer();
    
    /**
     * gets the reference of socket server session
     */
    RSocketServ& SocketServ();
    
    /**
     * gets the reference of registry session.
     */
    RBTRegServ& BTRegServ();
    
    /**
     *  Transfer responsiblity to the specified object.
     *  @param aPairer the object to which the responsibility is transfered.
     */
    void RenewPairer( CBTEngPairBase* aPairer );
    
    /**
     * Be notified when handling of an outgoing pair has been completed.
     * @param aErr the result of pairing
     * 
     */
    void OutgoingPairCompleted( TInt aErr );
    
    /**
     * Unpair a device via registry
     */
    void UnpairDevice( const TBTDevAddr& aAddr );
    
    /**
     * Add the bit indicating the device is user-aware Just worked paired to
     * UI cookie.
     */
    TInt AddUiCookieJustWorksPaired( const TBTNamelessDevice& aDev );
    
    /**
     * Update a nameless device in registry
     */
    TInt UpdateRegDevice( const TBTNamelessDevice& aDev );
    
    
    /**
     * Returns the service (limited to services managed in bteng scope)
     * level connection status of the specified device.
     *
     * @param aAddr the address of the device
     * @return one of TBTEngConnectionStatus enums
     */
    TBTEngConnectionStatus IsDeviceConnected( const TBTDevAddr& aAddr );
    
private: // from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 v5.1
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aId, TInt aStatus );

    /**
     * From MBTEngActiveObserver.
     * Callback to notify that an error has occurred in RunL.
     *
     * @since S60 v5.1
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aId, TInt aError );

private:
    
    /**
     * C++ default constructor
     */
    CBTEngPairMan( CBTEngServer& aServer );
    
    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * Activate / deactivate a pair observer
     */
    TInt SetPairObserver(const TBTDevAddr& aAddr, TBool aActivate);
    
    /**
     * Pair a BT device.
     */
    void PairDeviceL( const TBTDevAddr& aAddr, TUint32 aCod );
    
    /**
     * Cancel all subscribes to pairng server
     */
    void CancelSubscribe();
    
    /**
     * subscribe to receive SSP pairing result from pairing server
     */
    void SubscribeSspPairingResult();
    
    /**
     * Subscribe to receive authentication result from pairing server
     */
    void SubscribeAuthenticateResult();

    /**
     * Handle a pairing result from the pairing server.
     *
     * @since S60 v5.1
     * @param aResult The status code of the authentication result.
     */
    void HandlePairingResultL( const TBTDevAddr& aAddr, TInt aResult );
    
    /**
     * Creates a registry view which shall contain all paired devices.
     */    
    void CreatePairedDevicesView( TInt aReqId );
    
    /**
     * retrieves paired devices from registry.
     */
    void GetPairedDevices( TInt aReqId );
    
    /**
     * handles the completion of paired device view creation
     */
    void HandleCreatePairedDevicesViewCompletedL( TInt aStatus, TInt aReqId );
    
    /**
     * handles the completion of getting paired devices
     */
    void HandleGetPairedDevicesCompletedL( TInt aStatus, TInt aReqId );
    
    /**
     * Copy the paired devices to internal array storage.
     */
    void UpdatePairedDeviceListL();
    
    /**
     * Checks if any devices have been paired or unpaired. For each new paired device,
     * the pair event will be delegated to corresponding pair observer if it exists;
     * otherwise create a new pair observer for this pair event.
     */
    void CheckPairEventL();
    
private:

    /**
     * Owner of this class.
     * Not owned.
     */
    CBTEngServer& iServer;
    
    /**
     * Registry sub session for remote device db
     */
    RBTRegistry iBTRegistry;
    
    /**
     * contains the list of all paired devices.
     * Using heap here to make efficient array swap.
     */
    RArray<TBTNamelessDevice>* iPairedDevices;
    
    /**
     * temporary instance to retrieve paired devices.
     */
    CBTRegistryResponse* iPairedDevicesResp;
    
    /**
     * the counter of not handled registry events.
     */
    TInt iNotHandledRegEventCounter;
    
    /**
     * Session with the pairing server.
     * Allocate it in heap to ease the handling for 
     * situation of unavailable dedicated pairing service.
     * 
     * own.
     */
    RBluetoothPairingServer* iPairingServ;

    /**
     * Subsession with the pairing server for 
     * getting the simple pairing result.
     * own
     */
    RBluetoothPairingResult iPairingResult;

    /**
     * Subsession with the pairing server for 
     * getting the authentication result.
     * own
     */
    RBluetoothAuthenticationResult iAuthenResult;

    /**
     * the address with which a simple pairing has been performed
     */
    TBTDevAddr iSimplePairingRemote;

    /**
     * the address with which an authentication has been performed
     */
    TBTDevAddr iAuthenticateRemote;
    
    /**
     * Active object helper for receiving simple pairing results.
     * Own.
     */
    CBTEngActive* iSSPResultActive;

    /**
     * Active object helper for receiving authentication results.
     * Own.
     */
    CBTEngActive* iAuthenResultActive;

    /**
     * AO for registry operations
     */
    CBTEngActive* iRegistryActive;
    
    /**
     * pair hanlder at the time.
     * Own.
     */
    CBTEngPairBase* iPairer;
    };

#endif /*BTENGPAIRMANAGER_H_*/
