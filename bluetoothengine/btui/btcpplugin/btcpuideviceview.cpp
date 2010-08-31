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

#include "btcpuideviceview.h"
#include "btuiviewutil.h"
#include <QtGui/QGraphicsLinearLayout>
#include <HbInstance>
#include <hbdocumentloader.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hbgroupbox.h>
#include <hbpushbutton.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hblineedit.h>
#include <hblistview.h>
#include <hbmenu.h>
#include <qstring>
#include <qstringlist>
#include <qdebug>
#include <bluetoothuitrace.h>
#include "btcpuimainview.h"
#include <btabstractdelegate.h>
#include <btdelegatefactory.h>
#include <QModelIndex>
#include "btuiiconutil.h"
#include "btuidevtypemap.h"

#include "btcpuidevicedetail.h"

// docml to load
const char* BTUI_DEVICEVIEW_DOCML = ":/docml/bt-device-view.docml";


BtCpUiDeviceView::BtCpUiDeviceView(
        BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent) :
    BtCpUiBaseView(settingModel,deviceModel,parent),
    mPairedStatus(false), mConnectedStatus(false),mPreviousConnectedStatus(false), mTrustedStatus(false), 
    mBlockedStatus(false), mConnectable(false), mAbstractDelegate(0), mDeviceDetail(0)   
{
    mDeviceIndex = QModelIndex();//is it needed to initialize mIndex???
    
    mMainView = (BtCpUiMainView *) parent;
    
    mMainWindow = hbInstance->allMainWindows().first();
    
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiBaseView::BtCpUiBaseView", "can't create back action");

    // read view info from docml file

    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_device_view");

    mLoader = new HbDocumentLoader();
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);
    
    bool ret = false;

    bool ok = false;
    mLoader->load( BTUI_DEVICEVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file" );
    
    mOrientation = mMainWindow->orientation();
        
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_DEVICEVIEW_DOCML, "landscape", &ok);
        BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape section problem" );
    }
    else {
        mLoader->load(BTUI_DEVICEVIEW_DOCML, "portrait", &ok);
        BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape section problem" );        
    }
    
    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceView::BtCpUiDeviceView()", "connect orientationChanged() failed");

    
    // assign automatically created widgets to local variables
    /*
    mGroupBox = 0;
    mGroupBox = qobject_cast<HbGroupBox *>( mLoader->findWidget( "groupBox_deviceView" ) );
    BTUI_ASSERT_X( mGroupBox != 0, "bt-device-view", "Device groupbox not found" );
    */
    mDeviceIcon=0;
    //can't use qobject_cast since HbIcon is not derived from QObject!
    mDeviceIcon = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceIcon" ) );  
    BTUI_ASSERT_X( mDeviceIcon != 0, "bt-device-view", "Device Icon not found" );
    
    mDeviceName=0;
    mDeviceName = qobject_cast<HbLineEdit *>( mLoader->findWidget( "deviceName" ) );
    BTUI_ASSERT_X( mDeviceName != 0, "bt-device-view", "Device Name not found" );
    ret = connect(mDeviceName, SIGNAL(editingFinished ()), this, SLOT(changeBtDeviceName()));
    
    mDeviceCategory=0;
    mDeviceCategory = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceCategory" ) );  
    BTUI_ASSERT_X( mDeviceCategory != 0, "bt-device-view", "Device Category not found" );
    
    mDeviceStatus=0;
    mDeviceStatus = qobject_cast<HbLabel *>( mLoader->findWidget( "deviceStatus" ) );  
    BTUI_ASSERT_X( mDeviceStatus != 0, "bt-device-view", "Device status not found" );
    
    
    mConnectionCombobox = 0;
    mConnectionCombobox = qobject_cast<HbDataForm *>( mLoader->findWidget( "connectionCombobox" ) );
    BTUI_ASSERT_X( mConnectionCombobox != 0, "bt-device-view", "connection combobox not found" );
    
    mConnectionComboboxModel = new HbDataFormModel();
    
    mPair_Unpair=0;
    mPair_Unpair = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton_0" ) );
    BTUI_ASSERT_X( mPair_Unpair != 0, "bt-device-view", "pair/unpair button not found" );
    ret =  connect(mPair_Unpair, SIGNAL(clicked()), this, SLOT(pairUnpair()));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceView::BtCpUiDeviceView", "can't connect pair button" );
 
    mConnect_Disconnect=0;
    mConnect_Disconnect = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton_1" ) );
    BTUI_ASSERT_X( mConnect_Disconnect != 0, "bt-device-view", "connect/disconnect button not found" );
    ret =  connect(mConnect_Disconnect, SIGNAL(clicked()), this, SLOT(connectDisconnect()));
    BTUI_ASSERT_X( ret, "BtCpUiDeviceView::BtCpUiDeviceView", "can't connect disconnect button" );
      
    mDeviceSetting = 0;
    mDeviceSetting = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton_2" ) );
    BTUI_ASSERT_X( mDeviceSetting != 0, "bt-device-view", "settings button not found" );
    
    
    ret = connect(mDeviceSetting, SIGNAL(clicked()), this,
            SLOT(handleDeviceSetting()));
    BTUI_ASSERT_X( ret, "Btui, BtCpUiDeviceView::BtCpUiDeviceView", "clicked() connect failed");

    setConnectionCombobox();
     
}

BtCpUiDeviceView::~BtCpUiDeviceView()
{
    delete mLoader; // Also deletes all widgets that it constructed.
    
    delete mConnectionComboboxModel;
    
    setNavigationAction(0);
    delete mSoftKeyBackAction;
    if(mAbstractDelegate)
    {
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }

}


void BtCpUiDeviceView::setSoftkeyBack()
{
    if (navigationAction() != mSoftKeyBackAction) {
        setNavigationAction(mSoftKeyBackAction);
        connect( mSoftKeyBackAction, SIGNAL(triggered()), this, SLOT(switchToPreviousView()) );
    }
}

void BtCpUiDeviceView::switchToPreviousView()
{
    BTUI_ASSERT_X(mMainView, "BtCpUiSearchView::switchToPreviousView", "invalid mMainView");
    mMainView->switchToPreviousView();

    if(mDeviceDetail) {
        mDeviceDetail->sendCloseEvent();
    }
    unloadDeviceDetails();
}

void BtCpUiDeviceView::loadDeviceDetails()
{
    bool ret(false);
    
    unloadDeviceDetails();
    
    mDeviceDetail = new BtCpUiDeviceDetail();
    
    ret=connect(mDeviceDetail, SIGNAL(deviceSettingsChanged(bool)),
           this, SLOT(handleDeviceSettingsChange(bool)));
    BTUI_ASSERT_X( ret, "Btui, BtCpUiDeviceView::loadDeviceDetails", "deviceSettingsChanged() connect failed");

    mDeviceDetail->loadDeviceDetailPlugins(mDeviceBdAddr.toString(), mDeviceName->text());
}

void BtCpUiDeviceView::unloadDeviceDetails()
{
    if(mDeviceDetail) {
        disconnect(mDeviceDetail);
        delete mDeviceDetail;
        mDeviceDetail = 0;
    }
}

void BtCpUiDeviceView::activateView( const QVariant& value, bool fromBackButton )
{
    
    Q_UNUSED( fromBackButton );  
    
    mConnectedStatus = false;
    setSoftkeyBack();
    
    
    QModelIndex index = value.value<QModelIndex>();
    mDeviceBdAddr = (mDeviceModel->data(index, BtDeviceModel::ReadableBdaddrRole));
    
    //activate view is called when device is selected
    //clearViewData();
    updateDeviceData();
    
    mDeviceSetting->setVisible(false);
        
    bool ret(false);
    ret=connect(mDeviceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this, SLOT(updateDeviceData()));
    BTUI_ASSERT_X( ret, "Btui, BtCpUiDeviceView::activateView", "dataChanged() connect failed");

}

void BtCpUiDeviceView::handleDeviceSettingsChange(bool status)
{
    mDeviceSetting->setVisible(status);
}

void BtCpUiDeviceView::handleDeviceSetting()
{
    if(mDeviceDetail) {
        mDeviceDetail->loadDeviceDetailsView();
    }
}

void BtCpUiDeviceView::deactivateView()
{
}

// called due to real orientation change event coming from main window
void BtCpUiDeviceView::changeOrientation( Qt::Orientation orientation )
{
    bool ok = false;
    mOrientation = orientation;
    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_DEVICEVIEW_DOCML, "portrait", &ok );
        BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: portrait section problem" );
    } else {
        // load "landscape" section
        mLoader->load( BTUI_DEVICEVIEW_DOCML, "landscape", &ok );
        BTUI_ASSERT_X( ok, "bt-device-view", "Invalid docml file: landscape section problem" );
    }
}

void BtCpUiDeviceView::clearViewData()
{
    mDeviceIcon->clear();
    mDeviceCategory->clear();
    mDeviceStatus->clear();
    mConnectable = false;
    /*
    mPairedStatus = false;
    mConnectedStatus = false;
    mTrustedStatus = false;
    mBlockedStatus = false;
    
    mConnectable = false;
    */
}
    
void BtCpUiDeviceView::updateDeviceData()
{
    clearViewData();
    //Get the QModelIndex of the device using the device BDAddres
    QModelIndex start = mDeviceModel->index(0,0);
    QModelIndexList indexList = mDeviceModel->match(start,BtDeviceModel::ReadableBdaddrRole, mDeviceBdAddr);
    mDeviceIndex = indexList.at(0);
    
    //populate device view with device data fetched from UiModel
    QString deviceName = (mDeviceModel->data(mDeviceIndex, 
             BtDeviceModel::NameAliasRole)).toString(); 
    mDeviceName->setText(deviceName);
     
    int cod = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::CoDRole)).toInt();
     
    int majorRole = (mDeviceModel->data(mDeviceIndex,BtDeviceModel::MajorPropertyRole)).toInt();
    
	setDeviceCategory(cod, majorRole);
    setDeviceStatus(majorRole);
    setTextAndVisibilityOfButtons();
}

void BtCpUiDeviceView::setDeviceCategory(int cod,int majorRole)
{
    mDeviceCategory->setPlainText( getDeviceTypeString( cod ));
    HbIcon icon =
    getBadgedDeviceTypeIcon(cod, majorRole,
                            BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight );
    mDeviceIcon->setIcon(icon);

    if (majorRole & BtuiDevProperty::Connectable ) {
        mConnectable = true;
    } 
}

void BtCpUiDeviceView::setDeviceStatus(int majorRole)
{
    QString deviceStatus;
    
    updateStatusVariables(majorRole);  // should we use bitmap instead??
    
    if (majorRole & BtuiDevProperty::Bonded && 
        majorRole & BtuiDevProperty::Trusted &&
        majorRole & BtuiDevProperty::Connected ) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired_trused_connected"));
    } 
    else if (majorRole & BtuiDevProperty::Bonded && 
             majorRole & BtuiDevProperty::Connected ) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired_connected"));
    }
    else if (majorRole & BtuiDevProperty::Bonded && 
             majorRole & BtuiDevProperty::Trusted ) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired_trusted"));
    } 
    else if (majorRole & BtuiDevProperty::Bonded) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_paired"));
    }
    else if (majorRole & BtuiDevProperty::Connected) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_connected"));
    }
    else if (majorRole & BtuiDevProperty::Blocked) {
        mDeviceStatus->setPlainText(hbTrId("txt_bt_info_blocked"));
    }
    else {
        // device not paired, connected, trusted or blocked.  is this ok?
    }

}

void BtCpUiDeviceView::setConnectionCombobox(){
    
    //create a model class
    
    mConnectionComboboxModel->appendDataFormItem(
    HbDataFormModelItem::ComboBoxItem, QString("Connection"), mConnectionComboboxModel->invisibleRootItem());
    
    //set the model to the view, once model and data class are created
    mConnectionCombobox->setModel(mConnectionComboboxModel);



}


/*!
 *  instead of using separate boolean variables we could use bitmap in single variable
 */
void BtCpUiDeviceView::updateStatusVariables(int majorRole)
{
    mPreviousConnectedStatus = mConnectedStatus;
    
    if (majorRole & BtuiDevProperty::Trusted ) {
        mTrustedStatus = true;
    } 
    else {
        mTrustedStatus = false;
    }
    if (majorRole & BtuiDevProperty::Bonded) {
        mPairedStatus = true;
    }
    else {
        mPairedStatus = false;
    }
    if (majorRole & BtuiDevProperty::Connected) {
        mConnectedStatus = true;
        if (!mPreviousConnectedStatus){
            //Loading device detail plugins after successfull connection.
            loadDeviceDetails();
        }
    }
    else {
        mConnectedStatus = false;
    }
    if (majorRole & BtuiDevProperty::Blocked) {
        mBlockedStatus = true;
    }
    else {
        mBlockedStatus = false;
    }
}

void BtCpUiDeviceView::setTextAndVisibilityOfButtons()
{
    mPair_Unpair->setStretched(true);
    if (mPairedStatus)
    {
        HbIcon icon("qtg_mono_bt_unpair");
        icon.setIconName("qtg_mono_bt_unpair");
        mPair_Unpair->setIcon(icon);
        mPair_Unpair->setText(hbTrId("txt_bt_button_unpair"));
				
    }
    else
    {
        HbIcon icon("qtg_mono_bt_pair");
        icon.setIconName("qtg_mono_bt_pair");
        mPair_Unpair->setIcon(icon);
        mPair_Unpair->setText(hbTrId("txt_bt_button_pair"));
    }
    
    if (mConnectable)
    {
        mConnect_Disconnect->setVisible(true);
        mConnect_Disconnect->setStretched(true);
        if (mConnectedStatus)
        {
            HbIcon icon("qtg_mono_speaker_off");
            icon.setIconName("qtg_mono_speaker_off");
            mConnect_Disconnect->setIcon(icon);
            mConnect_Disconnect->setText(hbTrId("txt_bt_button_disconnect"));
        }
        else
        {
            HbIcon icon("qtg_mono_speaker");
            icon.setIconName("qtg_mono_speaker");
            mConnect_Disconnect->setIcon(icon);
            mConnect_Disconnect->setText(hbTrId("txt_bt_button_connect"));
        }
        
    }
    else
    {
        //it is not possible to connect, set the button invisible
        mConnect_Disconnect->setVisible(false);
    }
    
}


void BtCpUiDeviceView::pairUnpair()
{
    if (mPairedStatus)
    {
        //if the device is paired, call unpairDevice() when the button is tabbed
        unpairDevice();
    }
    else
    {
        //if the device is unpaired, call pairDevice() when the button is tabbed
        pairDevice();

    }
    
    
}

void BtCpUiDeviceView::connectDisconnect()
{
    if (mConnectedStatus)
    {
        //if the device is connected, call disconnectDevice() when the button is tabbed
        disconnectDevice();
    }
    else
    {
        //if the device is disconnected, call connectDevice() when the button is tabbed
        connectDevice();

    }
}

void BtCpUiDeviceView::pairDevice()
{
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Pair, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(pairDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
    
}

void BtCpUiDeviceView::pairDelegateCompleted(int status)
{
    Q_UNUSED(status);
    //TODO: handle the error here
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
}

void BtCpUiDeviceView::unpairDevice()
{
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Unpair, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(unpairDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
        
    
}

void BtCpUiDeviceView::unpairDelegateCompleted(int status)
{
    Q_UNUSED(status);
    //TODO: handle the error here 
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
}

void BtCpUiDeviceView::connectDevice()
{
    
    
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QVariant params;
        params.setValue(mDeviceIndex);
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::Connect, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(connectDelegateCompleted(int)) );
        mAbstractDelegate->exec(params);
    }
    
    
}

void BtCpUiDeviceView::connectDelegateCompleted(int status)
{
    Q_UNUSED(status);
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }   
}

void BtCpUiDeviceView::disconnectDevice()
{
    if (!mAbstractDelegate)//if there is no other delegate running
        { 
           
            
            DisconnectOption discoOpt = ServiceLevel;
                    
            QList<QVariant>list;
            QVariant paramFirst;
            paramFirst.setValue(mDeviceIndex);
            
            QVariant paramSecond;
            paramSecond.setValue((int)discoOpt);
                    
            list.append(paramFirst);
            list.append(paramSecond);
                    
            QVariant params;
            params.setValue(list);
            
            
            //params.setValue(mDeviceIndex);
            mAbstractDelegate = BtDelegateFactory::newDelegate(
                    BtDelegate::Disconnect, mSettingModel, mDeviceModel); 
            connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(disconnectDelegateCompleted(int)) );
            mAbstractDelegate->exec(params);
        }
    
}

void BtCpUiDeviceView::disconnectDelegateCompleted(int status)
{
    Q_UNUSED(status);
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
}

void BtCpUiDeviceView::setPrevBtDeviceName()
{
    QString deviceName = (mDeviceModel->data(mDeviceIndex, 
                BtDeviceModel::NameAliasRole)).toString(); 
   mDeviceName->setText(deviceName);
}

void BtCpUiDeviceView::changeBtDeviceName(){
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        QList<QVariant>list;
        
        QVariant index;
        index.setValue(mDeviceIndex);
        
        QVariant name;
        name.setValue(mDeviceName->text());
        
        list.append(index);
        list.append(name);
        
        QVariant params;
        params.setValue(list);
        
        mAbstractDelegate = BtDelegateFactory::newDelegate(
                BtDelegate::RemoteDevName, mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int, QVariant)), this, SLOT(changeDevNameDelegateCompleted(int, QVariant)) );
        mAbstractDelegate->exec(params);
    }
    else {
        setPrevBtDeviceName();
    }
    
}

void BtCpUiDeviceView::changeDevNameDelegateCompleted(int status, QVariant param)
{
    
    
    if(KErrNone == status) {
        mDeviceName->setText(param.toString());
    }
    else {
        setPrevBtDeviceName();
    }
    //TODO:Error handling has to be done.    
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    
    
}
