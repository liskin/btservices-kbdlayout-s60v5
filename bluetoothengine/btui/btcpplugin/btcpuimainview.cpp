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
* Description:  BtCpUiMainView implementation
*
*/

#include "btcpuimainview.h"
#include "btuiviewutil.h"
#include <QtGlobal>
#include <QGraphicsLinearLayout>
#include <HbInstance>
#include <hbdocumentloader.h>
#include <hbnotificationdialog.h>
#include <hbgridview.h>
#include <hblistview.h>
#include <hbpushbutton.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hblineedit.h>
#include <hbtooltip.h>
#include <btengconstants.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbcombobox.h>
#include <hbgroupbox.h>
#include <hbdataform.h>
#include "btcpuisearchview.h"
#include "btcpuideviceview.h"
#include <bluetoothuitrace.h>
#include <btdelegatefactory.h>
#include <btabstractdelegate.h>
#include "btqtconstants.h"
#include "btcpuimainlistviewitem.h"
#include "btuidevtypemap.h"

// docml to load
const char* BTUI_MAINVIEW_DOCML = ":/docml/bt-main-view.docml";

/*!
    Constructs a new BtUiMainView using HBDocumentLoader.  Docml (basically xml) file
    has been generated using Application Designer.   

 */
BtCpUiMainView::BtCpUiMainView(        
        BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent )
    : BtCpUiBaseView( settingModel, deviceModel, parent ),
      mAbstractDelegate(0), mMainFilterModel(0)
{
    bool ret(false);

    mMainWindow = hbInstance->allMainWindows().first();
    mMainView = this;
    
    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("view");

    mLoader = new HbDocumentLoader();
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);

    bool ok = false;
    mLoader->load( BTUI_MAINVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file" );
    
    mOrientation = mMainWindow->orientation();
    
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_MAINVIEW_DOCML, "landscape", &ok);
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: landscape section problem" );
    }
    else {
        mLoader->load(BTUI_MAINVIEW_DOCML, "portrait", &ok);
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: landscape section problem" );        
    }

    mDeviceNameEdit=0;
    mDeviceNameEdit = qobject_cast<HbLineEdit *>( mLoader->findWidget( "lineEdit" ) );
    BTUI_ASSERT_X( mDeviceNameEdit != 0, "bt-main-view", "Device Name not found" );
    ret =  connect(mDeviceNameEdit, SIGNAL(editingFinished ()), this, SLOT(changeBtLocalName()));
    
    mPowerButton=0;
    mPowerButton = qobject_cast<HbPushButton *>( mLoader->findWidget( "pushButton" ) );
    BTUI_ASSERT_X( mPowerButton != 0, "bt-main-view", "power button not found" );
    ret =  connect(mPowerButton, SIGNAL(clicked()), this, SLOT(changePowerState()));
    BTUI_ASSERT_X( ret, "BtCpUiMainView::BtCpUiMainView", "can't connect power button" );
    
    mVisibilityMode=0;
    mVisibilityMode = qobject_cast<HbComboBox *>( mLoader->findWidget( "combobox" ) );
    BTUI_ASSERT_X( mVisibilityMode != 0, "bt-main-view", "visibility combobox not found" );
    // add new item for temporary visibility
    // NOTE:  translation (at least default english) gives string "(p)Visible for 5 min", 
    // if setting 1 min --> "(s)Visible for 1 min", ie p=plural, s=singular, but these should
    // not be shown to the user!
    // ToDo:  change this to use translation once it starts working
    QString tempVis(hbTrId("txt_bt_setlabel_visibility_val_visible_for_l1_min", 5));  
    //QString tempVis(hbTrId("Visible for 5 min"));  
    mVisibilityMode->addItem(tempVis, Qt::DisplayRole);
        
    mDeviceList=0;
    mDeviceList = qobject_cast<HbListView *>( mLoader->findWidget( "listView" ) );
    BTUI_ASSERT_X( mDeviceList != 0, "bt-main-view", "Device List (grid view) not found" );   
    
    ret = connect(mDeviceList, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
    BTUI_ASSERT_X( ret, "bt-search-view", "deviceSelected can't connect" ); 
    
    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtCpUiMainView::BtCpUiMainView()", "connect orientationChanged() failed");

    // load tool bar actions
    HbAction *discoverAction = static_cast<HbAction*>( mLoader->findObject( "discoverAction" ) );
    BTUI_ASSERT_X( discoverAction, "bt-main-view", "discover action missing" ); 
    ret = connect(discoverAction, SIGNAL(triggered()), this, SLOT(goToDiscoveryView()));
    BTUI_ASSERT_X( ret, "bt-main-view", "discover action can't connect" ); 

    // load tool bar actions
    mAllAction = static_cast<HbAction*>( mLoader->findObject( "allAction" ) );
    BTUI_ASSERT_X( mAllAction, "bt-main-view", "All action missing" ); 
    ret = connect(mAllAction, SIGNAL(triggered()), this, SLOT(allActionTriggered()));
    BTUI_ASSERT_X( ret, "bt-main-view", "all action can't connect" ); 

    // load tool bar actions
    mPairAction = static_cast<HbAction*>( mLoader->findObject( "pairedAction" ) );
    BTUI_ASSERT_X( mPairAction, "bt-main-view", "Pair action missing" ); 
    ret = connect(mPairAction, SIGNAL(triggered()), this, SLOT(pairActionTriggered()));
    BTUI_ASSERT_X( ret, "bt-main-view", "pair action can't connect" ); 

    mDataForm = qobject_cast<HbDataForm *>( mLoader->findWidget( "dataForm" ) );
    BTUI_ASSERT_X( mDataForm != 0, "bt-main-view", "dataForm not found" ); 
    
        
    // load menu
    HbMenu *optionsMenu = qobject_cast<HbMenu *>(mLoader->findWidget("viewMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "bt-main-view", "Options menu not found" );   
    this->setMenu(optionsMenu);
    
    HbMenu *menu = this->menu();
    mRemovePairedDevices = menu->addAction(hbTrId("txt_bt_opt_remove_paired_devices"));
    
    mSubMenu = new HbMenu(hbTrId("txt_bt_opt_remove"));
    mSubMenu->addAction(hbTrId("txt_bt_opt_remove_sub_all_devices"));
    mSubMenu->addAction(hbTrId("txt_bt_opt_remove_sub_paired_devices"));
    mSubMenu->addAction(hbTrId("txt_bt_opt_remove_sub_blocked_devices"));
    
    // update display when setting data changed
    ret = connect(mSettingModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
            this, SLOT(updateSettingItems(QModelIndex,QModelIndex)));
    BTUI_ASSERT_X( ret, "BtCpUiMainView::BtCpUiMainView", "can't connect dataChanged" );
    
    QModelIndex top = mSettingModel->index( BtSettingModel::LocalBtNameRow, 0 );
    QModelIndex bottom = mSettingModel->index( BtSettingModel::AllowedInOfflineRow, 0 );
    // update name, power and visibility rows
    updateSettingItems( top, bottom );

    //Handle Visibility Change User Interaction
    ret = connect(mVisibilityMode, SIGNAL(currentIndexChanged (int)), 
            this, SLOT(visibilityChanged (int)));
    // create other views
    createViews();
    mCurrentView = this;
    mCurrentViewId = MainView;
    
    mMainFilterModel = new BtuiModelSortFilter(this);
    
    mMainFilterModel->setSourceModel( mDeviceModel );
    mDeviceList->setModel(mMainFilterModel);
    updateDeviceListFilter(BtuiPaired);
	    // List view item
    BtCpUiMainListViewItem *prototype = new BtCpUiMainListViewItem(mDeviceList);
    prototype->setModelSortFilter(mMainFilterModel);
    mDeviceList->setItemPrototype(prototype);

}

/*!
    Destructs the BtCpUiMainView.
 */
BtCpUiMainView::~BtCpUiMainView()
{
    delete mLoader; // Also deletes all widgets that it constructed.
    mMainWindow->removeView(mSearchView);
    mMainWindow->removeView(mDeviceView);
	if (mAbstractDelegate) {
        delete mAbstractDelegate;
    }

}

/*! 
    from base class, initialize the view
 */
void BtCpUiMainView::activateView(const QVariant& value, bool fromBackButton )
{
    Q_UNUSED(value);
    Q_UNUSED(fromBackButton);
    
    //Reset the device list when returning to the view, as it may have been invalidated by the device view
    mMainFilterModel->setSourceModel( mDeviceModel );
    mDeviceList->setModel(mMainFilterModel);
}

/*! 
    From base class. Handle resource before the current view is deactivated.
 */
void BtCpUiMainView::deactivateView()
{

}

void BtCpUiMainView::goToDiscoveryView()
{
    changeView( SearchView, false );
}

void BtCpUiMainView::goToDeviceView(const QModelIndex& modelIndex)
{
    //the QModelIndex of the selected device should be given as parameter here 
    QVariant params;
    params.setValue(modelIndex);
    changeView( DeviceView, false, params );
}

Qt::Orientation BtCpUiMainView::orientation()
{
    return mOrientation;
}

void BtCpUiMainView::changeBtLocalName()
{
    //Error handling has to be done.  
    if (!mAbstractDelegate) {
        mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::DeviceName, 
                mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int,QVariant)), this, SLOT(btNameDelegateCompleted(int,QVariant)) );
        mAbstractDelegate->exec(mDeviceNameEdit->text ());
    }
    else {
        setPrevBtLocalName();
    }
}

void BtCpUiMainView::setPrevBtLocalName()
{
    //ToDo: Should we notify user this as Error...?
    //HbNotificationDialog::launchDialog(hbTrId("Error"));
    QModelIndex index = mSettingModel->index( BtSettingModel::LocalBtNameRow,0 );
    
    mDeviceNameEdit->setText( mSettingModel->data(
            index,BtSettingModel::settingDisplayRole).toString() );
}


void BtCpUiMainView::btNameDelegateCompleted(int status, QVariant param)
{
    if(KErrNone == status) {
        mDeviceNameEdit->setText(param.toString());
    }
    else {
        setPrevBtLocalName();
    }
    //Error handling has to be done.    
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }

}

void BtCpUiMainView::visibilityChanged (int index)
{
    QList<QVariant> list;
    
    VisibilityMode mode = indexToVisibilityMode(index);
    list.append(QVariant((int)mode));
    if( BtTemporary == VisibilityMode(mode) ) {
        //Right now hardcoded to 5 Mins.
        list.append(QVariant(5));
    }
    //Error handling has to be done.    
    if ( !mAbstractDelegate ) {
        mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::Visibility, 
                mSettingModel, mDeviceModel); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(visibilityDelegateCompleted(int)) );
        mAbstractDelegate->exec(list);
    }
    else {
        setPrevVisibilityMode();
    }

}

void BtCpUiMainView::setPrevVisibilityMode()
{
   
    QModelIndex index = mSettingModel->index( BtSettingModel::VisibilityRow, 0 );
    
    mVisibilityMode->setCurrentIndex ( visibilityModeToIndex((VisibilityMode)
                mSettingModel->data(index,BtSettingModel::SettingValueRole).toInt()) );
    
}


void BtCpUiMainView::allActionTriggered()
{
    HbMenu *menu = this->menu();
    menu->removeAction(mRemovePairedDevices);
    mRemoveDevices = menu->addMenu( mSubMenu );

    updateDeviceListFilter(BtuiAll);
}

void BtCpUiMainView::pairActionTriggered()
{
    HbMenu *menu = this->menu();
    menu->removeAction(mRemoveDevices);
    mRemovePairedDevices = menu->addAction(hbTrId("txt_bt_opt_remove_paired_devices"));
    updateDeviceListFilter(BtuiPaired);
}

void BtCpUiMainView::updateDeviceListFilter(BtCpUiMainView::filterType filter)
{
    mMainFilterModel->clearDeviceMajorFilters();
    
    switch (filter) {
        case BtuiAll:
            mDataForm->setHeading(hbTrId("txt_bt_subhead_bluetooth_all_devices"));
            mPairAction->setEnabled(true);
            mAllAction->setEnabled(false);
            mMainFilterModel->addDeviceMajorFilter(
                    BtuiDevProperty::InRegistry, 
                    BtuiModelSortFilter::AtLeastMatch);

            break;
        case BtuiPaired:
            mDataForm->setHeading(hbTrId("txt_bt_subhead_bluetooth_paired_devices"));
            mPairAction->setEnabled(false);
            mAllAction->setEnabled(true);
            mMainFilterModel->addDeviceMajorFilter(
                    BtuiDevProperty::InRegistry | BtuiDevProperty::Bonded, 
                    BtuiModelSortFilter::AtLeastMatch);

            break;
    }
}


void BtCpUiMainView::visibilityDelegateCompleted(int status)
{
    
    //This should be mapped to Qt error
    if(KErrNone != status) {
        setPrevVisibilityMode();
    }
    
    //Error handling has to be done.    
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }

}


// called due to real orientation change event coming from main window
void BtCpUiMainView::changeOrientation( Qt::Orientation orientation )
{
    bool ok = false;
    mOrientation = orientation;
    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_MAINVIEW_DOCML, "portrait", &ok );
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: portrait section problem" );
    } else {
        // load "landscape" section
        mLoader->load( BTUI_MAINVIEW_DOCML, "landscape", &ok );
        BTUI_ASSERT_X( ok, "bt-main-view", "Invalid docml file: landscape section problem" );
    }
}

/*!
    Slot for receiving notification of local setting changes from the model.
    Identify the setting changed and update the corresponding UI item.
 */
void BtCpUiMainView::updateSettingItems(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{   
    bool val(false);
    
    // update only the part of the view specified by the model's row(s)
    for (int i=topLeft.row(); i <= bottomRight.row(); i++) {
        QModelIndex index = mSettingModel->index( i, 0);
        // Distinguish which setting value is changed.
        switch ( i ) {
        case BtSettingModel::LocalBtNameRow :
            mDeviceNameEdit->setText( 
                    mSettingModel->data(index,BtSettingModel::settingDisplayRole).toString() );
            break;
        case BtSettingModel::PowerStateRow:
            val = mSettingModel->data(index, BtSettingModel::SettingValueRole).toBool();
            if (val) {
                HbIcon icon("qtg_mono_bluetooth");
                icon.setIconName("qtg_mono_bluetooth");
                mPowerButton->setIcon(icon);  
            }
            else {
                HbIcon icon("qtg_mono_bluetooth_off");
                icon.setIconName("qtg_mono_bluetooth_off");
                mPowerButton->setIcon(icon);
            }
            break;
        case BtSettingModel::VisibilityRow:
            mVisibilityMode->setCurrentIndex ( visibilityModeToIndex((VisibilityMode)
                    mSettingModel->data(index,BtSettingModel::SettingValueRole).toInt()) );
            break;
        }
    }   
}

/*!
    Slot for receiving notification for user interaction on power state.
    Manually update model data since HbPushButton is not linked to model directly.
 */
void BtCpUiMainView::changePowerState()
{
    QModelIndex index = mSettingModel->index(BtSettingModel::PowerStateRow, 0);
    PowerStateQtValue powerState = (PowerStateQtValue)mSettingModel->data(index, Qt::EditRole).toInt();
    BTUI_ASSERT_X(((powerState == BtPowerOn) || (powerState == BtPowerOff)), 
            "BtCpUiMainView::changePowerState()", "incorrect qt power state");

    if (powerState == BtPowerOff) {
        powerState = BtPowerOn;
    }
    else {
        powerState = BtPowerOff;
    } 
    
    if (!mAbstractDelegate)//if there is no other delegate running
    { 
        mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::ManagePower, 
                mSettingModel, mDeviceModel ); 
        connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(powerDelegateCompleted(int)) );
        mAbstractDelegate->exec(QVariant((int)powerState));
    }
   
}

void BtCpUiMainView::powerDelegateCompleted(int status)
{
    Q_UNUSED(status);
    //ToDo: Error handling here 
    if (mAbstractDelegate)
    {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    //BTUI_ASSERT_X( status, "bt-main-view", "error in delegate complete" );  
}

/*!
 * Mapping from visibility mode UI row to VisibilityMode
 */
VisibilityMode BtCpUiMainView::indexToVisibilityMode(int index)
{
    VisibilityMode mode = BtVisibilityUnknown;
    switch(index) {
    case UiRowBtHidden:  
        mode = BtHidden;
        break;
    case UiRowBtVisible:  
        mode = BtVisible;
        break;
    case UiRowBtTemporary:  
        mode = BtTemporary;
        break;
    default:
        BTUI_ASSERT_X(false, "BtCpUiMainView::indexToVisibilityMode", "invalid mode");
    }
    return mode;
}

/*!
 * Mapping from VisibilityMode to visibility mode UI row  
 */
int BtCpUiMainView::visibilityModeToIndex(VisibilityMode mode)
{
    int uiRow = UiRowBtUnknown;
    switch(mode) {
    case BtHidden:  
        uiRow = UiRowBtHidden;
        break;
    case BtVisible:  
        uiRow = UiRowBtVisible;
        break;
    case BtTemporary:  
        uiRow = UiRowBtTemporary;
        break;
    default:
        BTUI_ASSERT_X(false, "BtCpUiMainView::visibilityModeToIndex", "invalid mode");
    }
    return uiRow;
}



/*!
    Create views(main view, device view and search view).
    Add them to MainWindow.  All views are long-lived and are deleted only when exiting the application 
    (or when main view is deleted).
 */
void BtCpUiMainView::createViews()
{
    Qt::Orientation orientation = mMainWindow->orientation();
    // Create other views
    mSearchView = new BtCpUiSearchView( *mSettingModel, *mDeviceModel, this );
    mMainWindow->addView(mSearchView);
    
    mDeviceView = new BtCpUiDeviceView( *mSettingModel, *mDeviceModel, this );  
    mMainWindow->addView(mDeviceView);
    
    mCurrentView = this;
    mCurrentViewId = MainView;

    
    // QList<int> stores the previous view ids for each view.
    for( int i=0; i < LastView; i++ ) {
        mPreviousViewIds.append( 0 );
    }
}

/*!
    Switch between the views.  
    Parameter "value" is optional except for GadgetView, 
    which needs the QModelIndex of device
 */
void BtCpUiMainView::changeView(int targetViewId, bool fromBackButton, 
        const QVariant& value )
{
    mCurrentView->deactivateView();

    // update the previous view Id in QList<int> 
    // If launching the target view from back softkey, 
    // the previous viewId of target view should not be changed. 
    // Otherwise, loop happens between two views.
    if(!fromBackButton) {
        // normal case:  return to previous view
        mPreviousViewIds[ targetViewId ] = mCurrentViewId;
    }

    // set the new current view 
    mCurrentView = idToView(targetViewId);
    mCurrentViewId = targetViewId;
    mMainWindow->setCurrentView( mCurrentView );

    // do preparation or some actions when new view is activated 
    mCurrentView->activateView( value, fromBackButton );
}
 

void BtCpUiMainView::deviceSelected(const QModelIndex& modelIndex)
{
    QModelIndex index = mMainFilterModel->mapToSource(modelIndex);
    
    QVariant params;
    params.setValue(index);
    
    changeView( DeviceView, false, params );
}

BtCpUiBaseView * BtCpUiMainView::idToView(int targetViewId)
{
    switch (targetViewId) {
    case MainView:
        return mMainView;
    case SearchView:
        return mSearchView;
    case DeviceView:
        return mDeviceView;
    default :
        BTUI_ASSERT_X(false, "BtCpUiMainView::idToView", "invalid view id");
    }
    return 0;
}

void BtCpUiMainView::setSoftkeyBack()
{
    // not needed in main view
}

/*!
   Jump to previous view.  This function is used when back button is pressed.
   semantics slightly different than in other views, since this is called by other
   views when a view switch is needed
 */
void BtCpUiMainView::switchToPreviousView()
{
    BTUI_ASSERT_X( (mCurrentViewId >= 0) && (mCurrentViewId < LastView), 
            "BtCpUiMainView::switchToPreviousView", "invalid view id");
    changeView( mPreviousViewIds[mCurrentViewId], true );
}

