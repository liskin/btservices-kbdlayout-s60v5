/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#include "btcpuibaseview.h"
#include <hbaction.h>

/*!
    This constructor constructs new setting and device models.
 */
BtCpUiBaseView::BtCpUiBaseView(QGraphicsItem *parent )
    :CpBaseSettingView( 0 , parent )
{
    mSettingModel = new BtSettingModel(this);
    mDeviceModel = new BtDeviceModel(this);
}

/*!
    This constructor constructs models from the given setting and device models.
    This implies the model impl and data structure is shared.
 */
BtCpUiBaseView::BtCpUiBaseView(
        BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent )
    :CpBaseSettingView( 0 , parent )
{
    mSettingModel = new BtSettingModel(settingModel, this);
    mDeviceModel = new BtDeviceModel(deviceModel, this);    
}

/*!
    Destructor.
 */
BtCpUiBaseView::~BtCpUiBaseView()
{

}




