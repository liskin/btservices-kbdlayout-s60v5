/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is used by BTUI to contain the information on a displaid device.
*
*/

#include "btdevmodel.h"
// ---------------------------------------------------------------------
// Default Constructor
// ---------------------------------------------------------------------
EXPORT_C TBTDevice::TBTDevice()
    {
    iType = EDeviceDefault;
    iName = KNullDesC;
    iFriendlyName = KNullDesC;
    iStatus = 0;
    iOperation = EOpNone;
    iAddr = KNullAddress;
    iIndex = KErrNotFound;    
    iDeviceClass=TBTDeviceClass(0);
    iLinkKeyType = ELinkKeyCombination;
    iNameEntry() = TNameRecord();
    }
// ---------------------------------------------------------------------
// Copy Constructor
// ---------------------------------------------------------------------    
EXPORT_C TBTDevice::TBTDevice(const TBTDevice& aDevice) 
    {
    iType = aDevice.iType;
    iName = aDevice.iName;
    iFriendlyName = aDevice.iFriendlyName;
    iStatus = aDevice.iStatus;
    iOperation = aDevice.iOperation;
    iAddr = aDevice.iAddr;
    iIndex = aDevice.iIndex;
    iDeviceClass= aDevice.iDeviceClass;
    iLinkKeyType = aDevice.iLinkKeyType;
    iNameEntry() = TNameRecord();
    }
// ---------------------------------------------------------------------
// operator=
// ---------------------------------------------------------------------    
EXPORT_C TBTDevice& TBTDevice::operator=(const TBTDevice& aDevice)
    {
    iType = aDevice.iType;
    iName = aDevice.iName;
    iFriendlyName = aDevice.iFriendlyName;
    iStatus = aDevice.iStatus;
    iOperation = aDevice.iOperation;
    iAddr = aDevice.iAddr;
    iIndex = aDevice.iIndex;
    iDeviceClass= aDevice.iDeviceClass;    
    iLinkKeyType = aDevice.iLinkKeyType;
    iNameEntry = aDevice.iNameEntry;
	return *this;
    } 
// End of File
