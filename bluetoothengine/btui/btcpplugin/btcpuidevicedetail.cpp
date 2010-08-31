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

#include "btcpuidevicedetail.h"
#include "btcpuideviceview.h"
#include "btcpuidevicedetailsview.h"

#include <btdevsettingpluginloader.h>
#include <bluetoothuitrace.h>
#include <HbInstance>


BtCpUiDeviceDetail::BtCpUiDeviceDetail(QObject *parent) :
    QObject(parent), mDeviceDetailView(0)
{
    mMainWindow = hbInstance->allMainWindows().first();
}

BtCpUiDeviceDetail::~BtCpUiDeviceDetail()
{
    clearDeviceDetailList();
    delete mDeviceDetailView;
}

void BtCpUiDeviceDetail::loadDeviceDetailPlugins(QString deviceAddress, QString deviceName)
{
    QList<BtDevSettingInterface*> interfaces;
    BtAbstractDevSetting *devSetting;
    
    clearDeviceDetailList();
    interfaces = BtDevSettingPluginLoader::loadDevSettingInterfaces();
    QList<BtDevSettingInterface*>::const_iterator i;
    
    for (i = interfaces.constBegin(); i != interfaces.constEnd(); ++i) {
        devSetting = 0;
        devSetting = (*i)->createDevSetting( deviceAddress );
        if(devSetting) {
            appendDeviceToList(devSetting);
        }
    }

    for (int i = interfaces.count() - 1; i >= 0; --i ) {
        delete interfaces.at(i);
    }
    
    createDeviceDetailsView(deviceName);
    notifyDeviceDetailStatus();
    
}

void BtCpUiDeviceDetail::appendDeviceToList(BtAbstractDevSetting *devSetting)
{
    bool ret = false;
    BtDeviceDetails devicedetails;
    devicedetails.mSetting = 0;
    devicedetails.mSettingForm = 0;
    devicedetails.mSettingAvailable = false;
    
    devicedetails.mSetting = devSetting;
    devicedetails.mSettingAvailable = devSetting->isSettingAvailable();
    devicedetails.mSettingForm = devSetting->getSettingWidget();
    
    mDeviceDetailList.append(devicedetails);
    
    ret = connect(devicedetails.mSetting, SIGNAL(settingAvailabilityChanged(BtAbstractDevSetting *, bool)),
            this, SLOT(handleSettingChange(BtAbstractDevSetting *, bool)));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceDetail::appendDeviceToList", "connect settingAvailabilityChanged() failed");

}

void BtCpUiDeviceDetail::handleSettingChange(BtAbstractDevSetting *setting, bool available)
{
    QList<BtDeviceDetails>::iterator i;
       
    for (i = mDeviceDetailList.begin(); i != mDeviceDetailList.end(); ++i) {
        if((setting == (*i).mSetting) && mDeviceDetailView) {
            (*i).mSettingAvailable = available;
            if(available) {
                if(((*i).mSettingForm)) {
                    //If item already exists, remove it first.
                    mDeviceDetailView->removeItem((*i).mSettingForm);
                }
                (*i).mSettingForm = setting->getSettingWidget();
                //add widget
                mDeviceDetailView->addItem((*i).mSettingForm);
            }
            else {
                if((*i).mSettingForm) {
                    //remove widget
                    mDeviceDetailView->removeItem((*i).mSettingForm);
                    (*i).mSettingForm = 0;
                    checkDeviceDetailSettings();
                }
            }
            notifyDeviceDetailStatus();
        }
    }
}


void BtCpUiDeviceDetail::checkDeviceDetailSettings()
{
    QList<BtDeviceDetails>::const_iterator i;
    bool devicedetail = false;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if((*i).mSettingForm) {
            devicedetail = true;
            break;
        }
    }
    //If no setting is available and current view is device detail 
    //view move to previous view.
    if((!devicedetail) && (mMainWindow->currentView() == mDeviceDetailView)) {
        mMainWindow->removeView(mDeviceDetailView); 
        mMainWindow->setCurrentView( mPreviousView );
    }
}

void BtCpUiDeviceDetail::notifyDeviceDetailStatus()
{
    QList<BtDeviceDetails>::const_iterator i;
    bool devicedetail = false;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if(((*i).mSettingAvailable == true) && ((*i).mSettingForm) ) {
            devicedetail = true;
            break;
        }
    }
    emit deviceSettingsChanged(devicedetail);
}

void BtCpUiDeviceDetail::clearDeviceDetailList()
{
    QList<BtDeviceDetails>::const_iterator i;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if((*i).mSetting ) {
            disconnect((*i).mSetting);
            delete (*i).mSetting;
        }
    }
    mDeviceDetailList.clear();
}

void BtCpUiDeviceDetail::createDeviceDetailsView(QString deviceName)
{
    bool ret = false;
    //Launch Device Detail View.
    mDeviceDetailView = new BtCpUiDeviceDetailsView();
    mDeviceDetailView->setDeviceName(deviceName);
    
    ret = connect(mDeviceDetailView, SIGNAL(aboutToClose()), this,
            SLOT(handleDeviceDetailViewClose()));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceDetail::loadDeviceDetailsView", "connect deviceDetailViewClosed() failed");

    QList<BtDeviceDetails>::const_iterator i;
    
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        if((*i).mSettingForm) {
            mDeviceDetailView->addItem((*i).mSettingForm);
        }
    }
}

void BtCpUiDeviceDetail::loadDeviceDetailsView()
{

    mPreviousView = mMainWindow->currentView();
    mMainWindow->addView(mDeviceDetailView);
    
    notifyViewStatusToPlugins(AboutToShow);
    
    mMainWindow->setCurrentView( mDeviceDetailView );
}

void BtCpUiDeviceDetail::handleDeviceDetailViewClose()
{
    notifyViewStatusToPlugins(AboutToHide);
    
    mMainWindow->removeView(mDeviceDetailView);
    
    mMainWindow->setCurrentView( mPreviousView );

}

void BtCpUiDeviceDetail::notifyViewStatusToPlugins(BtCpUiDeviceDetail::NotifyType type)
{
    QList<BtDeviceDetails>::const_iterator i;
    BtAbstractDevSetting *deviceSetting = 0;
    for (i = mDeviceDetailList.constBegin(); i != mDeviceDetailList.constEnd(); ++i) {
        deviceSetting = (*i).mSetting;
        if(deviceSetting) {
            switch(type) {
                case AboutToShow:
                    deviceSetting->aboutToForeground();
                    break;
                case AboutToHide:
                    deviceSetting->aboutToBackground();
                    break;
                case AboutToClose:
                    deviceSetting->aboutToClose();
                    break;
            }
        }
    }

}

void BtCpUiDeviceDetail::sendCloseEvent()
{
    notifyViewStatusToPlugins(AboutToClose);
}


