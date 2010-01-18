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


#ifndef __BTHIDTYPES_H__
#define __BTHIDTYPES_H__

#include <e32base.h>
#include <btsdp.h>

// HID Profile Service
const TInt KHidPServiceUUID = 0x0011;
// Plug and Play Information
const TInt KPnPServiceUUID = 0x1200;

// Values for the attributes for the HID P Service
const TSdpAttributeID KHIDAttrDevRelNum = 0x0200;
const TSdpAttributeID KHIDAttrDevSubClass = 0x0202;
const TSdpAttributeID KHIDAttrCountryCode = 0x0203;
const TSdpAttributeID KHIDAttrVirtuallyCabled = 0x0204;
const TSdpAttributeID KHIDAttrReconnectInitiate = 0x0205;
const TSdpAttributeID KHIDAttrDescriptorList = 0x0206;
const TSdpAttributeID KHIDAttrProfileVersion = 0x020b;
const TSdpAttributeID KHIDAttrNormallyConnectable = 0x020d;

// Values for the attributes for the PNP Service
const TSdpAttributeID KPnPAttrVendorID = 0x0201;
const TSdpAttributeID KPnPAttrProductID = 0x0202;

const TInt KL2CAPHidControl = 0x0011;
const TInt KL2CAPHidInterrupt = 0x0013;

#endif // __BTHIDTYPES_H__
