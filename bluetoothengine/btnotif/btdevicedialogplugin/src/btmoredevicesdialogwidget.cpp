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

#include "btmoredevicesdialogwidget.h"
#include <hblabel.h>
#include <hblistview.h>
#include <hbtoolbar.h>
#include <hbpushbutton.h>
#include <hblistwidget.h>
#include <qstandarditemmodel.h>
#include <btuidevtypemap.h>
#include <btuiiconutil.h>
#include <bluetoothdevicedialogs.h>



const char* DOCML_BT_MORE_DEV_DIALOG = ":/docml/bt-more-devices-dialog.docml";


BTMoreDevicesDialogWidget::BTMoreDevicesDialogWidget(const QVariantMap &parameters)
    {
    mDeviceDialogData = 0;
    mLoader = 0;
    mContentItemModel = 0;
    constructDialog(parameters);
/*    try 
            {
            //May throws badAlloc on exception
            constructDialog(parameters);
            }
        catch(std::bad_alloc &badAlloc)
            {
            //Failure to allocate memory
            Q_UNUSED(badAlloc);
          //   = UnknownDeviceDialogError;
            }*/
    }

BTMoreDevicesDialogWidget::~BTMoreDevicesDialogWidget()
    {
    delete mLoader;
    delete mContentItemModel;
    }

bool BTMoreDevicesDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
    {
    double cod  = parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceClass)).toDouble();
    int uiMajorDevice;
    int uiMinorDevice;

    BtuiDevProperty::mapDeiveType(uiMajorDevice, uiMinorDevice, cod);
    if ((uiMajorDevice & BtuiDevProperty::Phone)||(uiMajorDevice & BtuiDevProperty::Computer) )
        {
        BtSendDataItem devData;
        devData[NameAliasRole] = QVariant(parameters.value(QString::number(TBluetoothDeviceDialog::EDeviceName)).toString());
        devData[ReadableBdaddrRole] = QVariant(parameters.value(QString::number(TBluetoothDialogParams::EAddress)).toString());
        devData[CoDRole] = QVariant(cod);
        
        setMajorProperty(devData,BtuiDevProperty::Bonded,
                parameters.value("Bonded").toBool());
        setMajorProperty(devData,BtuiDevProperty::Blocked,
                parameters.value("Blocked").toBool());
        setMajorProperty(devData,BtuiDevProperty::Trusted,
                parameters.value("Trusted").toBool());
        setMajorProperty(devData,BtuiDevProperty::Connected,
                parameters.value("Connected").toBool());
        mData.append(devData);
        
        QStandardItem* listitem = new QStandardItem();
        QStringList info;
        info.append(devData[NameAliasRole].toString());

        listitem->setData(info, Qt::DisplayRole);
        HbIcon icon =  getBadgedDeviceTypeIcon(devData[CoDRole].toDouble(),
                devData[MajorPropertyRole].toInt(),
                BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight);        
        listitem->setIcon(icon.qicon());
     //   listitem->setIcon(icon(parameters.value("deviceType").toString()));
        
        mContentItemModel->appendRow(listitem);        
        }
    return true;
    }




int BTMoreDevicesDialogWidget::deviceDialogError() const
    {
    return 0;
    }

void BTMoreDevicesDialogWidget::closeDeviceDialog(bool byClient)
    {
    Q_UNUSED(byClient);
    // below code is required and written based on the documentation of closeDeviceDialog API
    mMoreDeviceDialog->close();
    QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    //below signal is emitted to make dialog server aware that our dialog is closed
    emit deviceDialogClosed();
    }

HbPopup* BTMoreDevicesDialogWidget::deviceDialogWidget() const
    {
    return mMoreDeviceDialog;
    }

QObject* BTMoreDevicesDialogWidget::signalSender() const
    {
    return const_cast<BTMoreDevicesDialogWidget*>(this);
    }

bool BTMoreDevicesDialogWidget::constructDialog(const QVariantMap &/*parameters*/)
    {
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_MORE_DEV_DIALOG, &ok);
    if(ok)
        {
        mMoreDeviceDialog = qobject_cast<HbDialog*>(mLoader->findWidget("lastuseddialog"));
        mMoreDeviceDialog->setFrameType(HbDialog::Strong);
        mMoreDeviceDialog->setBackgroundFaded(false);
/*        HbLabel* label = qobject_cast<HbLabel*>(mLoader->findWidget("label"));
        if(label)
            {
            label->setTextWrapping(Hb::TextWordWrap);
            label->setPlainText("Send to:");
            }
        this->setHeadingWidget(label);*/
   //     HbPushButton* moreDevices = qobject_cast<HbPushButton*>(mLoader->findWidget("moreDevices"));
    //    HbPushButton* cancel = qobject_cast<HbPushButton*>(mLoader->findWidget("cancel"));
        
        HbListView* listView = qobject_cast<HbListView*>(mLoader->findWidget("listView"));
        listView->setSelectionMode(HbAbstractItemView::SingleSelection);

        mContentItemModel = new QStandardItemModel(this);
        listView->setModel(mContentItemModel);//, prototype);
        
 //       QList<QVariant> values = parameters.values();
        
 /*       for(int i=0;i < values.count();i++)
            {
            QStandardItem* listitem = new QStandardItem();
            // parameters.
      //      QString string = values.at(i).toString();
                        
            listitem->setData(values.at(i).toString(), Qt::DisplayRole);
   //         listitem->setData(QString("search"), Qt::DisplayRole);
            //Todo - Insert icons based on the device class        
            QIcon icon(QString(":/qgn_prop_sml_bt.svg"));
            listitem->setIcon(icon);
        
            mContentItemModel->appendRow(listitem);
            }*/
        
        connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(deviceSelected(QModelIndex)));
  //      connect(moreDevices, SIGNAL(clicked()), this, SLOT(moreDevicesClicked()));
   //     connect(cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
        mMoreAction = static_cast<HbAction*>( mLoader->findObject( "moreaction" ) );
        mMoreAction->disconnect(mMoreDeviceDialog);
        
        mCancelAction = static_cast<HbAction*>( mLoader->findObject( "cancelaction" ) );
        mCancelAction->disconnect(mMoreDeviceDialog);
//        QGraphicsWidget *widget = mLoader->findWidget(QString("container"));
  //      mMoreDeviceDialog->setContentWidget(widget);
        
         connect(mMoreAction, SIGNAL(triggered()), this, SLOT(moreDevicesClicked()));
         connect(mCancelAction, SIGNAL(triggered()), this, SLOT(cancelClicked()));
               
        }


    mMoreDeviceDialog->setBackgroundFaded(false);
    mMoreDeviceDialog->setDismissPolicy(HbPopup::NoDismiss);
    mMoreDeviceDialog->setTimeout(HbPopup::NoTimeout);
    mMoreDeviceDialog->setAttribute(Qt::WA_DeleteOnClose);
    return true;
    }

/*void BTMoreDevicesDialogWidget::hideEvent(QHideEvent *event)
    {
    //HbDialog::hideEvent(event);
    mMoreDeviceDialog->hideEvent(event);
//    if(mDeviceDialogData == 0)
        {
        QVariantMap val;
        QVariant index(-1);
        val.insert("selectedindex",index);
        emit deviceDialogData(val);    
        emit deviceDialogClosed();
        }*/    
 //   
   // }

/*void BTMoreDevicesDialogWidget::showEvent(QShowEvent *event)
    {
    //HbDialog::showEvent(event);
    }*/

void BTMoreDevicesDialogWidget::moreDevicesClicked()
    {
    QVariantMap val;
    QVariant index("MoreDevices");
    val.insert("MoreDevices",index);    
    emit deviceDialogData(val);
 //   mDeviceDialogData = 1;//flag is to say that device dialog data is emitted required when we cancel the dialog
   // this->close();
    // TODO
    }

void BTMoreDevicesDialogWidget::cancelClicked()
    {
    // TODO
    mMoreDeviceDialog->close();
    QVariantMap val;
    QVariant index(-1);
    val.insert("selectedindex",index);
    emit deviceDialogData(val);    
    emit deviceDialogClosed();
    }

void BTMoreDevicesDialogWidget::deviceSelected(const QModelIndex& modelIndex)
    {
    int row = modelIndex.row();
    QVariantMap val;
//    QVariant index(row);
    const BtSendDataItem& qtdev = mData.at(row);
    val.insert("selectedindex",QVariant(row));
    val.insert("devicename",QVariant(qtdev[NameAliasRole]));
    val.insert("deviceaddress",QVariant(qtdev[ReadableBdaddrRole]));
    val.insert("deviceclass",QVariant(qtdev[CoDRole]));
    emit deviceDialogData(val);
  //  mDeviceDialogData = 1;//flag is to say that device dialog data is emitted required when we cancel the dialog    
   // this->close();

    }
  
QIcon BTMoreDevicesDialogWidget::icon(/*QString deviceType*/)
    {
 /*   if(deviceType == "Audio")
        {
        return (QIcon(QString(":/qgn_prop_bt_audio.svg")));
        }
    else if(deviceType == "Car-kit")
        {
        return (QIcon(QString(":/qgn_prop_bt_car_kit.svg")));
        }
    else if(deviceType == "Computer")
        {
        return (QIcon(QString(":/qgn_prop_bt_computer.svg")));
        }
    else if(deviceType == "Headset")
        {
        return (QIcon(QString(":/qgn_prop_bt_headset.svg")));
        }
    else if(deviceType == "Keyboard")
        {
        return (QIcon(QString(":/qgn_prop_bt_keyboard.svg")));
        }
    else if(deviceType == "Mouse")
        {
        return (QIcon(QString(":/qgn_prop_bt_mouse.svg")));
        }
    else if(deviceType == "Phone")
        {
        return (QIcon(QString(":/qgn_prop_bt_phone.svg")));
        }
    else if(deviceType == "Printer")
        {
        return (QIcon(QString(":/qgn_prop_bt_printer.svg")));
        }
    else
        {
        return (QIcon(QString(":/qgn_prop_bt_unknown.svg")));
        }*/
    return QIcon(QString(":/icons/qtg_large_bluetooth.svg"));
    }

