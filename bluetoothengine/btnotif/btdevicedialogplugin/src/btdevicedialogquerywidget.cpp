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


#include "btdevicedialogquerywidget.h"
#include "btdevicedialogplugintrace.h"
#include <bluetoothdevicedialogs.h>
#include <hbaction.h>
#include <hbdialog.h>
#include <hblabel.h>
#include "btdevicedialogpluginerrors.h"

/*!
    class Constructor
 */
BtDeviceDialogQueryWidget::BtDeviceDialogQueryWidget(
        HbMessageBox::MessageBoxType type, const QVariantMap &parameters)
{
    TRACE_ENTRY
    // set properties
    mLastError = NoError;
    mShowEventReceived = false;
    mMessageBox = new HbMessageBox(type);
    resetProperties();
    constructQueryDialog(parameters);
    TRACE_EXIT
}

/*!
    Set parameters, implementation of interface
    Invoked when HbDeviceDialog::update calls.
 */
bool BtDeviceDialogQueryWidget::setDeviceDialogParameters(
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
int BtDeviceDialogQueryWidget::deviceDialogError() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mLastError;
}

/*!
    Close notification, implementation of interface
 */ 
void BtDeviceDialogQueryWidget::closeDeviceDialog(bool byClient)
{
    TRACE_ENTRY
    Q_UNUSED(byClient);
    // Closed by client or internally by server -> no action to be transmitted.
    mSendAction = false;
    mMessageBox->close();
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
HbDialog *BtDeviceDialogQueryWidget::deviceDialogWidget() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return mMessageBox;
}

QObject *BtDeviceDialogQueryWidget::signalSender() const
{
    return const_cast<BtDeviceDialogQueryWidget*>(this);
}        

/*!
    Construct display widget
 */
bool BtDeviceDialogQueryWidget::constructQueryDialog(const QVariantMap &parameters)
{
    TRACE_ENTRY
    // analyze the parameters to compose the properties of the message box widget 
    processParam(parameters);
 
    connect(mMessageBox, SIGNAL(finished(HbAction*)), this, SLOT(messageBoxClosed(HbAction*)));
    
    TRACE_EXIT
    return true;
}

/*!
    Take parameter values and generate relevant property of this widget
 */
void BtDeviceDialogQueryWidget::processParam(const QVariantMap &parameters)
{
    TRACE_ENTRY
    QString keyStr, prompt,title;
    QVariant name,param,addval;
    keyStr.setNum( TBluetoothDialogParams::EResource );
    // Validate if the resource item exists.
    QVariantMap::const_iterator i = parameters.constFind( keyStr );
    // item of ResourceId is not found, can't continue.
    if ( i == parameters.constEnd() ) {
        mLastError = UnknownDeviceDialogError;
        return;
    }

    param = parameters.value( keyStr );
    int key = param.toInt();
    switch ( key ) {
        case ENumericComparison:
            prompt = QString( hbTrId( "txt_bt_info_does_this_code_match_the_code_on_1" ) );
            break;
        case EPasskeyDisplay:
            prompt = QString( hbTrId( "txt_bt_info_enter_the_following_code_to_the_1" ) );
            break;
        case EPairingFailureRetry:
            prompt = QString( hbTrId( "txt_bt_info_pairing_with_1_failed_either_the_pas" ) );
            break;
        case EPairingFailureOk:
            prompt = QString( hbTrId( "txt_bt_info_unable_to_pair_with_1" ) );
            break;
        // Input dialogs
        case EPinInput:
        case EObexPasskeyInput:
        // NULL parameters
        case ENoResource:
        case EUnusedResource:
        default:
            mLastError = ParameterError;
            break;
    }
    int repls = prompt.count( QString( "%" ) );
    if ( repls > 0 ) {
        name = parameters.value( QString::number( TBluetoothDeviceDialog::EDeviceName ) );
        prompt = prompt.arg( name.toString() );
        if(key != EPairingFailureRetry && key != EPairingFailureOk)
            {
            addval = parameters.value( QString::number( TBluetoothDeviceDialog::EAdditionalDesc ) );
            // todo: Formating the prompt need to be discussed with UI designer
            // The passcode could be displayed on a separate row if it the label did support
            // the use of newline escape character.
            prompt.append(tr("\n\n")); // insert 2 newlines for clarity
            prompt.append(addval.toString());
            if(key == EPasskeyDisplay)
                {
                prompt.append("\n");   
                }
            }
    }
    // set property value to this dialog widget
    if(key != EPairingFailureRetry && key != EPairingFailureOk)
        {
        title = QString(hbTrId("txt_bt_title_pairing_with_1"));
        title = title.arg(name.toString());
        mMessageBox->setHeadingWidget(new HbLabel(title));
        mMessageBox->setIconVisible(false);
        mMessageBox->setStandardButtons( HbMessageBox::Yes | HbMessageBox::No);
        }
    else
        {
        if(key == EPairingFailureRetry)
            {
            mMessageBox->setStandardButtons( HbMessageBox::Retry | HbMessageBox::Cancel);
            }
        else
            {
            mMessageBox->setStandardButtons( HbMessageBox::Ok);
            }
        }
    mMessageBox->setText( prompt );
    TRACE_EXIT
}

/*!
    Reset properties to default values
 */
void BtDeviceDialogQueryWidget::resetProperties()
{
    TRACE_ENTRY
    // set to default values
    mMessageBox->setModal(true);
    mMessageBox->setTimeout(HbDialog::NoTimeout);
    mMessageBox->setDismissPolicy(HbDialog::NoDismiss);
    mSendAction = true;
    TRACE_EXIT
    return;
}


void BtDeviceDialogQueryWidget::messageBoxClosed(HbAction* action)
{
    QVariantMap data;
    
    HbMessageBox *dlg=static_cast<HbMessageBox*>(sender());
    if(dlg->actions().first() == action) {
        //Yes
        data.insert( QString( "result" ), QVariant(true));
    } 
    else if(dlg->actions().at(1) == action) {
        //No
        data.insert( QString( "result" ), QVariant(false));
    }
    
    emit deviceDialogData(data);
    emit deviceDialogClosed();
    mSendAction = false;
}
