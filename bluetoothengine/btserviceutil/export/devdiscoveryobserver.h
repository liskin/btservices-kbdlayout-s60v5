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

#ifndef DEVDISCOVEREYOBSERVER_H
#define DEVDISCOVEREYOBSERVER_H

#include <btdevice.h>
#include <bt_sock.h>

/**
 *  Class MDevDiscoveryObserver
 *
 *  Callback class to notify the completion of device discovery. 
 */
class MDevDiscoveryObserver
    {
public:
    
    /**
     * Callback to notify that a device has been found.
     *
     * @param aAddr the inquiry address that contains the inquiry information
     *  of the found device.
     * @param aName the Bluetooth device name of the found device
     */
    virtual void HandleNextDiscoveryResultL( 
            const TInquirySockAddr& aAddr, const TDesC& aName ) = 0;
    
    /**
     * Callback to notify that the device search has completed.
     *
     * @param aErr the error code of device search result.
     */
    virtual void HandleDiscoveryCompleted( TInt aErr ) = 0;
    };

#endif

// End of File
