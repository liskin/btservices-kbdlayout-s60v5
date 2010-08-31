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
* Description:  BTRecvcompletedDialogWidget class declaration.
*
*/

#include <xqaiwrequest.h>
#include <xqappmgr.h>
#include <QThreadPool>
#include "btrecvcompleteddialogwidget.h"
#include "bluetoothdevicedialogs.h"

const char* DOCML_BT_RECV_CMPLTD_DIALOG = ":/docml/bt-receive-done-dialog.docml";
const qint64 KBluetoothMsgsConversationId = 0x01;


CoversationViewServiceStarter::CoversationViewServiceStarter(qint64 conversationId)
:mCnvId(conversationId)
    {
    
    }

CoversationViewServiceStarter::~CoversationViewServiceStarter()
    {
    
    }

void CoversationViewServiceStarter::run()
    {
    QList<QVariant> args;
    QString serviceName("com.nokia.services.hbserviceprovider");
    QString operation("open(qint64)");
    XQAiwRequest* request;
    XQApplicationManager appManager;
    request = appManager.create(serviceName, "conversationview", operation, false); // not embedded
    if ( request == NULL )
        {
        return;       
        }
    args << QVariant(mCnvId);
    request->setArguments(args);
    request->send();
    delete request;
    }


BTRecvcompletedDialogWidget::BTRecvcompletedDialogWidget(const QVariantMap &parameters)
{
    mLoader = 0;
    constructDialog(parameters);
}

BTRecvcompletedDialogWidget::~BTRecvcompletedDialogWidget()
{
    if(mLoader)
    {
        delete mLoader;
        mLoader = NULL;
    }
}

bool BTRecvcompletedDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
{
    return true;
}

int BTRecvcompletedDialogWidget::deviceDialogError() const
{
    return 0;
}

void BTRecvcompletedDialogWidget::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    mDialog->close();
}

HbPopup* BTRecvcompletedDialogWidget::deviceDialogWidget() const
{
    return mDialog;
}

QObject* BTRecvcompletedDialogWidget::signalSender() const
{
    return const_cast<BTRecvcompletedDialogWidget*>(this);
}

bool BTRecvcompletedDialogWidget::constructDialog(const QVariantMap &parameters)
{
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_RECV_CMPLTD_DIALOG, &ok);
    if(ok)
    {
        mDialog = qobject_cast<HbDialog*>(mLoader->findWidget("recvCompleteddialog"));
        mHeading = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        
        mFileName = qobject_cast<HbLabel*>(mLoader->findWidget("fileName"));
        mFileSize = qobject_cast<HbLabel*>(mLoader->findWidget("fileSize"));
        mFileCount = qobject_cast<HbLabel*>(mLoader->findWidget("fileCount_label"));
        mFileCount->setVisible(false);
        
        //TODO - set icon based on the file icon.
        
        mShow = qobject_cast<HbAction*>(mLoader->findObject("showaction"));
        mCancel = qobject_cast<HbAction*>(mLoader->findObject("cancelaction"));
        
        QString headingStr(hbTrId("txt_bt_title_received_from_1"));
        QString senderName(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
        mHeading->setPlainText(headingStr.arg(senderName));
        mDialog->setHeadingWidget(mHeading);
        
        mFileName->setPlainText(parameters.value(QString::number(TBluetoothDeviceDialog::EReceivingFileName)).toString());
        
        mFileSz = parameters.value(QString::number(TBluetoothDeviceDialog::EReceivingFileSize)).toInt();
        
        //Format the file size into a more readable format
        if ( mFileSz >> 20 )    // size in MB
            {       
            float sizeInMB = 0;
            sizeInMB = ((float)mFileSz ) / (1024*1024);
            QString fileSzMb;
            fileSzMb.setNum(sizeInMB);
            //TODO - check for localization
            fileSzMb.append(QString(" Mb"));
            mFileSize->setPlainText(fileSzMb);
            }
        
        else if( mFileSz >> 10 )        // size in KB
            {
            TInt64 sizeInKB = 0;
            sizeInKB = mFileSz >> 10;
            QString fileSzKb;
            fileSzKb.setNum(sizeInKB);
            //TODO - check for localization
            fileSzKb.append(QString(" Kb"));
            mFileSize->setPlainText(fileSzKb);
            }

        else                              // size is unknown or less than 1K
            {
            QString fileSzB;
            fileSzB.setNum(mFileSz);
            //TODO - check for localization
            fileSzB.append(QString(" Bytes"));
            mFileSize->setPlainText(fileSzB);
            }

        //Set the received file count
        int fCnt = parameters.value(QString::number(TBluetoothDeviceDialog::EReceivedFileCount)).toInt();
        if(fCnt > 1)
            {
            mFileCount->setVisible(true);  
            
            QString fCntStr(hbTrId("txt_bt_info_ln_other_files_received", (fCnt-1)));
            mFileCount->setPlainText(fCntStr);
            }
    }

    mDialog->setBackgroundFaded(false);
    mDialog->setDismissPolicy(HbPopup::NoDismiss);
    mDialog->setTimeout(HbPopup::NoTimeout);
     
    connect(mShow, SIGNAL(triggered()), this, SLOT(showClicked()));
    connect(mCancel, SIGNAL(triggered()), this, SLOT(cancelClicked()));
    
    return true;
}

void BTRecvcompletedDialogWidget::showClicked()
{   
    CoversationViewServiceStarter* service = new CoversationViewServiceStarter(KBluetoothMsgsConversationId);
    service->setAutoDelete(true);
    
    QThreadPool::globalInstance()->start(service);
          
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(TBluetoothDialogParams::EShow));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

void BTRecvcompletedDialogWidget::cancelClicked()
{
    QVariantMap data;
    data.insert(QString("actionResult"), QVariant(TBluetoothDialogParams::ECancelShow));
    emit deviceDialogData(data);
    emit deviceDialogClosed();
}

