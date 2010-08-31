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

#include "btcpplugin.h"
#include <cpsettingformentryitemdataimpl.h>
#include "btcpuisettingitem.h"

/*!
  BtCpPlugin::BtCpPlugin
 */
BtCpPlugin::BtCpPlugin()
{
}

/*!
  BtCpPlugin::~BtCpPlugin
 */
BtCpPlugin::~BtCpPlugin()
{
}

/*!
  BtCpPlugin::createSettingFormItemData
 */
QList<CpSettingFormItemData*> BtCpPlugin::createSettingFormItemData(CpItemDataHelper &itemDataHelper) const
{
    
    return QList<CpSettingFormItemData*> () << new BtCpUiSettingItem(
            itemDataHelper);

}

Q_EXPORT_PLUGIN2(BtCpPlugin, BtCpPlugin);
