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
* Description:  BtDeviceDialogPlugin class implementation.
*
*/


#include "btdevicedialogplugin.h"
#include "btdevicedialogplugintrace.h"
#include <bluetoothdevicedialogs.h>
#include <QtPlugin>
#include <hbdevicedialog.h>
#include "btdevicedialoginputwidget.h"
#include "btdevicedialogquerywidget.h"
#include "btdevicedialognotifwidget.h"
#include "btrecvprgrsdialogwidget.h"

#include "btdevicedialogpluginerrors.h"
#include "btdevicesearchdialogwidget.h"
#include "btmoredevicesdialogwidget.h"
#include "btsenddialogwidget.h"
#include "btdevicedialogrecvquerywidget.h"
#include "btrecvcompleteddialogwidget.h"
#include <hbtranslator.h>

Q_EXPORT_PLUGIN2(btdevicedialogplugin, BtDeviceDialogPlugin)

const char* BTDIALOG_TRANSLATION = "btdialogs";
const char* BTVIEW_TRANSLATION = "btviews";        

// This plugin implements one device dialog type
static const struct {
    const char *mTypeString;
} noteInfos[] = {
    {"com.nokia.hb.btdevicedialog/1.0"}
};

class BtDeviceDialogPluginPrivate
{
public:
    BtDeviceDialogPluginPrivate();
public:
    int mError;
};
/*! 
    BtDeviceDialogPluginPrivate Constructor
 */
BtDeviceDialogPluginPrivate::BtDeviceDialogPluginPrivate()
{
    mError = NoError;
}

/*! 
    BtDeviceDialogPlugin Constructor
 */
BtDeviceDialogPlugin::BtDeviceDialogPlugin():
 mDialogTranslator(0),mViewTranslator(0)
{
    d = new BtDeviceDialogPluginPrivate;
}

/*!
    Destructor
 */
BtDeviceDialogPlugin::~BtDeviceDialogPlugin()
{
    delete d;
    delete mDialogTranslator;
    delete mViewTranslator;
}

/*! 
    Check if client is allowed to use device dialog widget
 */
bool BtDeviceDialogPlugin::accessAllowed(const QString &deviceDialogType,
    const QVariantMap &parameters, const QVariantMap &securityInfo) const
{
    Q_UNUSED(deviceDialogType)
    Q_UNUSED(parameters)
    Q_UNUSED(securityInfo)

    // This plugin doesn't perform operations that may compromise security.
    // All clients are allowed to use.
    return true;
}

/*! 
    From interface class.
    Use the dialog type in the parameter to create widget.    
 */
HbDeviceDialogInterface *BtDeviceDialogPlugin::createDeviceDialog(
    const QString &deviceDialogType, const QVariantMap &parameters)
{
    d->mError = NoError;

    int i;
    
    if(!mDialogTranslator)
        {
        mDialogTranslator = new HbTranslator(BTDIALOG_TRANSLATION);
        }
    if(!mViewTranslator)
        {
        mViewTranslator = new HbTranslator(BTVIEW_TRANSLATION);
        }
    // verify that requested dialog type is supported
    const int numTypes = sizeof(noteInfos) / sizeof(noteInfos[0]);
    for(i = 0; i < numTypes; i++) {
        if (noteInfos[i].mTypeString == deviceDialogType) {
            break;
        }
    }
    // dialog type was found
    if (i < numTypes) {
        return checkDialogType( parameters );
    } 
    else {
        // unknown dialog type, return error
        d->mError = UnknownDeviceDialogError;
        return 0;
    }
}

/*! 
    Return information of device dialog the plugin creates
    Currently only supporting 1 device dialog type, so no need to check the type.
 */
bool BtDeviceDialogPlugin::deviceDialogInfo(const QString &deviceDialogType,
    const QVariantMap &parameters, DeviceDialogInfo *info) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(deviceDialogType)
    // set return values
    info->group = GenericDeviceDialogGroup;
    info->flags = NoDeviceDialogFlags;
    info->priority = DefaultPriority;
    return true;
}

/*! 
    Return device dialog types this plugin implements
    Function will work fine (unchanged) when new dialogs are added.
 */
QStringList BtDeviceDialogPlugin::deviceDialogTypes() const
{
    QStringList types;
    // read supported types from noteInfos
    const int numTypes = sizeof(noteInfos) / sizeof(noteInfos[0]);
    for(int i = 0; i < numTypes; i++) {
        types.append(noteInfos[i].mTypeString);
    }
    return types;
}

/*! 
    Return plugin flags
 */
HbDeviceDialogPlugin::PluginFlags BtDeviceDialogPlugin::pluginFlags() const
{
    return NoPluginFlags;
}

/*! 
    Return last error
 */
int BtDeviceDialogPlugin::error() const
{
    return d->mError;
}

/*!
    Check the device dialog type to decide which widget to be used. 
    And create the specified widget.
 */
HbDeviceDialogInterface *BtDeviceDialogPlugin::checkDialogType( const QVariantMap &parameters )
{
    // Construct the key of EDialogType
    QString keyStr;
    keyStr.setNum( TBluetoothDialogParams::EDialogType );
    // Find the const iterator with key EDialogType
    QVariantMap::const_iterator i = parameters.constFind( keyStr );

    // item with key EDialogType is not found
    if ( i == parameters.constEnd() ) {
        d->mError = UnknownDeviceDialogError;
        return NULL;
    }
    
    // item with key EDialogType is found
    // generate specified widget based on the dialog type value.
    HbDeviceDialogInterface *deviceDialog = NULL;
    switch ( i.value().toInt() ) {
        case TBluetoothDialogParams::ENote:
            deviceDialog =
                new BtDeviceDialogNotifWidget(parameters);
            break;
        case TBluetoothDialogParams::EQuery:
            deviceDialog =
                new BtDeviceDialogQueryWidget(HbMessageBox::MessageTypeWarning,parameters);
            break;
        case TBluetoothDialogParams::EInput:
            deviceDialog = new BtDeviceDialogInputWidget(parameters);
            break;
        case TBluetoothDialogParams::EMoreDevice:
            deviceDialog = new BTMoreDevicesDialogWidget(parameters);
            break;
        case TBluetoothDialogParams::EDeviceSearch:
            deviceDialog = new BTDeviceSearchDialogWidget(parameters);
            break;
        case TBluetoothDialogParams::ESend:
            deviceDialog = new BTSendDialogWidget(parameters);
            break;
        case TBluetoothDialogParams::EGlobalNotif:
            deviceDialog = new BtDeviceDialogNotifWidget(parameters);
            break;
        case TBluetoothDialogParams::EUserAuthorization:
            deviceDialog = new BTRecvQueryDialogWidget(parameters);
            break;
        case TBluetoothDialogParams::EReceiveProgress:
            deviceDialog = new BTRecvPrgrsDialogWidget(parameters);
            break;
        case TBluetoothDialogParams::EReceiveDone:
            deviceDialog = new BTRecvcompletedDialogWidget(parameters);
            break;
        default:
            d->mError = UnknownDeviceDialogError;
            break;
    }
    if ( deviceDialog ) {
        // verify no error has occurred
        d->mError = deviceDialog->deviceDialogError();
        if ( d->mError ) {
            // Do not continue if an error occurred
            delete deviceDialog;
            deviceDialog = NULL;
        }
        else {
            d->mError = UnknownDeviceDialogError;
        }
    }
    return deviceDialog;
}
