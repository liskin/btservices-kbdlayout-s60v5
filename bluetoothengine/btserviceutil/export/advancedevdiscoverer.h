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
* Description:  Declares Bluetooth device inquiry helper class.
*
*/

#ifndef ADVANCEBTDEVDISCOVERER_H
#define ADVANCEBTDEVDISCOVERER_H

#include <btdevice.h>
#include <bt_sock.h>
#include <btservices/devdiscoveryobserver.h>
#include <btservices/btdevrepository.h>

class CAdvanceDevDiscovererImpl;

/**
 * APIs from this class offer functionalities that are common in mw and app 
 * components of Bluetooth packages. They do not serve as domain APIs.
 * 
 * Using these from external components is risky, due to possible source
 * and binary breaks in future.
 * 
 */

/**
* The interface of discovering in-range Bluetooth devices via Host Resolver.
* This class offers some options to search certain devices by CoD and other 
* filters e.g., bonded and blocked.
* 
* It requires the client to supply a CBtDevRepository instance for filtering
* devices.
*/
NONSHARABLE_CLASS( CAdvanceDevDiscoverer ) : public CBase
    {

public: // Constructor and destructor

    /**
     * Options for clients to specify criteria on discovered devices.
     */
    enum TDevDiscoveryFilter
        {
        // do not filter in-range devices
        ENoFilter = 0,

        // Excludes an in-range device if its device name is not available
        ExcludeIfNoDevName = 0x01,
        
        // Excludes an in-range device if it is user-aware bonded with phone.
        ExcludeUserAwareBonded = 0x02,
        
        // Excludes an in-range device if it is banned by phone.
        ExcludeBanned = 0x04,
        };

    IMPORT_C static CAdvanceDevDiscoverer* NewL(
            CBtDevRepository& aDevRep,
            MDevDiscoveryObserver& aObserver );
    
    /**
    * Destructor.
    */
    IMPORT_C virtual ~CAdvanceDevDiscoverer();

    /**
     * sets the discovery result receiver.
     * 
     * @param aObserver the new observer to receive inquiry results
     */
    IMPORT_C void SetObserver( MDevDiscoveryObserver& aObserver );
    
    /**
    * Discover currently in-range devices that matches the given major 
    * device class type.
     
    * Found devices will be informed by 
    * MBtDevDiscoveryObserver::HandleNextDiscoveryResult().
    * 
    * When no more device can be found, 
    * MBtDevDiscoveryObserver::HandleDiscoveryCompleted() will be issued.
    * 
    * @param aFilter the filter that shall be applied when a device 
    *        is discovered. If this is specified, only a device passing 
    *        the filter will be informed to client. 
    *        By default, no filter is applied.
    * 
    * @param aDeviceClass the major device class which a found device 
    *        must match. 
    *        By default, it includes any device types.
    */
    IMPORT_C void DiscoverDeviceL( 
            TDevDiscoveryFilter aFilter = CAdvanceDevDiscoverer::ENoFilter, 
            TBTMajorDeviceClass aDeviceClass = EMajorDeviceMisc );

    /**
    * Cancels any outstanding discovery request.
    */
    IMPORT_C void CancelDiscovery();
    
private:

    /**
    * C++ default constructor.
    */
    CAdvanceDevDiscoverer();

    /**
    * The 2nd phase constructor
    */
    void ConstructL( CBtDevRepository& aDevRep, MDevDiscoveryObserver& aObserver );

private:  // Data

    CAdvanceDevDiscovererImpl* iImpl;
};

#endif

// End of File
