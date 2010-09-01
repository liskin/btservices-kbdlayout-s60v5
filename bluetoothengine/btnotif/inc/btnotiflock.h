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
* Description:  Implements getting/setting notifier locks on BT devices
*               from/to P&S KBTNotifierLocks from btengprivatepskeys.h.
*
*/

#ifndef BTNOTIFIERLOCK_H
#define BTNOTIFIERLOCK_H

#include <bttypes.h>
#include <btengprivatepskeys.h>

/**
 *  BT Notifier lock identifiers 
 *  @since S60 v5.1
 */

enum TBTNotifLockId
    {
    /**
     * None
     */
    EBTNotifierLockNone = 0x00,
    
    /**
     * notifier is asking or going to ask if user wants to set device trusted.
     */
    EBTNotiferLockPairedDeviceSetting = 0x01,
    
    /**
     * notifer is asking or going to ask user if he wants to accept connection request
     * from the device.
     */
    EBTNotifierLockAuthorizeConnectRequest = 0x02,
    };

/**
 * the notifier lock status on a BT device.
 * This lock mechanism uses by btui, bteng and btnotif to ensure the order of 
 * user interaction when a device is been paired.
 */
NONSHARABLE_CLASS( TBTNotifLock)
    {
public:
    
    /**
     * default constructor.  iAddr is initialized to zero BT address and iLocks is set to 0  
     */
    inline TBTNotifLock();
    
    /**
     * constructor to the specified value.
     * @param aAddr the address of the BT device this object is for.
     * @param aLocks one or more lock identifiers
     */
    inline TBTNotifLock(const TBTDevAddr& aAddr, TInt aLocks);
    
    /**
     * gets the target device address.
     * @return the target device address
     */
    inline const TBTDevAddr& Addr() const;
    
    /**
     * Gets the locks on the device.
     */
    inline TInt Locks() const;
    
    /**
     * Gets the modifiable locks on the device
     */
    inline TInt& Locks();

private:
    
    /**
     * the BT device this object is for
     */
    TBTDevAddr iAddr;
    
    /**
     * the lock status, logical OR of one or more TBTNotifLockId values
     */
    TInt iLocks;
    };

NONSHARABLE_CLASS( TBTNotifLockPublish )
    {
public:

    /**
     * gets from P&S KBTNotifierLocks the locks for a device.
     * 
     * @param aLocks contains the locks for the device at return
     * @param aAddr the address of the device which the locks are for
     */
    inline static void GetNotifLocks( TInt& aLocks, const TBTDevAddr& aAddr );

    /**
     * gets from P&S KBTNotifierLocks the locks for a device.
     * 
     * @aProperty the property instance which has been attached to key KBTNotifierLocks. 
     * @param aLocks contains the locks for the device at return
     * @param aAddr the address of the device which the locks are for
     */
    inline static void GetNotifLocks(RProperty& aProperty, TInt& aLocks, 
            const TBTDevAddr& aAddr );
    
    /**
     * Publishes locks to P&S KBTNotifierLocks.
     * 
     * @param aLocks the locks to be published
     * @param aAddr the address of the device which the locks are for
     */
    inline static void AddNotifLocks( TInt aLocks, const TBTDevAddr& aAddr );    
    
    /**
     * Publishes locks to P&S KBTNotifierLocks.
     * @aProperty the property instance which has been attached to key KBTNotifierLocks. 
     * @param aLocks the locks to be published
     * @param aAddr the address of the device which the locks are for
     */
    inline static void AddNotifLocks(RProperty& aProperty, TInt aLocks, 
            const TBTDevAddr& aAddr );    
    
    /**
     * Deletes locks from P&S KBTNotifierLocks.
     * @param aLocks the locks to be deleted
     * @param aAddr the address of the device which the locks are for     
     */
    inline static void DeleteNotifLocks( TInt aLocks, 
            const TBTDevAddr& aAddr );    
    
    /**
     * Deletes locks from P&S KBTNotifierLocks.
     * @aProperty the property instance which has been attached to key KBTNotifierLocks.  
     * @param aLocks the locks to be deleted
     * @param aAddr the address of the device which the locks are for     
     */
    inline static void DeleteNotifLocks(RProperty& aProperty, TInt aLocks, 
            const TBTDevAddr& aAddr );    
    };

#include "btnotiflock.inl"

#endif // BTNOTIFIERLOCK_H
