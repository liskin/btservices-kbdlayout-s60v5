/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/

#ifndef BTNOTIFSECURITYMANAGER_H
#define BTNOTIFSECURITYMANAGER_H

#include <bttypes.h>
#include <bt_sock.h>
#include <bluetooth/pairing.h>
#include <btservices/btsimpleactive.h>
#include <btservices/btdevrepository.h>
#include <btengconstants.h>

class CBTNotifBasePairingHandler;
class CBTNotifPairNotifier;
class CBTNotifConnectionTracker;
class CBtDevRepository;
class CBTNotifServiceAuthorizer;

/**
 *  Class CBTNotifSecurityManager
 *
 *  This class manages pairing with BT devices.
 *  The responsibility of handling incoming and outgoing pairings is
 *  delegated to CBTNotifIncomingPairingHandler and CBTNotifOutgoingPairingHandler 
 *  respectively.
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifSecurityManager ) : 
    public CBase, 
    public MBtSimpleActiveObserver,
    public MBtDevRepositoryObserver
    {
public:

    /**
     * Two-phase constructor
     */
    static CBTNotifSecurityManager* NewL( 
            CBTNotifConnectionTracker& aParent, 
            CBtDevRepository& aDevRepository );

    /**
     * Destructor
     */
    ~CBTNotifSecurityManager();

    /**
     * Cancels an outstanding pairing request.
     */
    void CancelOutgoingPair();    
    
    /**
     * Process commands relevant to pairing
     */
    void HandleBondingRequestL( const RMessage2& aMessage );

    void HandleNotifierRequestL( const RMessage2& aMessage );
    
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
     * gets the reference of devrepository.
     */
    CBtDevRepository& BTDevRepository();
    
    /**
     * gets the reference of connection tracker.
     */
    CBTNotifConnectionTracker& ConnectionTracker();
    
    /**
     *  Transfer responsiblity to the specified object.
     *  @param aPairer the object whose the responsibility is transfered.
     */
    void RenewPairingHandler( CBTNotifBasePairingHandler* aPairingHandler );
    
    /**
     * Be notified when handling of an outgoing pair has been completed.
     * @param aErr the result of pairing
     * 
     */
    void OutgoingPairCompleted( TInt aErr );
    
    /**
     * Be informed that a session will be closed.
     *
     * @since Symbian^4
     * @param aSession the session to be cloased.
     */
    void SessionClosed(CSession2* aSession );
    
    /**
     * Unpair a device via registry
     */
    void UnpairDevice( const TBTDevAddr& aAddr );

    /**
     * Block a device via registry
     */
    void BlockDevice( const TBTDevAddr& aAddr , TBool aBanned);
    
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
     * Gets the pin code to be used for pairing a device.
     * @param aPin contains the pin code if it is not empty
     * @param aAddr the device to which pairing is performed.
     * @param the required minimum length of a pin code.
     */
    void GetPinCode( TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength );
    
    /**
     * Returns the service (limited to services managed in bteng scope)
     * level connection status of the specified device.
     *
     * @param aAddr the address of the device
     * @return one of TBTEngConnectionStatus enums
     */
    TBTEngConnectionStatus ConnectStatus( const TBTDevAddr& aAddr );
    
    /**
     * Activate / deactivate a pair observer
     */
    TInt SetPairObserver(const TBTDevAddr& aAddr, TBool aActivate);
    
    /**
     * Trust a device via the registry
     */
    void TrustDevice( const TBTDevAddr& aAddr );
    
private: 
    
    // from base class MBtSimpleActiveObserver

    /**
     * 
     * Callback to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );

    /**
     * Callback from Docancel() for handling cancelation of an outstanding request.
     *
     * @since Symbian^4
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );
    
    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBtSimpleActive* aActive, TInt aError );

    // From base class MBtDevRepositoryObserver
    
    /**
     * Callback to notify that the repository has finished initialization.
     * Initialization completion means the repository has retieved all
     * Bluetooth devices from BT registry, and it is subscribing to
     * registry update events.
     */
    virtual void RepositoryInitialized();    
    
    /**
     * Callback to notify that a device has been deleted from BT registry.
     * 
     *
     * @param aAddr the bd_addr of the deleted device
     */
    virtual void DeletedFromRegistry( const TBTDevAddr& aAddr );
    
    /**
     * Callback to notify that the device has been added to BT registry.
     *
     * @param aDevice the device that has been added to registry
     */
    virtual void AddedToRegistry( const CBtDevExtension& aDevice );
    
    /**
     * Callback to notify that the property of a device in BT registry has been
     * changed.
     *
     * @param aDevice the device that possesses the latest properties.
     * @param aSimilarity the similarity of the properties comparing to the ones
     *        prior to this change.
     *        Refer CBTDevice::TBTDeviceNameSelector and 
     *        TBTNamelessDevice::TBTDeviceSet for the meanings of the bits 
     *        in this parameter.
     */
    virtual void ChangedInRegistry(
            const CBtDevExtension& aDevice, TUint aSimilarity ); 
    
    /**
     * Callback to notify that the status of service (limited to 
     * services maintained in btengsrv scope) connections with 
     * a device has changed.
     *
     * @param aDevice the device to which the status change refers
     * @param aConnected ETrue if at least one service is currently connected.
     *        EFalse if no service is currently connected.
     */
    virtual void ServiceConnectionChanged(
            const CBtDevExtension& aDevice, TBool aConnected );

    
private:
    
    /**
     * C++ default constructor
     */
    CBTNotifSecurityManager(
            CBTNotifConnectionTracker& aParent,
            CBtDevRepository& aDevRepository );
    
    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * Subscribe local device address for determining 
     * Bluetooth HW power status
     */
    void SubscribeLocalAddress();

    TBool IsLocalAddressAvailable();
   
    /**
     * Pair a BT device.
     */
    void PairDeviceL( const TBTDevAddr& aAddr, TUint32 aCod );
    
    /**
     * Cancel all subscribes to pairng server
     */
    void CancelSubscribePairingAuthenticate();
    
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
     * @since Symbian^4
     * @param aResult The status code of the authentication result.
     */
    void HandlePairingResultL( const TBTDevAddr& aAddr, TInt aResult );
    
    /**
     * Copy the paired devices to internal array storage.
     */
    void UpdatePairedDeviceListL();
    
    /**
     * Handles a new bonding event from Registry.
     */
    void HandleRegistryBondingL( const TBTNamelessDevice& aNameless );
    
   
private:

    /**
     * Owner of this class.
     * Not owned.
     */
    CBTNotifConnectionTracker& iParent;
    
    /**
     * Reference to repository.
     * Not owned.
     */
    CBtDevRepository& iDevRepository;
    
    /**
     * The session with BT registry.
     * own.
     */
    RBTRegistry iRegistry;
    
    /**
     * contains the list of all paired devices.
     */
    RArray<TBTNamelessDevice> iPairedDevices;

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
    CBtSimpleActive* iSSPResultActive;

    /**
     * Active object helper for receiving authentication results.
     * Own.
     */
    CBtSimpleActive* iAuthenResultActive;

    /**
     * pairing hanlder at the time.
     * Own.
     */
    CBTNotifBasePairingHandler* iPairingHandler;
    
    /**
     * The handler for pairing notifiers 
     */
    CBTNotifPairNotifier* iPairNotifier;
    
    /**
     * Client-server message for power change requests.
     */
    RMessage2 iMessage;

    /**
     * AO for local address updates.
     */
    CBtSimpleActive* iLocalAddrActive;

    /**
     * Provides access to the BT local device address.
     */
    RProperty iPropertyLocalAddr;
    
    CBTNotifServiceAuthorizer* iServiceAuthorizer;
    };

#endif /*BTNOTIFSECURITYMANAGER_H*/



