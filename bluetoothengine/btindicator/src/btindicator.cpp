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
 * Description:
 *
 */
#include "btindicator.h" 
#include <QVariant.h>
//#include <hbmessagebox.h>
//#include <xqaiwrequest.h>
#include "btindicatorconstants.h"
//#include <XQServiceRequest.h>
//#include <cpbasesettingview.h>
//#include <cppluginlauncher.h>
#define LOC_BLUETOOTH hbTrId("txt_bt_dblist_bluetooth")
#define LOC_BLUETOOTH_OFF hbTrId("txt_bt_dblist_bluetooth_val_off")
#define LOC_BLUETOOTH_ON_VISIBLE hbTrId("txt_bt_dblist_bluetooth_val_on_and_visible")
#define LOC_BLUETOOTH_VISIBLE_CONNECTED hbTrId("txt_bt_dblist_bluetooth_val_visible_and_connected")
#define LOC_BLUETOOTH_ON_HIDDEN hbTrId("txt_bt_dblist_bluetooth_val_on_and_hidden")
#define LOC_BLUETOOTH_HIDDEN_CONNECTED hbTrId("txt_bt_dblist_bluetooth_val_hidden_and_connected")

struct BTIndicatorInfo
    {
     QString icon;
     QString secondaryText;
    };

static const int BTIndicatorCount = 5;


static const BTIndicatorInfo IndicatorInfo[BTIndicatorCount] = { 
     { "qtg_large_bluetooth_off", "LOC_BLUETOOTH_OFF" }, 
     { "qtg_large_bluetooth", "LOC_BLUETOOTH_ON_VISIBLE" }, 
     { "qtg_large_bluetooth_hide","LOC_BLUETOOTH_ON_HIDDEN" },
     { "qtg_large_bluetooth_active_connection", "LOC_BT_VISIBLE_CONNECT" }, 
     { "qtg_large_bluetooth_hide_connection","LOC_BT_HIDDEN_CONNECT"  }
 };


// ----------------------------------------------------------------------------
// BTIndicator::BTIndicator
// ----------------------------------------------------------------------------
BTIndicator::BTIndicator(const QString &indicatorType) :
HbIndicatorInterface(indicatorType,
        HbIndicatorInterface::SettingCategory ,
        InteractionActivated)
    {
    mIndicatorStatus = 0;
    }

// ----------------------------------------------------------------------------
// BTIndicator::~BTIndicator
// ----------------------------------------------------------------------------
BTIndicator::~BTIndicator()
    {
    }


// ----------------------------------------------------------------------------
// BTIndicator::handleInteraction
// ----------------------------------------------------------------------------
bool BTIndicator::handleInteraction(InteractionType type)
    {
  //  bool handled = false;
    if (type == InteractionActivated) 
        {
    //@TODO need to code for launching the BT Control Panel Plugin is available

 //       launchBTCpSettingView();
        }
    return true;
    }

// ----------------------------------------------------------------------------
// BTIndicator::indicatorData
// returns the data and icon that needs to be displayed in the universal pop up and indicator menu 
// ----------------------------------------------------------------------------
QVariant BTIndicator::indicatorData(int role) const
{
    switch(role)
    {
    case PrimaryTextRole: 
        {
        QString text(LOC_BLUETOOTH);
        return text;
        }
    case SecondaryTextRole:
        {
        return IndicatorInfo[mIndicatorStatus].secondaryText;//mSecDisplayName;
        }
    case DecorationNameRole:
        {
        //QString iconName("qtg_large_bluetooth");
        return IndicatorInfo[mIndicatorStatus].icon;//iconName;
        }
    case MonoDecorationNameRole :
        {
       // QString iconName("qtg_large_bluetooth");
        return IndicatorInfo[mIndicatorStatus].icon;//iconName;
        }

    default: 
        return QVariant();      
     }
}

// ----------------------------------------------------------------------------
// BTIndicator::handleClientRequest
// handles client's activate and deactivate request
// ----------------------------------------------------------------------------
bool BTIndicator::handleClientRequest( RequestType type, 
        const QVariant &parameter)
    {
    bool handled(false);
    switch (type) {
        case RequestActivate:
            {
            mSecDisplayName.clear();
/*            if(parameter.toInt() == EBTIndicatorOff)
                mSecDisplayName.append("LOC_BLUETOOTH_OFF");
            else if(parameter.toInt() == EBTIndicatorOnVisible)
                mSecDisplayName.append("LOC_BLUETOOTH_ON_VISIBLE");
            else if(parameter.toInt() == EBTIndicatorOnHidden)
                mSecDisplayName.append("LOC_BLUETOOTH_ON_HIDDEN");
            else if(parameter.toInt() == EBTIndicatorVisibleConnected)
                mSecDisplayName.append("LOC_BT_VISIBLE_CONNECT");
            else if(parameter.toInt() == EBTIndicatorHiddenConnected)
                mSecDisplayName.append("LOC_BT_HIDDEN_CONNECT");*/
            mIndicatorStatus = parameter.toInt();
            emit dataChanged();
            handled =  true;
            }
            break;
        default:
            {
            mSecDisplayName.clear();
            emit deactivate();
            }
            break;
    }
    return handled;
    }


/*void BTIndicator::launchBTCpSettingView()
    {
    XQAiwRequest *request = mAppMgr.create("obexhighway","com.nokia.symbian.IFileShare","send(QVariant)",true);

    if (!request)
    {
       qDebug("BTIndicator request returned with NULL");
        return;
    }
    else
    {
        connect(request, SIGNAL(requestOk(QVariant)), SLOT(handleReturnValue(QVariant)));
        connect(request, SIGNAL(requestError(int,QString)), SLOT(handleError(int,QString)));
    }
    // Set arguments for request 
    QList<QVariant> args;
    //c:\resource\qt\plugins\controlpanel\nfccpplugin.qtplugin
    args << QVariant( "c:\\two.jpg" );//btcpplugin.dll
//    args << QVariant ( "c:\\two.jpg" );
    request->setArguments(args);

    // Make the request
    if (!request->send())
    {
        //report error  
        qDebug("BTIndicator::launchBTCpSettingView request not sent");
    }
    
    delete request;
    
    }*/


/*void BTIndicator::handleReturnValue(const QVariant &returnValue)
{
}

void BTIndicator::handleError(int errorCode,const QString &errorMessage)
    {
    Q_UNUSED(errorCode);
    }*/
