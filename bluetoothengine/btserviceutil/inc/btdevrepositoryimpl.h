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
* Description:  The repository of remote devices from BT registry
*
*/

#ifndef BTDEVICEREPOSITORYIMPL_H
#define BTDEVICEREPOSITORYIMPL_H

#include <bttypes.h>
#include <bt_sock.h>
#include <btmanclient.h>
#include <e32property.h>
#include <btengconnman.h>
#include <btservices/btsimpleactive.h>
#include <btservices/btdevextension.h>

class MBtDevRepositoryObserver;

/**
 *  Class CBtDevRepositoryImpl
 *
 *  This class keep a storage of remote device information.
 *
 */
NONSHARABLE_CLASS( CBtDevRepositoryImpl ) : 
    public CBase, 
    public MBtSimpleActiveObserver,
    public MBTEngConnObserver
    {
public:

    /**
     * Two-phase constructor
     */
    static CBtDevRepositoryImpl* NewL();

    /**
     * Destructor
     */
    ~CBtDevRepositoryImpl();

    /**
     * Add an observer to this reposity for receivng repository update
     * events.
     * @param aObserver the observer to be added.
     */
    void AddObserverL( MBtDevRepositoryObserver* aObserver );

    /**
     * Remove an observer from this repository.
     * @param aObserver the observer to be removed.
     */
    void RemoveObserver( MBtDevRepositoryObserver* aObserver );
    
    /**
     * Tells if this repository has finished the initialization.
     * Initialization completion means the repository has retieved all
     * Bluetooth devices from BT registry, and it is subscribing to
     * registry update events.
     */
    TBool IsInitialized() const;
    
    /**
     * Gets all devices in this repository.
     * @return the device list.
     */
    const RDevExtensionArray& AllDevices() const;
    
    /**
     * Get a specific device by the given address.
     * @param aAddr the address of the device to be retrieved
     * @return the device pointer, NULL if the device is unavailable.
     */
    const CBtDevExtension* Device( const TBTDevAddr& aAddr ) const;
    
    /**
     * Forces the repository to initialize its data store.
     * At Initialization completion, corresponding callback will be invoked.
     * Initialization completion means the repository has retieved all
     * Bluetooth devices from BT registry, and it is subscribing to
     * registry update events.
     * 
     */
     void ReInitialize();
    
    /**
     * Returns the service (limited to services managed in bteng scope)
     * level connection status of the specified device.
     *
     * @param aAddr the address of the device
     * @return one of TBTEngConnectionStatus enums
     */
    //TBTEngConnectionStatus IsDeviceConnected( const TBTDevAddr& aAddr );
    
private:
    
    // from MBtSimpleActiveObserver
    
    /**
     * Callback from RunL() to notify that an outstanding request has completed.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aStatus The status of the completed request.
     */
    void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );

    /**
     * Callback from Docancel() for handling cancelation of an outstanding request.
     *
     * @since Symbian^4
     * @param aId The ID that identifies the outstanding request.
     */
    void CancelRequest( TInt aRequestId );

    /**
     * Callback from RunError() to notify that an error has occurred in RunL.
     *
     * @since Symbian^4
     * @param aActive Pointer to the active object that completed.
     * @param aError The error occurred in RunL.
     */
    void HandleError( CBtSimpleActive* aActive, TInt aError );

    // From MBTEngConnObserver
    
    /**
     * Indicates to the caller that a service-level connection has completed.
     * This function is called for both incoming and outgoing connections. 
     * This function is also called when an outgoing connection request fails, 
     * e.g. with error code KErrCouldNotConnect.
     * When this function is called, new commands can be issued to the 
     * CBTEngConnMan API immediately.
     * @param  aAddr The address of the remote device.
     * @param  aErr Status information of the connection. KErrNone if the
     *              connection succeeded, otherwise the error code with 
     *              which the outgoing connection failed. KErrAlreadyExists 
     *              is returned if there already is an existing connection 
     *              for the selected profile(s), or otherwise e.g. 
     *              KErrCouldNotConnect or KErrDisconnected for indicating 
     *              connection problems.
     * @param  aConflicts If there already is a connection for the selected 
     *                    profile(s) of an outgoing connection request (the 
     *                    selection is performed by BTEng), then this array 
     *                    contains the bluetooth device addresses of the 
     *                    remote devices for those connections.
     */
    void ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                          RBTDevAddrArray* aConflicts = NULL );

    /**
     * Indicates to the caller that a service-level connection has disconnected.
     * When this function is called, new commands can be issued to the 
     * CBTEngConnMan API immediately.
     * 
     * @param  aAddr The address of the remote device.
     * @param  aErr The error code with which the disconnection occured. 
     *              KErrNone for a normal disconnection, 
     *              or e.g. KErrDisconnected if the connection was lost.
     */
    void DisconnectComplete( TBTDevAddr& aAddr, TInt aErr );
    
private:
    
    /**
     * C++ default constructor
     */
    CBtDevRepositoryImpl();
    
    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();
    
    /**
     * Creates a registry view which shall contain all remote devices.
     */    
    void CreateRemoteDeviceView();
    
    /**
     * retrieves remote devices from registry.
     */
    void GetRemoteDevicesL();
    
    /**
     * handles the completion of paired device view creation
     */
    void HandleCreateRemoteDeviceViewCompletedL( TInt aStatus);
    
    /**
     * handles the completion of getting paired devices
     */
    void HandleGetRemoteDevicesCompletedL( TInt aStatus );
    
    /**
     * Copy the remote devices to internal array storage.
     */
    void UpdateRemoteDeviceRepositoryL();
    
private:
    
    /**
     * The observers
     * do not own them.
     */
    RPointerArray<MBtDevRepositoryObserver> iObservers;
    
    /**
     * Registry sub session for remote device db.
     * not own
     */
    RBTRegistry iBTRegistry;
    
    // own.
    RBTRegServ iBTRegServ;
    
    /**
     * AO for registry operations
     * own.
     */
    CBtSimpleActive* iRegistryActive;    
    
    /**
     * the counter of not handled registry events.
     */
    TInt iNotHandledRegEventCounter;
    
    /**
     * temporary instance to retrieve paired devices.
     * own.
     */
    CBTRegistryResponse* iRegRespRemoteDevices;

    /**
     * Property containing the BT registry change monitoring key
     * own.
     */
    RProperty iBtRegistryKey;
     
    /**
     * AO for subscribing registry PubSub key
     * own.
     */
    CBtSimpleActive* iRegistryKeyActive;  
    
    /**
     * For connection status
     */
    CBTEngConnMan* iBtengConn;
    
    /**
     * contains the list of all devices.
     * own.
     */
    RDevExtensionArray iDevices;

    TInt iInitialized;
    
    };

#endif /*BTDEVICEREPOSITORYIMPL_H*/
