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


#ifndef BTDEVICEDIALOGRECVQUERYWIDGET_H
#define BTDEVICEDIALOGRECVQUERYWIDGET_H

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hbcheckbox.h>
#include <hblabel.h>
#include <hbaction.h>


class BTRecvQueryDialogWidget : public QObject, 
                                public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTRecvQueryDialogWidget(const QVariantMap &parameters);
    ~BTRecvQueryDialogWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
public slots:
    void yesClicked();
    void noClicked();
    void checkBoxStateChanged(bool checked);

private:
    bool constructDialog(const QVariantMap &parameters);
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);
    
private:
    HbDocumentLoader    *mLoader;
    QStandardItemModel  *mContentItemModel;
    HbLabel             *mHeading;
    HbLabel             *mDeviceName;
    HbLabel             *mDeviceType;
    HbLabel             *mDeviceIcon;
    HbAction            *mYes;
    HbAction            *mNo;
    HbCheckBox          *mCheckBox;
    HbDialog            *mDialog;
    
    Q_DISABLE_COPY(BTRecvQueryDialogWidget)
    };

#endif /* BTDEVICEDIALOGRECVQUERYWIDGET_H */
