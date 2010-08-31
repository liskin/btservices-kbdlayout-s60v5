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


#include "btdevicesearchdialogwidget.h"

#include <hblistview.h>
#include <hbtoolbar.h>


#include <qstandarditemmodel.h>
#include <hbaction.h>
#include <xqconversions.h>
#include <qtranslator.h>
#include <qcoreapplication.h>
#include <bluetoothdevicedialogs.h>
#include <btuidevtypemap.h>
#include <btuiiconutil.h>

const char* DOCML_BTDEV_SEARCH_DIALOG = ":/docml/bt-device-search-dialog.docml";


#define LOC_SEARCHING_DEVICE hbTrId("txt_bt_subhead_searching")
#define LOC_SEARCH_DONE hbTrId("txt_bt_subhead_search_done")
#define LOC_SEARCH_STOP hbTrId("txt_common_button_stop")
#define LOC_SEARCH_RETRY hbTrId("txt_common_button_retry")


BTDeviceSearchDialogWidget::BTDeviceSearchDialogWidget(const QVariantMap &parameters)
    {
//    mDeviceLstIdx = 0;
//    mViewByChosen = false;
    mSelectedDeviceType = 0;
 //   mDeviceDialogData = 0;
    mLoader = 0;
    mContentItemModel = 0;
    mStopRetryFlag = 0; // Stop 
    mQuery = 0;
    mSelectedDeviceType |= (BtuiDevProperty::AVDev | BtuiDevProperty::Computer |
            BtuiDevProperty::Phone | BtuiDevProperty::Peripheral |
            BtuiDevProperty::LANAccessDev | BtuiDevProperty::Toy |
            BtuiDevProperty::WearableDev | BtuiDevProperty::ImagingDev |
            BtuiDevProperty::HealthDev | BtuiDevProperty::UncategorizedDev);       
    constructDialog(parameters);
    }

BTDeviceSearchDialogWidget::~BTDeviceSearchDialogWidget()
    {
    delete mLoader;
    delete mContentItemModel;
    if ( mQuery ) {
        delete mQuery;
    }
    
 //   delete mRbl;
 //   delete mViewByDialog;
    }

bool BTDeviceSearchDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
    {
    if(parameters.keys().at(0).compare("Search Completed")==0)
        {
        mStopRetryFlag = 1; // Retry 
        mSearchLabel->hide();
        
        mSearchIconLabel->hide();
        
        mSearchDoneLabel->show();
        mSearchDoneLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchDoneLabel->setAlignment(Qt::AlignLeft);
        mSearchDoneLabel->setPlainText(LOC_SEARCH_DONE);
        
        mStopRetryAction->setText(LOC_SEARCH_RETRY);
        }
    else
        {
        double cod  = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
        int uiMajorDevice;
        int uiMinorDevice;
    
        BtuiDevProperty::mapDeiveType(uiMajorDevice, uiMinorDevice, cod);

        BtSendDataItem devData;
        devData[NameAliasRole] = QVariant(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
        devData[ReadableBdaddrRole] = QVariant(parameters.value(QString::number(TBluetoothDialogParams::EAddress)).toString());
        devData[CoDRole] = QVariant(cod);
        devData[DeviceTypeRole] = QVariant(uiMajorDevice);
        setMajorProperty(devData,BtuiDevProperty::Bonded,
                parameters.value("Bonded").toBool());
        setMajorProperty(devData,BtuiDevProperty::Blocked,
                parameters.value("Blocked").toBool());
        setMajorProperty(devData,BtuiDevProperty::Trusted,
                parameters.value("Trusted").toBool());
        setMajorProperty(devData,BtuiDevProperty::Connected,
                parameters.value("Connected").toBool());
        mData.append(devData);
        
        if(mSelectedDeviceType & devData[DeviceTypeRole].toInt())
            {
            QStandardItem* listitem = new QStandardItem();
            QStringList info;
            info.append(devData[NameAliasRole].toString());
            listitem->setData(info, Qt::DisplayRole);
            HbIcon icon =  getBadgedDeviceTypeIcon(devData[CoDRole].toDouble(),
                    devData[MajorPropertyRole].toInt(),
                    BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight);
            listitem->setIcon(icon.qicon());
            mContentItemModel->appendRow(listitem);    
            mSelectedData.append(devData);
            }
        }
                
    return true;
    }

int BTDeviceSearchDialogWidget::deviceDialogError() const
    {
    return 0;
    }

void BTDeviceSearchDialogWidget::closeDeviceDialog(bool byClient)
    {
    Q_UNUSED(byClient);
    mSearchDevicesDialog->close();
    //@ TODO to check below code is required which is written based on the documentation of closeDeviceDialog API
    
 /*   QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    emit deviceDialogClosed();*/
    }

HbPopup* BTDeviceSearchDialogWidget::deviceDialogWidget() const
    {
    return mSearchDevicesDialog;
    }

QObject* BTDeviceSearchDialogWidget::signalSender() const
    {
    return const_cast<BTDeviceSearchDialogWidget*>(this);
    }

bool BTDeviceSearchDialogWidget::constructDialog(const QVariantMap &parameters)
    {
    (void) parameters;
    bool ok = false;
    
    mLoader = new HbDocumentLoader();
    mLoader->load(DOCML_BTDEV_SEARCH_DIALOG, &ok);
    if(ok)
        {
        mSearchDevicesDialog = qobject_cast<HbDialog*>(mLoader->findWidget("searchdialog"));

 /*       HbLabel* heading = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        heading->setTextWrapping(Hb::TextWordWrap);
        heading->setAlignment(Qt::AlignHCenter);
        heading->setPlainText("Bluetooth - Found devices");
        setHeadingWidget(heading);*/
        
        mSearchLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchLabel"));
        mSearchLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchLabel->setAlignment(Qt::AlignHCenter);
        mSearchLabel->setPlainText(LOC_SEARCHING_DEVICE);
 
        mSearchIconLabel = qobject_cast<HbLabel*>(mLoader->findWidget("iconLabel"));
        mSearchIconLabel->setIcon(QIcon(QString(":/icons/qtg_large_bluetooth.svg")));

        mSearchDoneLabel = qobject_cast<HbLabel*>(mLoader->findWidget("searchDoneLabel"));
        mSearchDoneLabel->hide();
        
        
        mSearchDevicesDialog->setFrameType(HbDialog::Strong);
        mSearchDevicesDialog->setBackgroundFaded(false);

 //       mViewByBtn = qobject_cast<HbPushButton*>(mLoader->findWidget("viewby"));
  //      mStopRetryBtn = qobject_cast<HbPushButton*>(mLoader->findWidget("stop"));
        
        mListView = qobject_cast<HbListView*>(mLoader->findWidget("listView"));
        mListView->setSelectionMode(HbAbstractItemView::SingleSelection);

        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);//, prototype);

        connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
 //       connect(mStopRetryBtn, SIGNAL(clicked()), this, SLOT(stopRetryClicked()));
 //       connect(mViewByBtn, SIGNAL(clicked()), this, SLOT(viewByClicked()));
        
        mShowAction = static_cast<HbAction*>( mLoader->findObject( "viewaction" ) );
        mShowAction->disconnect(mSearchDevicesDialog);
        
        mStopRetryAction = static_cast<HbAction*>( mLoader->findObject( "stopretryaction" ) );
        mStopRetryAction->disconnect(mSearchDevicesDialog);
        
//        mCancelAction = static_cast<HbAction*>( mLoader->findObject( "cancelaction" ) );
//        mCancelAction->disconnect(mSearchDevicesDialog);
        
        connect(mShowAction, SIGNAL(triggered()), this, SLOT(viewByClicked()));
        connect(mStopRetryAction, SIGNAL(triggered()), this, SLOT(stopRetryClicked()));

        connect(mSearchDevicesDialog, SIGNAL(aboutToClose()), this, SLOT(searchDialogClosed()));
        
//        QGraphicsWidget *widget = mLoader->findWidget(QString("container"));
        //setContentWidget(widget);
        }
    mSearchDevicesDialog->setBackgroundFaded(false);
    mSearchDevicesDialog->setDismissPolicy(HbPopup::NoDismiss);
    mSearchDevicesDialog->setTimeout(HbPopup::NoTimeout);
    mSearchDevicesDialog->setAttribute(Qt::WA_DeleteOnClose);
    
    mDevTypeList << hbTrId("txt_bt_list_audio_devices")
            << hbTrId("txt_bt_list_computers") 
            << hbTrId("txt_bt_list_input_devices") 
            << hbTrId("txt_bt_list_phones") 
            << hbTrId("txt_bt_list_other_devices");
    
 /*   mViewByDialog = new HbDialog();
    mRbl = new HbRadioButtonList(mViewByDialog);
    connect(mRbl, SIGNAL(itemSelected(int)), this, SLOT(viewByItemSelected(int)));*/
    
    return true;
    }


void BTDeviceSearchDialogWidget::stopRetryClicked()
    {
    QVariantMap val;
    if(mStopRetryFlag == 1)//mStopRetryAction->text().compare(LOC_SEARCH_RETRY)==0
        {
        mStopRetryFlag = 0; // Stop 
        QVariant index("Retry");
        val.insert("Retry",index); 
        emit deviceDialogData(val);
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);
        mStopRetryAction->setText(LOC_SEARCH_STOP);
        
        mSearchLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchLabel->setAlignment(Qt::AlignHCenter);
        mSearchLabel->setPlainText(LOC_SEARCHING_DEVICE);
        
        mSearchIconLabel->setIcon(QIcon(QString(":/icons/qtg_large_bluetooth.svg")));     
        mSearchLabel->show();
        
        mSearchIconLabel->show();
        
        mSearchDoneLabel->hide();
        }
    else
        {
        mStopRetryFlag = 1; //Retry 
        mStopRetryAction->setText(LOC_SEARCH_RETRY);
        
        mSearchLabel->hide();
        
        mSearchIconLabel->hide();
        
        mSearchDoneLabel->show();
        mSearchDoneLabel->setTextWrapping(Hb::TextWordWrap);
        mSearchDoneLabel->setAlignment(Qt::AlignLeft);
        mSearchDoneLabel->setPlainText(LOC_SEARCH_DONE);        
        
        QVariantMap val;
        QVariant index("Stop");
        val.insert("Stop",index); 
        emit deviceDialogData(val);    
        }
    }

void BTDeviceSearchDialogWidget::retryClicked()
    {
    QVariantMap val;
    QVariant index("Retry");
    val.insert("Retry",index); 
    emit deviceDialogData(val);
    delete mContentItemModel;
    mContentItemModel = new QStandardItemModel(this);
    mListView->setModel(mContentItemModel);
    

    }

void BTDeviceSearchDialogWidget::viewByClicked()
    {
    
    if ( !mQuery ) {
        mQuery = new HbSelectionDialog;
        mQuery->setStringItems(mDevTypeList, 0);
        mQuery->setSelectionMode(HbAbstractItemView::MultiSelection);
    
        QList<QVariant> current;
        current.append(QVariant(0));
        mQuery->setSelectedItems(current);
 
        //todo need to check whether the dialog is destroyed without setting this flag
        //if not destoryed then set this flag in the destructor and then delete it
        
//        mQuery->setAttribute(Qt::WA_DeleteOnClose);
        // Set the heading for the dialog.
        HbLabel *headingLabel = new HbLabel(hbTrId("txt_bt_title_show"), mQuery);
        mQuery->setHeadingWidget(headingLabel);
    }
    mQuery->open(this,SLOT(selectionDialogClosed(HbAction*)));

     
    }

void BTDeviceSearchDialogWidget::searchDialogClosed() 
    {
    QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    emit deviceDialogClosed();
    }

void BTDeviceSearchDialogWidget::selectionDialogClosed(HbAction* action)
    {
    Q_UNUSED(action);
    
    disconnect( mQuery ); 
    int devTypesWanted = 0;

    if (action == mQuery->actions().first()) {  // user pressed "Ok"
        // Get selected items.
        QList<QVariant> selections;
        selections = mQuery->selectedItems();
        
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
                break;
            }
        }
    }
    else
        {
        devTypesWanted = mSelectedDeviceType;
        }
        
    

 /*   HbSelectionDialog *dlg = (HbSelectionDialog*)(sender());
    if(dlg->actions().first() == action) {

     } 
    else if(dlg->actions().at(1) == action) {
     }*/
    
    if((devTypesWanted != mSelectedDeviceType) &&(devTypesWanted !=0))
        {
        mSelectedDeviceType = devTypesWanted;
//        mViewByChosen = true;
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);
        mSelectedData.clear();
        for(int i=0;i<mData.count();i++)
            {
            const BtSendDataItem& qtdev = mData[i];
            if(devTypesWanted & qtdev[DeviceTypeRole].toInt() )
                {
                QStandardItem* listitem = new QStandardItem();
                QStringList info;
                info.append(qtdev[NameAliasRole].toString());
    
                listitem->setData(info, Qt::DisplayRole);
                HbIcon icon =  getBadgedDeviceTypeIcon(qtdev[CoDRole].toDouble(),
                        qtdev[MajorPropertyRole].toInt(),
                         BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight);
                listitem->setIcon(icon.qicon());
                mContentItemModel->appendRow(listitem);        
                mSelectedData.append(qtdev);
                }
            }
        }
    }

void BTDeviceSearchDialogWidget::deviceSelected(const QModelIndex& modelIndex)
    {
    int row = 0;
    
 /*   if(mViewByChosen)
        {
        row = mDeviceLstOfType[modelIndex.row()].mDeviceIdx;
        }
    
    else*/
        {
        row = modelIndex.row();
        }
    
    QVariantMap val;
//    QVariant index(row);
//    val.insert("selectedindex",index);
    
    const BtSendDataItem& qtdev = mSelectedData.at(row);
    val.insert("selectedindex",QVariant(row));
    val.insert("devicename",QVariant(qtdev[NameAliasRole]));
    val.insert("deviceaddress",QVariant(qtdev[ReadableBdaddrRole]));
    val.insert("deviceclass",QVariant(qtdev[CoDRole]));

    emit deviceDialogData(val);
//    mDeviceDialogData = 1;//flag is to say that device dialog data is emitted required when we cancel the dialog
    //emit deviceDialogClosed();
  //  this->close();
    }


