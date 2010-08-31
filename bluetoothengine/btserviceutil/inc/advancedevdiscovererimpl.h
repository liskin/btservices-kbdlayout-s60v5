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

#ifndef ADVANCEDEVDISCOVERERIMPL_H
#define ADVANCEDEVDISCOVERERIMPL_H

#include <btservices/devdiscoveryobserver.h>

#include <btservices/btsimpleactive.h>
#include <btservices/advancedevdiscoverer.h>

class CBasicDevDiscoverer;
class CBtDevRepository;
class CBtDevExtension;

/**
* A helper class for searching nearby Bluetooth devices.
*/
NONSHARABLE_CLASS(CAdvanceDevDiscovererImpl) : public CBase, 
        public MDevDiscoveryObserver,
        public MBtSimpleActiveObserver
    {
public:
    /**
     * instantiator
     */
    static CAdvanceDevDiscovererImpl* NewL( 
            CBtDevRepository& aDevRep,
            MDevDiscoveryObserver& aObserver );
    
    /**
    * Destructor.
    */
    virtual ~CAdvanceDevDiscovererImpl();
    
    /**
     * sets the inquiry result observer.
     * 
     * @param aObserver the new observer to receive inquiry results
     */
    void SetObserver( MDevDiscoveryObserver& aObserver );
    
    /**
    * Cancels all inquiry activity.
    */
    void Cancel();
    
    /**
    * Discover currently in-range devices that matches the given major device class type.
    * Found devices will be informed by 
    * MBtAdvanceDevDiscoveryObserver::HandleNextDiscoveryResult().
    * 
    * When no more device can be found, 
    * MBtAdvanceDevDiscoveryObserver::HandleDiscoveryCompleted() will be issued.
    * 
    * @param aFilter the filter that shall be applied when a device 
    *        is discovered. If this is specified, only a device passing 
    *        the filter will be informed to client. 
    * 
    * @param aDeviceClass the major device class which a found device 
    *        must match. 
    */
    void DiscoverDeviceL(
            CAdvanceDevDiscoverer::TDevDiscoveryFilter aFilter, 
            TBTMajorDeviceClass aDeviceClass );
    
    /**
    * Discover currently in-range devices that match the given major device class.
    * The devices in the given priority list have higher priority to 
    * be discovered, i.e., inquiry on these devices will take place first.
    * 
    * Found devices will be informed by 
    * MDevDiscoveryObserver::HandleNextDiscoveryResult().
    * 
    * When no more device can be found, 
    * MDevDiscoveryObserver::HandleDiscoveryCompleted() will be issued.
    * 
    * @param aPriorityList contains the devices to be discovered first
    */
    void DiscoverDeviceL(
            const RBTDevAddrArray& aPriorityList );
    
private: 
    
    // from MDevDiscoveryObserver
    
    /**
     * Callback to notify that a device has been found.
     *
     * @param aAddr the inquiry address that contains the inquiry information
     *  of the found device.
     * @param aName the Bluetooth device name of the found device
     */
    void HandleNextDiscoveryResultL( 
            const TInquirySockAddr& aAddr, const TDesC& aName );
    
    /**
     * Callback to notify that the device search has completed.
     *
     * @param aErr the error code of device search result.
     */
    void HandleDiscoveryCompleted( TInt aErr );

    // from MBtSimpleActiveObserver
       
    /**
    * Callback to notify that an outstanding request has completed.
    *
    * @param aActive Pointer to the active object that completed.
    * @param aStatus The status of the completed request.
    */
    void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );
    
    /**
    * Callback for handling cancelation of an outstanding request.
    *
    * @param aId The ID that identifies the outstanding request.
    */
    void CancelRequest( TInt aId );
    
    /**
    * Callback to notify that an error has occurred in RunL.
    *
    * @param aActive Pointer to the active object that completed.
    * @param aError The error occurred in RunL.
    */
    void HandleError( CBtSimpleActive* aActive, TInt aError );
    
private:
    
    /**
    * C++ default constructor.
    */
    CAdvanceDevDiscovererImpl( 
            CBtDevRepository& aDevRep,
            MDevDiscoveryObserver& aObserver );
    
    /**
    * The 2nd phase constructor
    */
    void ConstructL();

private:  // Data
    
    // for retrieving device properties
    // Not own
    CBtDevRepository& iDevRep;
    
    // our client
    // Not own
    MDevDiscoveryObserver& iObserver;
    
    // The class doing actual inquirying
    CBasicDevDiscoverer* iBasicDiscoverer;
    
    // filters from CAdvanceDevDiscoverer::TDevDiscoveryFilter
    TInt iFilter;
    };

#endif

// End of File
