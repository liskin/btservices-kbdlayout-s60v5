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
* Description:  Internal symbolic constants of this component
*
*/

#ifndef BTSERVICEUTILCONSTS_H
#define BTSERVICEUTILCONSTS_H

#include <e32base.h>

namespace BtServiceUtil {

const TInt KServiceRequestIdBase = 0x100;

enum TRequestIdentifier
    {
    // Inquiry with Host Resolver
    EBluetoothInquiry = KServiceRequestIdBase,
    // Get the name of a device
    EBluetoothPageDeviceName,
    // Schedule the notification of search completion
    EAsyncNotifyDeviceSearchCompleted,
    // Create view table of remote devices from BT registry
    ECreateRemoteDeviceViewRequest,
    // Retrieves remote devices from BT registry
    EGetRemoteDevicesRequest,
    // Subsribes the PubSub key for BT registry update events
    ERegistryPubSubWatcher
    };
}

#endif

// End of File
