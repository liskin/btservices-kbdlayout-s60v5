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
* Description:  BtDeviceDialogWidget class declaration.
*
*/


#include "btdevicedialogrecvquerywidget.h"
#include "bluetoothdevicedialogs.h"
#include <btuiiconutil.h>

const char* DOCML_BT_RECV_QUERY_DIALOG = ":/docml/bt-receive-auth-dialog.docml";


BTRecvQueryDialogWidget::BTRecvQueryDialogWidget(const QVariantMap &parameters)
{
    mLoader = 0;
    constructDialog(parameters);
}

BTRecvQueryDialogWidget::~BTRecvQueryDialogWidget()
{
    if(mLoader)
    {
        delete mLoader;
        mLoader = NULL;
    }
}

bool BTRecvQueryDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    Q_UNUSED(parameters);
    return true;
}

int BTRecvQueryDialogWidget::deviceDialogError() const
{
    return 0;
}

void BTRecvQueryDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mDialog->close();
}

HbPopup* BTRecvQueryDialogWidget::deviceDialogWidget() const
{
    return mDialog;
}

QObject* BTRecvQueryDialogWidget::signalSender() const
{
    return const_cast<BTRecvQueryDialogWidget*>(this);
}

bool BTRecvQueryDialogWidget::constructDialog(const QVariantMap &parameters)
{
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_RECV_QUERY_DIALOG, &ok);
    if(ok)
    {
        mDialog = qobject_cast<HbDialog*>(mLoader->findWidget("senddialog"));
        mHeading = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        
        mDeviceName = qobject_cast<HbLabel*>(mLoader->findWidget("deviceName"));
        mDeviceType = qobject_cast<HbLabel*>(mLoader->findWidget("deviceType"));
        mDeviceIcon = qobject_cast<HbLabel*>(mLoader->findWidget("deviceIcon"));
        
        int classOfDevice = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
        HbIcon icon = getBadgedDeviceTypeIcon(classOfDevice);
        mDeviceIcon->setIcon(icon);
                
        mDeviceName->setPlainText(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
        mDeviceType->setPlainText(getDeviceTypeString(classOfDevice));
        
        mYes = qobject_cast<HbAction*>(mLoader->findObject("yesaction"));
        mNo = qobject_cast<HbAction*>(mLoader->findObject("noaction"));
        
        mCheckBox = qobject_cast<HbCheckBox*>(mLoader->findWidget("checkbox"));

        int dialogType = parameters.value(QString::number(TBluetoothDialogParams::EDialogTitle)).toInt();
        switch(dialogType)
        {
            case TBluetoothDialogParams::EReceive:
            {
                mHeading->setPlainText(hbTrId("txt_bt_title_receive_messages_from"));
            }break;
                
            case TBluetoothDialogParams::EReceiveFromPairedDevice:
            {
                mHeading->setPlainText(hbTrId("txt_bt_title_receive_messages_from_paired_device"));
                mCheckBox->setCheckState(Qt::Checked);
            }break;
            
            case TBluetoothDialogParams::EConnect:
            {
                mHeading->setPlainText(hbTrId("txt_bt_title_connect_to"));
                mCheckBox->setCheckState(Qt::Checked);
            }break;
            case TBluetoothDialogParams::EPairingRequest:
                mHeading->setPlainText(hbTrId("txt_bt_title_pair_with"));
                mCheckBox->setCheckState(Qt::Checked);                
                break;
            default:
                break;

        }
        mDialog->setHeadingWidget(mHeading);
    }

    mDialog->setBackgroundFaded(false);
    mDialog->setDismissPolicy(HbPopup::NoDismiss);
    mDialog->setTimeout(HbPopup::NoTimeout);
     
    connect(mYes, SIGNAL(triggered()), this, SLOT(yesClicked()));
    connect(mNo, SIGNAL(triggered()), this, SLOT(noClicked()));
    connect(mCheckBox, SIGNAL(clicked(bool)), this, SLOT(checkBoxStateChanged(bool)));
    
    return true;
}

void BTRecvQueryDialogWidget::yesClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(true));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvQueryDialogWidget::noClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(false));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvQueryDialogWidget::checkBoxStateChanged(bool checked)
{
    QVariantMap data;
    data.insert(QString("checkBoxState"), QVariant(checked));
    emit deviceDialogData(data);
}



