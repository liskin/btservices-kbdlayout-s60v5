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
* Description:  This is the implementation of application class
 *
*/


#include "bthiddevice.h"
#include "hiddescriptorlist.h"

// A version number for this object when externalised.
// Only for future proofing.
const TInt KVersionNumber = 1;

CBTHidDevice* CBTHidDevice::NewL()
    {
    CBTHidDevice* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CBTHidDevice* CBTHidDevice::NewLC()
    {
    CBTHidDevice* self = new (ELeave) CBTHidDevice();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CBTHidDevice::CBTHidDevice()
    {
    }

CBTHidDevice::~CBTHidDevice()
    {
    delete iDescList;
    }

TInt CBTHidDevice::DiskSize() const
    {
    // Always using 10 bytes as an estimate for Series 60 Descriptor headers

    //4 for KVersionNumber
    //6 + 10 for the iAddress
    //4 for iDeviceReleaseNumber
    //4 for iDeviceSubClass
    //4 for iCountryCode
    //1 for iVirtuallyCabled
    //1 for iReconnectInit
    //1 for iNormallyConnectable
    //4 for iProfileVersion
    //4 for iVendorID
    //4 for iProductID
    //1 for iUseSecurity
    //256 + 10 for iDeviceName
    //4 for iDeviceClass
    TInt size = 318;

    // Now add the size required for the descriptor list
    size += iDescList->DiskSize();

    return size;
    }

void CBTHidDevice::ExternalizeL(RWriteStream& aStream) const
    {
    // NOTE!! When changing this function, also check DiskSize

    // Write the version number
    aStream.WriteInt32L(KVersionNumber);

    // Write all member variables to the stream.
    // Leave if we get any error
    aStream.WriteL(iAddress.Des());
    aStream.WriteUint32L(iDeviceReleaseNumber);
    aStream.WriteUint32L(iDeviceSubClass);
    aStream.WriteUint32L(iCountryCode);
    aStream.WriteInt8L(iVirtuallyCabled ? 1 : 0);
    aStream.WriteInt8L(iReconnectInit ? 1 : 0);
    aStream.WriteInt8L(iNormallyConnectable ? 1 : 0);
    aStream.WriteUint32L(iProfileVersion);
    aStream.WriteUint32L(iVendorID);
    aStream.WriteUint32L(iProductID);
    aStream.WriteInt8L(iUseSecurity ? 1 : 0);
    aStream << iDeviceName;
    aStream << iDeviceClass;
    aStream << *iDescList;
    }

void CBTHidDevice::InternalizeL(RReadStream& aStream)
    {
    // Read the version number
    // Ignored in this version.
    aStream.ReadInt32L();

    // Read all member variables from the stream
    // Leave if we get any error

    // Get a pointer to the address to fill
    TPtr8 addr = iAddress.Des();
    // Read the address in
    aStream.ReadL(addr);

    iDeviceReleaseNumber = aStream.ReadUint32L();
    iDeviceSubClass = aStream.ReadUint32L();
    iCountryCode = aStream.ReadUint32L();
    iVirtuallyCabled = aStream.ReadInt8L();
    iReconnectInit = aStream.ReadInt8L();
    iNormallyConnectable = aStream.ReadInt8L();
    iProfileVersion = aStream.ReadUint32L();
    iVendorID = aStream.ReadUint32L();
    iProductID = aStream.ReadUint32L();
    iUseSecurity = aStream.ReadInt8L();
    aStream >> iDeviceName;
    aStream >> iDeviceClass;
    aStream >> *iDescList;
    }

void CBTHidDevice::ConstructL()
    {
    iDescList = new (ELeave) CHidDescriptorList;
    }

// End of File
