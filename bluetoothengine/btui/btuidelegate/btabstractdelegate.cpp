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


#include "btabstractdelegate.h"
#include "btqtconstants.h"

#include <btsettingmodel.h>
#include <btdevicemodel.h>

/*!
    Constructor.
 */
BtAbstractDelegate::BtAbstractDelegate( BtSettingModel *settingModel, 
        BtDeviceModel *deviceModel, QObject *parent )
    : QObject( parent ), mSettingModel(settingModel),mDeviceModel(deviceModel)
{
}

/*!
    Destructor.
 */
BtAbstractDelegate::~BtAbstractDelegate()
{
}

BtSettingModel *BtAbstractDelegate::getSettingModel()
{
    return mSettingModel;
}

BtDeviceModel *BtAbstractDelegate::getDeviceModel()
{
    return mDeviceModel;
}

void BtAbstractDelegate::cancel()
{
    
}

bool BtAbstractDelegate::isBtPowerOn()
{
    QModelIndex powerIndex = getSettingModel()->index(BtSettingModel::PowerStateRow, 0);
    PowerStateQtValue powerState = (PowerStateQtValue)getSettingModel()->data(powerIndex, BtSettingModel::SettingValueRole).toInt();
    
    return (BtPowerOn == powerState);
}



