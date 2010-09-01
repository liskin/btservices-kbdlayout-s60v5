/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  Declares Bluetooth notifiers base class.
 *
 */

#ifndef BTNOTIFNAMEUTILS_H
#define BTNOTIFNAMEUTILS_H

#include <btdevice.h>

class BtNotifNameUtils
    {
public:
    static void StripDeviceName(TBTDeviceName& aDeviceName);
    static void GetDeviceDisplayName(TBTDeviceName& aName, const CBTDevice* aDev);
    static void GetDeviceName(TBTDeviceName& aName, const CBTDevice* aDev);
    static void SetDeviceNameL(const TBTDeviceName& aName, CBTDevice& aDev);
    };

#endif // BTNOTIFNAMEUTILS_H


