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
* Description:  BtDeviceDialogPlugin class declaration.
*
*/


#ifndef BTDEVICEDIALOGPLUGIN_P_H
#define BTDEVICEDIALOGPLUGIN_P_H

#include <QObject>
#include <QVariantMap>

#include <hbdevicedialogplugin.h>

// Forward declarations
class BtDeviceDialogPluginPrivate;
class HbTranslator;

/*!
    \class BtDeviceDialogPlugin
    \brief Device dialog widget plugin providing generic input dialog.

    BtDeviceDialogPlugin implements interface HbDeviceDialogPlugin and
    creates device dialog widget, which allows user's input. 
    Currently this plugin implements only one dialog type, identified by
    "com.nokia.hb.btdevicedialog/1.0".
    
    ToDo: create widget base on dialog types when multiple dialog types available.
 */
class BtDeviceDialogPlugin : public HbDeviceDialogPlugin
{
    Q_OBJECT

public:
    friend class BtDeviceDialogPluginPrivate;

    BtDeviceDialogPlugin();
    ~BtDeviceDialogPlugin();
    
    // from base HbDeviceDialogPluginInterface
    virtual HbDeviceDialogInterface *createDeviceDialog(const QString &deviceDialogType,
        const QVariantMap &parameters);
    
    // from base HbDeviceDialogPlugin
    virtual bool accessAllowed(const QString &deviceDialogType,
        const QVariantMap &parameters, const QVariantMap &securityInfo) const;
    virtual bool deviceDialogInfo(const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const;
    virtual QStringList deviceDialogTypes() const;
    virtual PluginFlags pluginFlags() const;
    virtual int error() const;

private: 
    HbDeviceDialogInterface *checkDialogType( const QVariantMap &parameters );
    
private:
    Q_DISABLE_COPY(BtDeviceDialogPlugin)
    BtDeviceDialogPluginPrivate *d;
    HbTranslator* mDialogTranslator;
    HbTranslator* mViewTranslator;
};

#endif // BTDEVICEDIALOGPLUGIN_P_H
