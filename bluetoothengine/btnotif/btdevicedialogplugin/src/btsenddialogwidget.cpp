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

#include "btsenddialogwidget.h"
#include <hblabel.h>
#include <hblistview.h>
#include <hbtoolbar.h>
#include <hblistwidget.h>
#include <qstandarditemmodel.h>
#include <hbaction.h>


#define LOC_SENDING_FILES_TO_DEVICE hbTrId("txt_bt_title_sending_file_l1l2_to_3")

const char* DOCML_BT_SEND_DIALOG = ":/docml/bt-send-dialog.docml";

BTSendDialogWidget::BTSendDialogWidget(const QVariantMap &parameters)
    {
    mLoader = 0;
    constructDialog(parameters);
    }

BTSendDialogWidget::~BTSendDialogWidget()
    {
    delete mLoader;
 /*   if(mContentItemModel)
        {
        delete mContentItemModel;
        mContentItemModel = NULL;
        }*/
    }

bool BTSendDialogWidget::setDeviceDialogParameters(const QVariantMap &parameters)
    {
    if(mFileIndex != parameters.value("currentFileIdx").toString().toInt() )
        {
        mDialogHeading->setTextWrapping(Hb::TextWordWrap);
        mDialogHeading->setAlignment(Qt::AlignHCenter);
        //Todo - replace this with the actual text from parameters
    
        QString headLabel = QString(LOC_SENDING_FILES_TO_DEVICE).arg(parameters.value("currentFileIdx").toInt())
                                        .arg(parameters.value("totalFilesCnt").toInt())
                                        .arg(parameters.value("destinationName").toString());
      /*  headLabel.append(QString("Sending file "));
        headLabel.append(parameters.value("currentFileIdx").toString());
        headLabel.append('/');
        headLabel.append(parameters.value("totalFilesCnt").toString());
        headLabel.append(QString(" to "));
        headLabel.append(parameters.value("destinationName").toString());*/
        mDialogHeading->setPlainText(headLabel);
        
 /*       QStringList info;
        info.append(parameters.value("fileName").toString());
        info.append(parameters.value("fileSzTxt").toString());
                    
        QStandardItem* listitem = new QStandardItem();
        // parameters.
        listitem->setData(info, Qt::DisplayRole);
    
        //Todo - Insert file icons here thumbnail icon        
        QIcon icon(QString(":/icons/qtg_large_bluetooth.svg"));
        listitem->setIcon(icon);
    
        delete mContentItemModel;
        mContentItemModel = new QStandardItemModel(this);
        mListView->setModel(mContentItemModel);//, prototype);
    
        mContentItemModel->appendRow(listitem);*/
        //Todo - Insert file icons here thumbnail icon        
        QIcon icon(QString(":/icons/qtg_large_bluetooth.svg"));        
        mFileIconLabel->setIcon(icon);
        mFileNameLabel->setPlainText(parameters.value("fileName").toString());
        mFileSizeLabel->setPlainText(parameters.value("fileSzTxt").toString());
        mProgressBar->setMinimum(0);
        mProgressBar->setProgressValue(0);
        mProgressBar->setMaximum(parameters.value("fileSz").toInt());
        mFileIndex = parameters.value("currentFileIdx").toString().toInt();
        }
    else
        {
        mProgressBar->setProgressValue(parameters.value("progressValue").toInt());
        }
    return true;
    }

int BTSendDialogWidget::deviceDialogError() const
    {
    return 0;
    }

void BTSendDialogWidget::closeDeviceDialog(bool byClient)
    {
    Q_UNUSED(byClient);
    mSendDialog->close();
// below redundant call is required because of the api documentation. 
    emit deviceDialogClosed();
    }

HbPopup* BTSendDialogWidget::deviceDialogWidget() const
    {
    return mSendDialog;
    }

QObject *BTSendDialogWidget::signalSender() const
{
    return const_cast<BTSendDialogWidget*>(this);
}  

bool BTSendDialogWidget::constructDialog(const QVariantMap&/*parameters*/)
    {
    mLoader = new HbDocumentLoader();
    bool ok = false;
    
    mLoader->load(DOCML_BT_SEND_DIALOG, &ok);
    if(ok)
        {
        mSendDialog = qobject_cast<HbDialog*>(mLoader->findWidget("senddialog"));
        mDialogHeading = qobject_cast<HbLabel*>(mLoader->findWidget("heading"));
        mFileIconLabel = qobject_cast<HbLabel*>(mLoader->findWidget("fileiconlabel"));
        mFileNameLabel = qobject_cast<HbLabel*>(mLoader->findWidget("filenamelabel"));
        mFileSizeLabel = qobject_cast<HbLabel*>(mLoader->findWidget("filesizelabel"));
 //       mSendDialog->setHeadingWidget(mLabel);
   /*     mListView = qobject_cast<HbListView*>(mLoader->findWidget("listView"));
        if(mListView)
            {
            mContentItemModel = new QStandardItemModel(this);
            mListView->setModel(mContentItemModel);//, prototype);
            }*/
         
        mProgressBar = qobject_cast<HbProgressBar*>(mLoader->findWidget("horizontalProgressBar"));
                
   //     HbAction* hide = new HbAction("Hide");
     //   HbAction* cancel = new HbAction("Cancel");
        
    //    this->addAction(hide);
  //      this->addAction(cancel);
        
  //      QGraphicsWidget *widget = mLoader->findWidget(QString("container"));
   //     this->setContentWidget(widget);
        }

    mSendDialog->setBackgroundFaded(false);
    mSendDialog->setDismissPolicy(HbPopup::NoDismiss);
    mSendDialog->setTimeout(HbPopup::NoTimeout);
    mSendDialog->setAttribute(Qt::WA_DeleteOnClose);
    
    mHideAction = static_cast<HbAction*>( mLoader->findObject( "hideaction" ) );
    mHideAction->disconnect(mSendDialog);
    
    mCancelAction = static_cast<HbAction*>( mLoader->findObject( "cancelaction" ) );
    mCancelAction->disconnect(mSendDialog);
    
    connect(mCancelAction, SIGNAL(triggered()), this, SLOT(cancelClicked()));

    return true;
    }

/*void BTSendDialogWidget::hideEvent(QHideEvent *event)
    {
    HbDialog::hideEvent(event);
    emit deviceDialogClosed();
    }

void BTSendDialogWidget::showEvent(QShowEvent *event)
    {
    HbDialog::showEvent(event);
    }*/

/*void BTSendDialogWidget::hideClicked()
    {
    // TODO
    this->close();
    emit deviceDialogClosed();
    }*/

void BTSendDialogWidget::cancelClicked()
    {
    mSendDialog->close();
    emit deviceDialogClosed();
    }

/*void BTSendDialogWidget::inputClosed(HbAction* action)
    {
    QVariantMap data;
     
    HbDialog *dlg=static_cast<HbDialog*>(sender());
    if(dlg->actions().first() == action) {
    } 
    else if(dlg->actions().at(1) == action) {
      }
    }*/

