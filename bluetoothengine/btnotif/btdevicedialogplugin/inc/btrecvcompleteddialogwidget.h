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
* Description:  BtDeviceDialogWidget class declaration.
*
*/


#ifndef BTRECVCOMPLETEDDIALOGWIDGET_H_
#define BTRECVCOMPLETEDDIALOGWIDGET_H_

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hblabel.h>
#include <hbaction.h>
#include <QRunnable>


class CoversationViewServiceStarter : public QRunnable
    {
public:
    CoversationViewServiceStarter(qint64 conversationId);
    ~CoversationViewServiceStarter();
    void run();
private:
    qint64 mCnvId;
    };


class BTRecvcompletedDialogWidget : public QObject, 
                                public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTRecvcompletedDialogWidget(const QVariantMap &parameters);
    ~BTRecvcompletedDialogWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
public slots:
    void showClicked();
    void cancelClicked();

private:
    bool constructDialog(const QVariantMap &parameters);
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);
    
private:
    HbDocumentLoader    *mLoader;
    QStandardItemModel  *mContentItemModel;
    HbLabel             *mHeading;
    HbLabel             *mFileName;
    HbLabel             *mFileSize;
    HbLabel             *mFileCount;
    HbAction            *mShow;
    HbAction            *mCancel;
    HbDialog            *mDialog;
    int                 mFileSz;
    
    Q_DISABLE_COPY(BTRecvcompletedDialogWidget)
    };



#endif /* BTRECVCOMPLETEDDIALOGWIDGET_H_ */
