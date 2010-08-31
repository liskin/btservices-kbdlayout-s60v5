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


#ifndef BTDEVICESEARCHDIALOGWIDGET_H
#define BTDEVICESEARCHDIALOGWIDGET_H

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hbradiobuttonlist.h>
#include <hblistwidget.h>
#include <hblabel.h>
#include <hbaction.h>
#include <hbselectiondialog.h>
#include "btdevicedialogutils.h"



/*struct device
    {
    QString mDeviceName;
    QString mDeviceType;
    int     mDeviceIdx;
    };*/


class BTDeviceSearchDialogWidget : public QObject,
                                public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTDeviceSearchDialogWidget(const QVariantMap &parameters);
    ~BTDeviceSearchDialogWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
public slots:
    void stopRetryClicked();
    void retryClicked();
    void viewByClicked();
    void deviceSelected(const QModelIndex& modelIndex);
//    void viewByItemSelected(int index);
    void selectionDialogClosed(HbAction *action);
    void searchDialogClosed(); 
    
private:
    bool constructDialog(const QVariantMap &parameters);
    
private:
    enum devTypeSelectionList {
        BtUiDevAudioDevice = 0,
        BtUiDevComputer,
        BtUiDevInputDevice,
        BtUiDevPhone,
        BtUiDevOtherDevice
    };    
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap  data);
    
private:
    HbDocumentLoader *mLoader;
    
    /**
     * 
     * item model for content list view.
     */
    
    QStandardItemModel* mContentItemModel;
    
    HbSelectionDialog *mQuery;
//    QList<QString>      mDeviceTypeList;
    QStringList mDevTypeList;
//    QList<device>       mDeviceList;
    HbListView*         mListView;
    HbLabel*            mSearchLabel; 
    HbLabel*            mSearchIconLabel;
    HbLabel*            mSearchDoneLabel;
//    bool                mViewByChosen;
//    QList<device>       mDeviceLstOfType;
//    int                 mDeviceLstIdx;
    int                 mSelectedDeviceType;
//    int                 mDeviceDialogData;
    HbDialog *mSearchDevicesDialog;
    HbAction *mStopRetryAction;
    HbAction *mShowAction;
    HbAction *mCancelAction;

    int mStopRetryFlag;
    
    BtSendDataSource mData;
    BtSendDataSource mSelectedData;
    
    Q_DISABLE_COPY(BTDeviceSearchDialogWidget)
    };

#endif /* BTDEVICESEARCHDIALOGWIDGET_H */
