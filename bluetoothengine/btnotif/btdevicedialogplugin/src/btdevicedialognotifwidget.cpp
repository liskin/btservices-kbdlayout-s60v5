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
* Description:  BtDeviceDialogWidget class implementation.
*
*/


#include "btdevicedialognotifwidget.h"
#include "btdevicedialogplugintrace.h"
#include <bluetoothdevicedialogs.h>
#include <hbaction.h>
#include <hbdialog.h>
#include "btdevicedialogpluginerrors.h"
#include <btuiiconutil.h>
/*!
    class Constructor
 */
BtDeviceDialogNotifWidget::BtDeviceDialogNotifWidget( const QVariantMap &parameters )
{
    TRACE_ENTRY
    // set properties
    mLastError = NoError;
    mShowEventReceived = false;
    mNotificationDialog = new HbNotificationDialog();
    resetProperties();
    constructNotifDialog(parameters);
    TRACE_EXIT
}

/*!
    class Constructor
 */
BtDeviceDialogNotifWidget::~BtDeviceDialogNotifWidget()
{
    TRACE_ENTRY
    delete mNotificationDialog;
    return;
}

/*!
    Set parameters, implementation of interface
    Invoked when HbDeviceDialog::update calls.
 */
bool BtDeviceDialogNotifWidget::setDeviceDialogParameters(
    const QVariantMap &parameters)
{
    TRACE_ENTRY
    mLastError = NoError;
    processParam(parameters);
    TRACE_EXIT
    return true;
}

/*!
    Get error, implementation of interface
 */
int BtDeviceDialogNotifWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

/*!
    Close notification, implementation of interface
 */ 
void BtDeviceDialogNotifWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    // Closed by client or internally by server -> no action to be transmitted.
    mSendAction = false;
    mNotificationDialog->close();
     // If show event has been received, close is signalled from hide event. If not,
    // hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
    TRACE_EXIT
}

/*!
    Return display widget, implementation of interface
 */
HbPopup *BtDeviceDialogNotifWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mNotificationDialog;
}

QObject* BtDeviceDialogNotifWidget::signalSender() const
    {
    return const_cast<BtDeviceDialogNotifWidget*>(this);
    }

/*!
    Construct display widget
 */
bool BtDeviceDialogNotifWidget::constructNotifDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    // analyze the parameters to compose the properties of the message box widget 
    processParam(parameters);
    connect(mNotificationDialog, SIGNAL(finished(HbAction*)), this, SLOT(NotifClosed(HbAction*)));
    TRACE_EXIT
    return true;
}

/*!
    Take parameter values and generate relevant property of this widget
 */
void BtDeviceDialogNotifWidget::processParam(const QVariantMap &parameters)
{
    TRACE_ENTRY
    QString keyStr, prompt,title;
    QVariant classOfDevice;
    keyStr.setNum( TBluetoothDialogParams::EResource );
    // Validate if the resource item exists.
    QVariantMap::const_iterator i = parameters.constFind( keyStr );
    // item of ResourceId is not found, can't continue.
    if ( i == parameters.constEnd() ) {
        mLastError = UnknownDeviceDialogError;
        return;
    }
    HbIcon icon;
    QVariant param = parameters.value( keyStr );
    int key = param.toInt();
    switch ( key ) {
        case EPairingSuccess:
            title = QString(hbTrId( "txt_bt_dpophead_paired" ));
            prompt = QString( hbTrId( "txt_bt_dpopinfo_paired_to_1" ) );
            classOfDevice = parameters.value(QString::number( TBluetoothDeviceDialog::EDeviceClass ));
            icon = getBadgedDeviceTypeIcon(classOfDevice.toInt());
            mNotificationDialog->setIcon(icon);
            break;
        // todo: remove this Unpaired notification if not used
        case EUnpairedDevice:
            title = QString(hbTrId( "txt_bt_dpophead_unpaired" ));
            prompt = QString( hbTrId( "txt_bt_dpopinfo_with_1" ) );
            classOfDevice = parameters.value(QString::number( TBluetoothDeviceDialog::EDeviceClass ));
            icon = getBadgedDeviceTypeIcon(classOfDevice.toInt());
            mNotificationDialog->setIcon(icon);
            break;
        case EVisibilityTimeout:
            title = QString(hbTrId( "txt_bt_dpophead_bluetooth" ));
            prompt = QString( hbTrId( "txt_bt_dpopinfo_is_now_hidden" ) );
            mNotificationDialog->setIcon(HbIcon("qtg_large_bluetooth"));
            break;
        default:
            mLastError = ParameterError;
            break;
    }
    int repls = prompt.count( QString( "%" ) );
    if ( repls > 0 ) {
        QVariant name = parameters.value( QString::number( TBluetoothDeviceDialog::EDeviceName ) );
        prompt = prompt.arg( name.toString() );
    }
    mNotificationDialog->setTitle( title );
    mNotificationDialog->setText( prompt );
    TRACE_EXIT
}

/*!
    Reset properties to default values
 */
void BtDeviceDialogNotifWidget::resetProperties()
{
    TRACE_ENTRY
    mSendAction = true;
    TRACE_EXIT
    return;
}

void BtDeviceDialogNotifWidget::NotifClosed(HbAction *action)
{
    Q_UNUSED(action);
    emit deviceDialogClosed();
    mSendAction = false;
}

