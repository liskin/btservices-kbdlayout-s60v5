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

#ifndef BTDEVICEREPOSITORY_H
#define BTDEVICEREPOSITORY_H

#include <btdevice.h>
#include <btmanclient.h>
#include <btservices/btdevextension.h>

class CBtDevRepositoryImpl;

/**
 * APIs from this class offer functionalities that are common in mw and app 
 * components of Bluetooth packages. They do not serve as domain APIs.
 * 
 * Using these from external components is risky, due to possible source
 * and binary breaks in future.
 * 
 */

/**
 *  Class MBtDevRepositoryObserver
 *
 *  Callback class to notify changes in BT registry.
 */
class MBtDevRepositoryObserver
    {
public:

    /**
     * Callback to notify that the repository has finished initialization.
     * Initialization completion means the repository has retieved all
     * Bluetooth devices from BT registry, and it is subscribing to
     * registry update events.
     * 
     */
    virtual void RepositoryInitialized() = 0;    
    
    /**
     * Callback to notify that a device has been deleted from BT registry.
     * 
     *
     * @param aAddr the bd_addr of the deleted device
     */
    virtual void DeletedFromRegistry( const TBTDevAddr& aAddr ) = 0;
    
    /**
     * Callback to notify that the device has been added to BT registry.
     *
     * @param aDevice the device that has been added to registry
     */
    virtual void AddedToRegistry( const CBtDevExtension& aDevice ) = 0;
    
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
            const CBtDevExtension& aDevice, TUint aSimilarity ) = 0; 
    
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
            const CBtDevExtension& aDevice, TBool aConnected ) = 0;
    };

/**
 *  Class CBtDevRepository
 *
 *  This class provides the full access to remote devices in BT registry, 
 *  and informs client when the properties of remote devices are changed.
 *
 */
NONSHARABLE_CLASS( CBtDevRepository ) : public CBase
    {
public:

    /**
     * Two-phase constructor
     */
    IMPORT_C static CBtDevRepository* NewL();

    /**
     * Destructor
     */
    IMPORT_C virtual ~CBtDevRepository();

    /**
     * Add an observer to this reposity for receivng repository update
     * events.
     * @param aObserver the observer to be added.
     */
    IMPORT_C void AddObserverL( MBtDevRepositoryObserver* aObserver );

    /**
     * Remove an observer from this repository.
     * @param aObserver the observer to be removed.
     */
    IMPORT_C void RemoveObserver( MBtDevRepositoryObserver* aObserver );
    
    /**
     * Tells if this repository has finished the initialization.
     * Initialization completion means the repository has retieved all
     * Bluetooth devices from BT registry, and it is subscribing to
     * registry update events.
     */
    IMPORT_C TBool IsInitialized() const;
    
    /**
     * Get all devices in this repository.
     * @return the device list
     */
    IMPORT_C const RDevExtensionArray& AllDevices() const;

    /**
     * Get a specific device by the given address.
     * @param aAddr the address of the device to be retrieved
     * @return the device pointer, NULL if the device is unavailable.
     */
    IMPORT_C const CBtDevExtension* Device( const TBTDevAddr& aAddr ) const;

    /**
     * Forces the repository to initialize its data store.
     * At Initialization completion, corresponding callback will be invoked.
     * Initialization completion means the repository has retieved all
     * Bluetooth devices from BT registry, and it is subscribing to
     * registry update events.
     * 
     */
    IMPORT_C void ReInitialize();
    
private:
    
    /**
     * C++ default constructor
     */
    CBtDevRepository();
    
    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();
    
private:
    
    CBtDevRepositoryImpl* iImpl; 
    };

#endif /*BTDEVICEREPOSITORY_H*/
