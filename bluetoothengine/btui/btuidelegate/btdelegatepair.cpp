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

#include "btdelegatepair.h"
#include "btuiutil.h"
#include "btdelegatefactory.h"
#include "btqtconstants.h"

#include <QModelIndex>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <bluetoothuitrace.h>
#include <hbnotificationdialog.h>
#include <hblabel.h>
#include <hbprogressbar.h>

// docml to load
const char* BTUI_PAIR_WAIT_DOCML = ":/docml/pairwaitingdialog.docml";

BtDelegatePair::BtDelegatePair(
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, 
        QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtengConnMan(0)
{
    mLoader = new HbDocumentLoader();
    mAbstractDelegate = NULL;
}

BtDelegatePair::~BtDelegatePair()
{
    delete mBtengConnMan;
    delete mLoader;
}

void BtDelegatePair::exec( const QVariant &params )
{
    
    deviceIndex = params.value<QModelIndex>();
    
    if(isBtPowerOn()) {
        exec_pair();
    }
    else {
        //If Bt Power is off, switch it on and then perform pairing.
        //todo: Do we ask for user confirmation here..?
        if (!mAbstractDelegate) { 
            mAbstractDelegate = BtDelegateFactory::newDelegate(BtDelegate::ManagePower, 
                    getSettingModel(), getDeviceModel() ); 
            connect( mAbstractDelegate, SIGNAL(commandCompleted(int)), this, SLOT(powerDelegateCompleted(int)) );
            mAbstractDelegate->exec(QVariant(BtPowerOn));
        }
    }
    
}

void BtDelegatePair::powerDelegateCompleted(int error)
{
    if (mAbstractDelegate) {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    if ( error == KErrNone ) {
        exec_pair();
    } 
    else {
        // error
        emitCommandComplete(error);
    }
}

void BtDelegatePair::exec_pair()
{
    int error = KErrNone;
    mdeviceName = getDeviceModel()->data(deviceIndex,BtDeviceModel::NameAliasRole).toString();
    
    QString strBtAddr = getDeviceModel()->data(deviceIndex,BtDeviceModel::ReadableBdaddrRole).toString();
    int cod = getDeviceModel()->data(deviceIndex,BtDeviceModel::CoDRole).toInt();
    
    if ( ! mBtengConnMan ){
        TRAP( error, mBtengConnMan = CBTEngConnMan::NewL(this) );
    }
    
    if ( !error ) {
        TBTDevAddr btEngddr;
        addrReadbleStringToSymbian( strBtAddr, btEngddr );
        TBTDeviceClass btEngDeviceClass(cod);
        error = mBtengConnMan->PairDevice(btEngddr, btEngDeviceClass);
        launchWaitDialog();
    }
    
    if(error) {
        emitCommandComplete(error);
    }

}


void BtDelegatePair::launchWaitDialog()
{
    QString headingText(hbTrId("txt_bt_title_pairing_with_1"));
    HbLabel *heading;
    HbProgressBar* progressBar;
    
    bool ok = false;
    mLoader->load( BTUI_PAIR_WAIT_DOCML, &ok );
    // Exit if the file format is invalid
    BTUI_ASSERT_X( ok, "BTUI_PAIR_WAIT_DOCML", "Invalid docml file" );

    mWaitDialog = qobject_cast<HbDialog *>( mLoader->findWidget( "dialog" ) );
    BTUI_ASSERT_X( mWaitDialog != 0, "BTUI_PAIR_WAIT_DOCML", "dialog not found" );
    
    heading = qobject_cast<HbLabel *>( mLoader->findWidget( "heading" ) );
    BTUI_ASSERT_X( heading != 0, "BTUI_PAIR_WAIT_DOCML", "heading not found" );

    progressBar = qobject_cast<HbProgressBar *>( mLoader->findWidget( "progressBar" ) );
    BTUI_ASSERT_X( progressBar != 0, "BTUI_PAIR_WAIT_DOCML", "progressBar not found" );
    progressBar->setRange(0,0);
    
    heading->setPlainText(headingText.arg(mdeviceName));
    
    mWaitDialog->setDismissPolicy(HbPopup::NoDismiss);
    mWaitDialog->setTimeout(HbPopup::NoTimeout);

    mWaitDialog->show();
}

void BtDelegatePair::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
    Q_UNUSED(aConflicts);

}

void BtDelegatePair::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
}

void BtDelegatePair::PairingComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    
    mWaitDialog->close();
    emitCommandComplete(aErr);
}

void BtDelegatePair::emitCommandComplete(int error)
{

    emit commandCompleted(error);
}

void BtDelegatePair::cancel()
{
    if ( mBtengConnMan ) {
        mBtengConnMan->CancelPairDevice();
    }
}



