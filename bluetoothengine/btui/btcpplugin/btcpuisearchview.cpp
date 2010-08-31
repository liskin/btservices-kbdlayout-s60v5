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

#include "btcpuisearchview.h"
#include "btuiviewutil.h"
#include <QtGui/QGraphicsLinearLayout>
#include <HbInstance>
#include <HbDocumentLoader>
#include <HbDataForm>
#include <HbPushButton>
#include <HbLabel>
#include <HbListView>
#include <HbMenu>
#include <HbSelectionDialog>
#include <HbGroupBox>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <bluetoothuitrace.h>
#include "btcpuimainview.h"
#include "btdelegatefactory.h"
#include "btabstractdelegate.h"
#include "btcpuisearchlistviewitem.h"
#include "btuidevtypemap.h"


// docml to load
const char* BTUI_SEARCHVIEW_DOCML = ":/docml/bt-search-view.docml";

BtCpUiSearchView::BtCpUiSearchView(
        BtSettingModel &settingModel, 
        BtDeviceModel &deviceModel, 
        QGraphicsItem *parent) :
    BtCpUiBaseView(settingModel,deviceModel, parent), mAbstractDelegate(0), mBtuiModelSortFilter(0)
{
    bool ret(false);
    
    mQuery = 0;
    mLoader = 0;
    mSoftKeyBackAction = 0;
    
    mMainView = (BtCpUiMainView *) parent;
    mMainWindow = hbInstance->allMainWindows().first();
    
    mSoftKeyBackAction = new HbAction(Hb::BackNaviAction, this);
    BTUI_ASSERT_X(mSoftKeyBackAction, "BtCpUiSearchView::BtCpUiSearchView", "can't create back action");
    
    // Create view for the application.
    // Set the name for the view. The name should be same as the view's
    // name in docml.
    setObjectName("bt_search_view");

    mLoader = new HbDocumentLoader();
    BTUI_ASSERT_X( mLoader != 0, "BtCpUiSearchView::BtCpUiSearchView", "can't create mLoader" );
    // Pass the view to documentloader. Document loader uses this view
    // when docml is parsed, instead of creating new view.
    QObjectList objectList;
    objectList.append(this);
    mLoader->setObjectTree(objectList);
    
    // read view info from docml file
    bool ok = false;
    mLoader->load( BTUI_SEARCHVIEW_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "bt-search-view", "Invalid docml file" );
    
    // Set title for the control panel
    // ToDo:  check if deprecated API
    setTitle("Control Panel");

    // assign automatically created widgets to local variables

    mDeviceIcon=0;
    // can't use qobject_cast since HbIcon is not derived from QObject!
    mDeviceIcon = qobject_cast<HbLabel *>( mLoader->findWidget( "icon" ) );  
    BTUI_ASSERT_X( mDeviceIcon != 0, "BtCpUiSearchView::BtCpUiSearchView", "Device Icon not found" );
    
    mDataForm=0;
    mDataForm = qobject_cast<HbDataForm *>( mLoader->findWidget( "dataForm" ) );
    BTUI_ASSERT_X( mDataForm != 0, "BtCpUiSearchView::BtCpUiSearchView", "dataForm not found" );
    mDataForm->setHeading(hbTrId("txt_bt_subhead_bluetooth_found_devices"));

    mLabelSearching=0;
    mLabelSearching = qobject_cast<HbLabel *>( mLoader->findWidget( "label_searching" ) );
    BTUI_ASSERT_X( mLabelSearching != 0, "BtCpUiSearchView::BtCpUiSearchView", "Searching not found" );
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_searching"));
    
    mDeviceList=0;
    mDeviceList = qobject_cast<HbListView *>( mLoader->findWidget( "deviceList" ) );
    BTUI_ASSERT_X( mDeviceList != 0, "BtCpUiSearchView::BtCpUiSearchView", "Device List not found" );   
    
    
    mDeviceList->setSelectionMode( HbAbstractItemView::SingleSelection );
    
    // read landscape orientation section from docml file if needed
    mOrientation = mMainWindow->orientation();
    
    if (mOrientation == Qt::Horizontal) {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "landscape_ui", &ok);
        BTUI_ASSERT_X( ok, "BtCpUiSearchView::BtCpUiSearchView", "Invalid docml file: landscape section problem" );
    } else {
        mLoader->load(BTUI_SEARCHVIEW_DOCML, "portrait_ui", &ok);
        BTUI_ASSERT_X( ok, "BtCpUiSearchView::BtCpUiSearchView", "Invalid docml file: portrait section problem" );        
    }

    // listen for orientation changes
    ret = connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)),
            this, SLOT(changeOrientation(Qt::Orientation)));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView()", "connect orientationChanged() failed");
    
    // load tool bar actions
    mViewBy = static_cast<HbAction*>( mLoader->findObject( "viewByAction" ) );
    BTUI_ASSERT_X( mViewBy, "BtCpUiSearchView::BtCpUiSearchView", "view by action missing" ); 
    ret = connect(mViewBy, SIGNAL(triggered()), this, SLOT(viewByDeviceTypeDialog()));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "viewByAction can't connect" ); 

    mStop = static_cast<HbAction*>( mLoader->findObject( "stopAction" ) );
    BTUI_ASSERT_X( mStop, "BtCpUiSearchView::BtCpUiSearchView", "stopAction missing" ); 
    ret = connect(mStop, SIGNAL(triggered()), this, SLOT(stopSearching()));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "stopAction can't connect" ); 
    mStop->setEnabled(true);
    
    mRetry = static_cast<HbAction*>( mLoader->findObject( "retryAction" ) );
    BTUI_ASSERT_X( mRetry, "BtCpUiSearchView::BtCpUiSearchView", "retryAction missing" ); 
    ret = connect(mRetry, SIGNAL(triggered()), this, SLOT(retrySearch()));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "retryAction can't connect" ); 
    // Disable for initial search
    mRetry->setEnabled(false);
    
    // load menu
    HbMenu *optionsMenu = qobject_cast<HbMenu *>(mLoader->findWidget("viewMenu"));
    BTUI_ASSERT_X( optionsMenu != 0, "BtCpUiSearchView::BtCpUiSearchView", "Options menu not found" );   
    this->setMenu(optionsMenu);      
    
    ret = connect(mDeviceList, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "deviceSelected can't connect" ); 
    
    // initialize device type list for "view by" option
    // Note:  this list needs to be in the same order as enum devTypeSelectionList
    mDevTypeList << hbTrId("txt_bt_list_audio_devices")
            << hbTrId("txt_bt_list_computers") 
            << hbTrId("txt_bt_list_input_devices") 
            << hbTrId("txt_bt_list_phones") 
            << hbTrId("txt_bt_list_other_devices");
    
    // initialize sort model & create
    // inquiry delegate
    // Sort is set to dynamic sort filter = true in the class, will sort automatically
    mBtuiModelSortFilter = new BtuiModelSortFilter(this);
    mBtuiModelSortFilter->setSourceModel(mDeviceModel);
    mBtuiModelSortFilter->addDeviceMajorFilter(
            BtuiDevProperty::InRange, BtuiModelSortFilter::AtLeastMatch);
    mDeviceList->setModel(mBtuiModelSortFilter);
    // add sort order to show devices in the order they are found
    mBtuiModelSortFilter->setSortRole(BtDeviceModel::SeqNumRole);
    // Sort according to the first column (and the only column) in the listview - sorted according SeqNumRole
    mBtuiModelSortFilter->sort(0, Qt::AscendingOrder);

    BtCpUiSearchListViewItem *prototype = new BtCpUiSearchListViewItem(mDeviceList);
    prototype->setModelSortFilter(mBtuiModelSortFilter);
    mDeviceList->setItemPrototype(prototype);

    // Create the inquiry delegate.
    mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::Inquiry, mSettingModel, mDeviceModel );
    

}

BtCpUiSearchView::~BtCpUiSearchView()
{
    delete mLoader; // Also deletes all widgets that it constructed.
    
    setNavigationAction(0);
    disconnect( mSoftKeyBackAction );
    delete mSoftKeyBackAction;
    
    if(mAbstractDelegate) {
        disconnect(mAbstractDelegate);   
        delete mAbstractDelegate;
    }
    if(mBtuiModelSortFilter) {
        delete mBtuiModelSortFilter;
    }
    if ( mQuery ) {
        delete mQuery;
    }
}

void BtCpUiSearchView::changeOrientation( Qt::Orientation orientation )
{
    // ToDo:  this does not handle redrawing list view items differently for portrait/landscape
    bool ok = false;
    mOrientation = orientation;

    if( orientation == Qt::Vertical ) {
        // load "portrait" section
        mLoader->load( BTUI_SEARCHVIEW_DOCML, "portrait_ui", &ok );
        BTUI_ASSERT_X( ok, "BtCpUiSearchView::changeOrientation", "Invalid docml file: portrait section problem" );
    } else {
        // load "landscape" section
        mLoader->load( BTUI_SEARCHVIEW_DOCML, "landscape_ui", &ok );
        BTUI_ASSERT_X( ok, "BtCpUiSearchView::changeOrientation", "Invalid docml file: landscape section problem" );
    }
}


void BtCpUiSearchView::viewByDeviceTypeDialog()
{
    if ( !mQuery ) {
        mQuery = new HbSelectionDialog;
        mQuery->setStringItems(mDevTypeList, 0);
        mQuery->setSelectionMode(HbAbstractItemView::MultiSelection);
    
        QList<QVariant> current;
        current.append(QVariant(0));
        mQuery->setSelectedItems(current);
    
        // Set the heading for the dialog.
        HbLabel *headingLabel = new HbLabel(hbTrId("txt_bt_title_show"), mQuery);
        mQuery->setHeadingWidget(headingLabel);
    }
    mQuery->open(this,SLOT(viewByDialogClosed(HbAction*)));
}
/*!
   Callback for HbSelectionDialog closing

 */
void BtCpUiSearchView::viewByDialogClosed(HbAction* action)
{
    disconnect( mQuery );  // remove slot
    if (action == mQuery->actions().first()) {  // user pressed "Ok"
        // Get selected items.
        QList<QVariant> selections;
        selections = mQuery->selectedItems();
        
        int devTypesWanted = 0;
        
        for (int i=0; i < selections.count(); i++) {
            switch (selections.at(i).toInt()) {
            case BtUiDevAudioDevice:
                devTypesWanted |= BtuiDevProperty::AVDev;
                break;
            case BtUiDevComputer:
                devTypesWanted |= BtuiDevProperty::Computer;
                break;
            case BtUiDevInputDevice:
                devTypesWanted |= BtuiDevProperty::Peripheral;
                break;
            case BtUiDevPhone:
                devTypesWanted |= BtuiDevProperty::Phone;
                break;
            case BtUiDevOtherDevice:
                devTypesWanted |= (BtuiDevProperty::LANAccessDev |
                        BtuiDevProperty::Toy |
                        BtuiDevProperty::WearableDev |
                        BtuiDevProperty::ImagingDev |
                        BtuiDevProperty::HealthDev |
                        BtuiDevProperty::UncategorizedDev);
                break;
            default:
                // should never get here
                BTUI_ASSERT_X(false, "BtCpUiSearchView::viewByDialogClosed()", 
                        "wrong device type in viewBy dialog!");
            }
        }
        if (devTypesWanted) {
            mBtuiModelSortFilter->clearDeviceMajorFilters();
            mBtuiModelSortFilter->addDeviceMajorFilter(BtuiDevProperty::InRange,
                            BtuiModelSortFilter::AtLeastMatch);   // device must be in range
            mBtuiModelSortFilter->addDeviceMajorFilter(devTypesWanted,
                    BtuiModelSortFilter::RoughMatch);             // device can be any one of selected ones
        }
    }
}

void BtCpUiSearchView::stopSearching()
{
    // Stop searching

    // Stop search delegate
    mAbstractDelegate->cancel();
    // reset view 
    deviceSearchCompleted(KErrNone);      
}

void BtCpUiSearchView::startSearchDelegate ()
{
    bool ret = false;
    
    // Connect to the signal from the BtDelegateInquiry for completion of the search request
    ret = connect(mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(searchDelegateCompleted(int)));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "searchDelegateCompleted can't connect" );
    
    // Connect to the signal from the BtuiModel when the search has been completed.
    ret = connect(mDeviceModel, SIGNAL(deviceSearchCompleted(int)), this, SLOT(deviceSearchCompleted(int)));
    BTUI_ASSERT_X( ret, "BtCpUiSearchView::BtCpUiSearchView", "deviceSearchCompleted can't connect" );     

    mAbstractDelegate->exec(QVariant());
}

void BtCpUiSearchView::retrySearch()
{
    // Retry search

    // Change label and buttons to reflect status
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_searching"));
    mRetry->setEnabled(false);
    mStop->setEnabled(true);
    
    // Make use of the delegate and start the search.
    startSearchDelegate ();
}

void BtCpUiSearchView::setSoftkeyBack()
{
    if (navigationAction() != mSoftKeyBackAction) {
        setNavigationAction(mSoftKeyBackAction);
        connect( mSoftKeyBackAction, SIGNAL(triggered()), this, SLOT(switchToPreviousView()) );
    }
}

void BtCpUiSearchView::switchToPreviousView()
{
    BTUI_ASSERT_X(mMainView, "BtCpUiSearchView::switchToPreviousView", "invalid mMainView");
    mMainView->switchToPreviousView();
}

void BtCpUiSearchView::activateView( const QVariant& value, bool fromBackButton )
{
    Q_UNUSED(value);

    // ToDo:  consider whether orientation should be updated here rather than when orientation really changes;
    // advantage:  if orientation changes temporarily in another view, but returns to previous setting before
    // search is reactived, there is no processing overhead
    
    setSoftkeyBack();
            
    if ( !fromBackButton ) {
        // Sets the label and toolbar buttons
        retrySearch();
    }
}

void BtCpUiSearchView::deactivateView()
{
    mAbstractDelegate->cancel();          // Stop searching when leaving view
    deviceSearchCompleted(KErrNone);      // reset view 
}

void BtCpUiSearchView::searchDelegateCompleted(int error)
{
    Q_UNUSED(error);
    
}

void BtCpUiSearchView::deviceSearchCompleted(int error)
{
    //TODO - handle error.
    Q_UNUSED(error);
    
    mLabelSearching->setPlainText(hbTrId("txt_bt_subhead_search_done"));
    mRetry->setEnabled(true);
    mStop->setEnabled(false);
    
    // disconnect signals
    disconnect(mAbstractDelegate);
    disconnect(mDeviceModel);
}

void BtCpUiSearchView::deviceSelected(const QModelIndex& modelIndex)
{
    QModelIndex index = mBtuiModelSortFilter->mapToSource(modelIndex);
    static_cast<BtCpUiMainView*>(mMainView)->goToDeviceView(index);
}
