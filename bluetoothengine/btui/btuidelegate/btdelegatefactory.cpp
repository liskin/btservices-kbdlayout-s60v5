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


#include "btdelegatefactory.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include "btdelegatepower.h"
#include "btdelegatedevname.h"
#include "btdelegateremotedevname.h"
#include "btdelegatevisibility.h"
#include "btdelegateinquiry.h"
#include "btdelegateconnect.h"
#include "btdelegatepair.h"
#include "btdelegatedisconnect.h"
#include "btdelegatedevsecurity.h"

/*!
    Constructor.
 */
BtAbstractDelegate * BtDelegateFactory::newDelegate(
        BtDelegate::Command cmd,             
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, 
        QObject *parent )
{
    switch ( cmd ) {
        case BtDelegate::ManagePower:
            return new BtDelegatePower( settingModel, deviceModel, parent );
        case BtDelegate::DeviceName:
            return new BtDelegateDevName( parent );
        case BtDelegate::Visibility:
            return new BtDelegateVisibility( parent );
        case BtDelegate::Inquiry:
            return new BtDelegateInquiry( settingModel, deviceModel, parent );
        case BtDelegate::Connect:
            return new BtDelegateConnect( settingModel, deviceModel, parent );
        case BtDelegate::Pair:
            return new BtDelegatePair( settingModel, deviceModel, parent );
        case BtDelegate::Disconnect:
            return new BtDelegateDisconnect( settingModel, deviceModel, parent );
        case BtDelegate::Unpair:
            return new BtDelegateDevSecurity( settingModel, deviceModel, parent );
        case BtDelegate::RemoteDevName:
            return new BtDelegateRemoteDevName( settingModel, deviceModel, parent );
    }
    return 0;
}


