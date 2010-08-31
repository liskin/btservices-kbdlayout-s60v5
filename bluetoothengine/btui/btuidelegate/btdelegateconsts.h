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

#ifndef BTDELEGATECONSTS_H
#define BTDELEGATECONSTS_H

#include <QObject>

namespace BtDelegate
    {
    enum Command
        {
        Undefined,
        ManagePower,
        DeviceName,
        Visibility,
        Inquiry,
        Connect,
        Pair,
        Disconnect,
        Unpair,
        RemoteDevName
        };
    }

#endif // BTDELEGATECONSTS_H
