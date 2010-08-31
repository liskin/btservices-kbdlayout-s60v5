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

#ifndef BASICDEVDISCOVERER_H
#define BASICDEVDISCOVERER_H

#include <btdevice.h>
#include <bt_sock.h>
#include <btservices/btsimpleactive.h>

class MDevDiscoveryObserver;

class CDeviceSearchRecord : public CBase
    {
public:
    TInquirySockAddr iAddr;
    TBTDeviceName iName;
    };

/**
* A basic implementation for searching nearby Bluetooth devices.
*/
NONSHARABLE_CLASS(CBasicDevDiscoverer) : public CBase, public MBtSimpleActiveObserver
    {
public:

    /**
     * factory method
     */
    static CBasicDevDiscoverer* NewL( MDevDiscoveryObserver& aObserver );
    
    /**
    * Destructor.
    */
    virtual ~CBasicDevDiscoverer();
    
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
    * MDevDiscoveryObserver::HandleNextDiscoveryResult().
    * 
    * When no device can be found any more, 
    * MDevDiscoveryObserver::HandleDiscoveryCompleted() will be issued.
    * 
    * @param aDeviceClass the major device class device must match.
    */
    void DiscoverDeviceL( TBTMajorDeviceClass aDeviceClass  = EMajorDeviceMisc );
    
private: // from MBtSimpleActiveObserver
    
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
    CBasicDevDiscoverer( MDevDiscoveryObserver& aObserver );
    
    /**
    * The 2nd phase constructor
    */
    void ConstructL();

    /*
     * retrieves the device name of the device pointed by 
     * iPagingNamePos to local array.
     */
    void PageNextDeviceName();
    
    /**
     * Performs the result from an inquiry operation.
     */
    TInt HandleInquiryResultL();
    
    /**
     * Create a heap object of CDeviceSearchRecord.
     */
    CDeviceSearchRecord* NewInstanceL( 
        const TInquirySockAddr& aAddr, const TDesC& aName = KNullDesC );

    /**
     * resets the state and memory caused by a client request.
     */
    void Reset();
    
private:  // Data

    // Not own
    MDevDiscoveryObserver& iObserver; 
    
    // The major device class filter from the client.
    TBTMajorDeviceClass iMajorDeviceClassFilter;
    
    // For inquiry and paging names:
    CBtSimpleActive* iActive;

    RSocketServ iSocketServer;
    RHostResolver iHostResolver;
    TInquirySockAddr iInquirySockAddr;
    TNameEntry iEntry; // Inquiry result record
    
    // Devices found so far
    RPointerArray<CDeviceSearchRecord> iDevices;
    
    // position in array iDevices: the item the current name paging operation is for
    TInt iPagingNamePos;

    };

#endif

// End of File
