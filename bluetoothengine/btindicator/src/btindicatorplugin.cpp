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

#include "btindicatorplugin.h"
#include "btindicator.h"
#include <hbtranslator.h>

const char* BTINDICATOR_TRANSLATION = "btindimenu";

Q_EXPORT_PLUGIN(BTIndicatorPlugin)

// ----------------------------------------------------------------------------
// BTIndicatorPlugin::BTIndicatorPlugin
// ----------------------------------------------------------------------------
BTIndicatorPlugin::BTIndicatorPlugin() : mError(0),mIndicatorTranslator(0)
{
}

// ----------------------------------------------------------------------------
// BTIndicatorPlugin::~BTIndicatorPlugin
// ----------------------------------------------------------------------------
BTIndicatorPlugin::~BTIndicatorPlugin()
{
    delete mIndicatorTranslator;
}

// ----------------------------------------------------------------------------
// BTIndicatorPlugin::indicatorTypes
// Return notification types this plugin implements
// ----------------------------------------------------------------------------
QStringList BTIndicatorPlugin::indicatorTypes() const
{
  QStringList types; 
  types << "com.nokia.bluetooth.indicatorplugin/1.0";
  return types;
}

// ----------------------------------------------------------------------------
// BTIndicatorPlugin::accessAllowed
// ----------------------------------------------------------------------------
bool BTIndicatorPlugin::accessAllowed(const QString &indicatorType,
        const QVariantMap &securityInfo) const
{
    Q_UNUSED(indicatorType)
    Q_UNUSED(securityInfo)

    // This plugin doesn't perform operations that may compromise security.
    // All clients are allowed to use.
    return true;
}


// ----------------------------------------------------------------------------
// BTIndicatorPlugin::createIndicator
// Creates and returns the HbIndicatorInterface
// ----------------------------------------------------------------------------
HbIndicatorInterface* BTIndicatorPlugin::createIndicator(const QString &indicatorType)
{
    if(!mIndicatorTranslator) {
        mIndicatorTranslator = new HbTranslator(BTINDICATOR_TRANSLATION);
    }
    HbIndicatorInterface *indicator = new BTIndicator(indicatorType);
    return indicator;
}

// ----------------------------------------------------------------------------
// BTIndicatorPlugin::error
// ----------------------------------------------------------------------------
int BTIndicatorPlugin::error() const
{
    return mError;
}

