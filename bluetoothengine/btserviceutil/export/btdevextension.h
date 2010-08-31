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
* Description:  an extended BT device offering properties of
* a Bluetooth device that may be needed by Bluetooth UIs
*
*/

#ifndef BTDEVEXTENSION_H
#define BTDEVEXTENSION_H

#include <btdevice.h>
#include <btmanclient.h>
#include <bt_sock.h>
#include <btengconstants.h>

/**
 * APIs from this class offer functionalities that are common in mw and app 
 * components of Bluetooth packages. They do not serve as domain APIs.
 * 
 * Using these from external components is risky, due to possible source
 * and binary breaks in future.
 * 
 */

/**
 * The option for specify the default name for a BT device in 
 * case the device has neither a device name nor a friendly name.
 */
enum TDefaultDevNameOption
    {
    ENoDefaultName,
    EColonSeperatedBDAddr, // device name will be formated
    EPlainBDAddr,
    };

/**
 *  Class CBtDevExtension
 *
 *  This class provides the access to the properties of devices from BT registry.
 *  In addition, it provides other dynamic properties such as the connection 
 *  and proximity statuses. Note that client should not store the dynamic 
 *  properties for future use since they may change frequently.
 *
 */
NONSHARABLE_CLASS( CBtDevExtension ) : public CBase
    {

public:
    // placeholder for providing more properties of a device
    enum TBtDevStatus
        {
        EUndefinedStatus,
        EInRangUnknown = 0x01,
        EInRange = 0x02,
        EPermanentInRegistry = 0x04,
        };
    
    /**
     * Two-phase constructor
     * @param aDev a CBTDevice instance. The ownership is transferred.
     * @param aNameOption the option for formating the default device
     *  name when the given aDev instance has no valid name.
     */
    IMPORT_C static CBtDevExtension* NewLC( CBTDevice* aDev, 
            TDefaultDevNameOption aNameOption = EColonSeperatedBDAddr );    

    /**
     * Two-phase constructor
     * @param aAddr the inquiry socket address of the device.
     * @param aName the device-name of the device.
     * @param aNameOption the option for formating the default device
     *  name when the given name is empty.
     */
    IMPORT_C static CBtDevExtension* NewLC( 
            const TInquirySockAddr& aAddr, 
            const TDesC& aName = KNullDesC,
            TDefaultDevNameOption aNameOption = EColonSeperatedBDAddr );        

    /**
     * Two-phase constructor
     * @param aAddr the bd_addr of the device.
     * @param aName the device-name of the device.
     * @param aNameOption the option for formating the default device
     *  name when the given name is empty.
     */
    IMPORT_C static CBtDevExtension* NewLC( 
            const TBTDevAddr& aAddr, 
            const TDesC& aName = KNullDesC,
            TDefaultDevNameOption aNameOption = EColonSeperatedBDAddr );      
    
    /**
     * Destructor
     */
    IMPORT_C virtual ~CBtDevExtension();
    
    /**
     * Tells if the given device is bonded regardless of whether the pairing was 
     * performed under user awareness.
     * 
     * @return ETrue if it is bonded.
    */
    IMPORT_C static TBool IsBonded( const TBTNamelessDevice &dev );
    
    /**
     * Tells if the given device is bonded with the Just Works pairing model.
     * 
     * @return ETrue if it is bonded and the pairing was performed with Just Works.
     */
    IMPORT_C static TBool IsJustWorksBonded( const TBTNamelessDevice &dev );
    
    /**
     *   Tells if the given device has been bonded under user awareness.
     *   User awareness refers that the user interacted or was informed during or 
     *   immediately after the pairing completed.
     *   
     *   @return ETrue if the user is aware of the bonding.
     */
    IMPORT_C static TBool IsUserAwareBonded( const TBTNamelessDevice &dev );
    
    /**
     *   Guesses if the given device is a headset.
     *   
     *   @param the Class of Device of the device of which the type is
     *          determined.
     *   @return ETrue if it is probably a headset. EFalse otherwise.
     */
    IMPORT_C static TBool IsHeadset( const TBTDeviceClass &aCod );    
    
    /**
     * Returns the display name of this device for end users. 
     * @return the friendly name of the device if it is available; else, the device
     * name if it is available; otherwise, the BDADDR seperated with ":".
     */
    IMPORT_C const TDesC& Alias() const;
    
    /**
     * Gets the device address.
     * @return the device.
     */
    IMPORT_C const TBTDevAddr& Addr() const;    
    
    /**
     * Gets the CBTDevice instance.
     * @return the device.
     */
    IMPORT_C const CBTDevice& Device() const;
    
    /**
     * Checks if this device was bonded under user awareness.
     * @return ETrue if it is user-aware bonded.
     */
    IMPORT_C TBool IsUserAwareBonded() const;
    
    /**
     * 
     * Returns the service (limited to services managed in bteng scope)
     * level connection status of the specified device.
     *
     * @param aAddr the address of the device
     * @return one of TBTEngConnectionStatus enums
     */
    IMPORT_C TBTEngConnectionStatus ServiceConnectionStatus() const;
    
    /**
     * Sets a device. The ownership is transferred.
     * 
     * @param aDev the device to be set.
     */
    IMPORT_C void SetDeviceL( CBTDevice* aDev );

    /**
     * Make a copy of this evice.
     * 
     * @return a new device instance.
     */
    IMPORT_C CBtDevExtension* CopyL();
    
public:
    
    /**
     * Internally invoked in this module, not a DLL level API.
     *
     * Sets the service connection status of this device.
     */
    void SetServiceConnectionStatus( TBTEngConnectionStatus aStatus ); 
    
private:
    
    /**
     * C++ default constructor
     */
    CBtDevExtension( TDefaultDevNameOption aNameOption );
    
    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL( CBTDevice* aDev );
    
    /**
     * Update device properties due to setDeviceL or other events.
     */
    void UpdateNameL();
    
    /**
     * Update the service connection status for this device:
     */
    //void UpdateServiceStatusL();
    
    /**
     * formats the BD_Addr as the device name. 
     */
    void FormatAddressAsNameL();
    
private:
    
    RBuf iAlias; // contains: 
                 // friendly name, if it is not empty; else
                 // device name, if it is not empty; else
                 // the assignment depending on the option chosen by client.
    
    // The Device instance ( in most case it is given by registry)
    CBTDevice* iDev;
    
    // The bits of dynamic status
    TInt iDynamicStatus;
    
    // Indicates the service connection status of this device
    TBTEngConnectionStatus iServiceStatus;
    
    // The option chosen by the client to deal with default BT name assignment.
    TDefaultDevNameOption iNameOption;
    };

typedef RPointerArray<CBtDevExtension> RDevExtensionArray;

#endif /*BTDEVEXTENSION_H*/
