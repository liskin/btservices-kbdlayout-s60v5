/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares main application class.
 *
*/


#ifndef __BTHIDDEVICE_H__
#define __BTHIDDEVICE_H__

#include <e32base.h>
#include <bttypes.h>
#include <btdevice.h>

class CHidDescriptorList;

/*!
 A bluetooth hid device.details
 */

class CBTHidDevice : public CBase
    {
public:
    /*!
     Create a CBTHidDevice object using two phase construction,
     and return a pointer to the created object
     @result Pointer to created CBTHidDevice object
     */
    static CBTHidDevice* NewL();

    /*!
     Create a CBTHidDevice object using two phase construction,
     and return a pointer to the created object, leaving a pointer to the
     object on the cleanup stack.
     @result Pointer to created CBTHidDevice object.
     */
    static CBTHidDevice* NewLC();

    /*!
     Destroy the object and release all memory objects.
     */
    ~CBTHidDevice();

    /*!
     Determine the number of bytes this object will take up when
     externalised to disk
     @return The size required to externalise.
     */
    TInt DiskSize() const;

    /*!
     Externalize data from a stream.
     @param aStream stream object.
     */
    void ExternalizeL(RWriteStream& aStream) const;

    /*!
     Internalize data to a stream.
     @param aStream stream object .
     */
    void InternalizeL(RReadStream& aStream);

private:
    /*!
     Constructs this object.
     */
    CBTHidDevice();

    /*!
     Performs second phase construction of this object
     */
    void ConstructL();

public:
    /*! Bluetooth Address */
    TBTDevAddr iAddress;

    /*! Bluetooth Name */
    TBTDeviceName iDeviceName;

    /*! Bluetooth Class of Device (Cod) */
    TBTDeviceClass iDeviceClass;

    /*! Use security on connection */
    TBool iUseSecurity;

    // HID Service Attributes

    /*! Vendor assigned device release number */
    TUint iDeviceReleaseNumber;

    /*! Device SubClass */
    TUint iDeviceSubClass;

    /*! Device Country Code */
    TUint iCountryCode;

    /*! Indicated if the device supports the virtual cable feature */
    TBool iVirtuallyCabled;

    /*! Device initiates reconnection process */
    TBool iReconnectInit;

    /*! The device BT HID Descriptor List */
    CHidDescriptorList* iDescList;

    /*! Device remains connectable */
    TBool iNormallyConnectable;

    /*! Device Profile Version */
    TUint iProfileVersion;

    //Bluetooth PNP Device Identification

    /*! Vendor ID */
    TUint iVendorID;

    /*! Product ID */
    TUint iProductID;

    };

#endif // __BTHIDDEVICE_H__
