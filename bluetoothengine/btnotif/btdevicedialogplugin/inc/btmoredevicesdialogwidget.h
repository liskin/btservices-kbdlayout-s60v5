/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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


#ifndef BTMOREDEVICESDIALOGWIDGET_H
#define BTMOREDEVICESDIALOGWIDGET_H

#include "btdevicedialogutils.h"
#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hbaction.h>

class BTMoreDevicesDialogWidget : public QObject,  public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTMoreDevicesDialogWidget(const QVariantMap &parameters);
    ~BTMoreDevicesDialogWidget();
    
public: // from HbDeviceDialogInterface`
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
public slots:
    void moreDevicesClicked();
    void cancelClicked();
    void deviceSelected(const QModelIndex& modelIndex);
 //   void inputClosed(HbAction *action);
    
private:
    bool constructDialog(const QVariantMap &parameters);
 //   void hideEvent(QHideEvent *event);
 //   void showEvent(QShowEvent *event);
    QIcon icon(/*QString deviceType*/);
 //   void setMajorProperty(
//           BtSendDataItem& qtdev, int prop, bool addto);
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap  data);
    
private:
    HbDocumentLoader *mLoader;
    int              mDeviceDialogData;
    HbDialog *mMoreDeviceDialog;
    HbAction *mMoreAction;
    HbAction *mCancelAction;
    
    /**
     * 
     * item model for content list view.
     */
    
    QStandardItemModel* mContentItemModel;
    
    BtSendDataSource mData;
    
    Q_DISABLE_COPY(BTMoreDevicesDialogWidget)
    };

#endif /* BTMOREDEVICESDIALOGWIDGET_H */
