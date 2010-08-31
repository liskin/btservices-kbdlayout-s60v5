/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BTCPUIDEVICEDETAIL_H
#define BTCPUIDEVICEDETAIL_H

#include <qobject.h>
#include <btdevsettinginterface.h>
#include <HbView>

class BtCpUiDeviceView;
class BtCpUiDeviceDetailsView;

class BtDeviceDetails
{
public:
    BtAbstractDevSetting *mSetting;
    bool mSettingAvailable;
    HbDataForm *mSettingForm;
};


class BtCpUiDeviceDetail : public QObject
{
    Q_OBJECT
    
public:
    explicit BtCpUiDeviceDetail( QObject *parent = 0 );
    
    virtual ~BtCpUiDeviceDetail();
    
    void loadDeviceDetailPlugins(QString deviceAddress, QString deviceName);
        
    void loadDeviceDetailsView();
    
    void sendCloseEvent(); 
    
signals:
    void deviceSettingsChanged(bool settingAvailable);
    
public slots:
    void handleSettingChange(BtAbstractDevSetting *setting, bool available);
    void handleDeviceDetailViewClose();
    
private:
    enum NotifyType {
        AboutToShow,
        AboutToHide,
        AboutToClose,
    };

    void appendDeviceToList(BtAbstractDevSetting *devSetting);
    void clearDeviceDetailList();
    void notifyDeviceDetailStatus();
    void createDeviceDetailsView(QString deviceName);
    void notifyViewStatusToPlugins(BtCpUiDeviceDetail::NotifyType type);
    void checkDeviceDetailSettings();
    
private:
    
    HbMainWindow* mMainWindow;
    QList<BtDeviceDetails> mDeviceDetailList;
    BtCpUiDeviceDetailsView* mDeviceDetailView;
    HbView *mPreviousView;
    
};


#endif /* BTCPUIDEVICEDETAIL_H */
