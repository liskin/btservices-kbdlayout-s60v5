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
#include "btdelegatedevsecurity.h"
#include "btqtconstants.h"
#include <QModelIndex>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <btdelegatefactory.h>
#include <hbnotificationdialog.h>

BtDelegateDevSecurity::BtDelegateDevSecurity(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, 
        QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtEngDevMan(0), mBtengConnMan(0), mDisconnectDelegate(0)
{
    
}

BtDelegateDevSecurity::~BtDelegateDevSecurity()
{
    delete mBtEngDevMan;
    delete mBtengConnMan;
    delete mDisconnectDelegate;
}


void BtDelegateDevSecurity::exec( const QVariant &params )
{
    int error = KErrNone;
    QModelIndex index = params.value<QModelIndex>();
    
    QString strBtAddr = getDeviceModel()->data(index,
            BtDeviceModel::ReadableBdaddrRole).toString();
    
    mdeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
    
    TBTDevAddr symaddr;
    TBuf<KBTDevAddrSize * 2> buffer(strBtAddr.utf16());
    symaddr.SetReadable( buffer );
    
    // Disconnect if paired device was connected 
    if ( ! mBtengConnMan ){
        TRAP( error, mBtengConnMan = CBTEngConnMan::NewL(this) );
    }
    TBTEngConnectionStatus connstatus;
    if ( !error && mBtengConnMan->IsConnected(symaddr, connstatus ) == KErrNone) {
        if ( connstatus == EBTEngConnected) {
            if (! mDisconnectDelegate){
                mDisconnectDelegate = BtDelegateFactory::newDelegate(
                                        BtDelegate::Disconnect, getSettingModel(), getDeviceModel()); 
                connect( mDisconnectDelegate, SIGNAL(commandCompleted(int)), this, SLOT(disconnectDelegateCompleted(int)) );
                
            }
            QList<QVariant>list;
            QVariant paramFirst;
            paramFirst.setValue(index);            
            QVariant paramSecond;
            DisconnectOption discoOpt = ServiceLevel;
            paramSecond.setValue((int)discoOpt);
            list.append(paramFirst);
            list.append(paramSecond);
            QVariant paramsList;
            paramsList.setValue(list);
            mDisconnectDelegate->exec(paramsList);
        }
    }

    // Set device as unpaired
    CBTDevice *symBtDevice = 0;
    TRAP( error, {
            symBtDevice = CBTDevice::NewL( symaddr );
            if( !mBtEngDevMan) {
                mBtEngDevMan = CBTEngDevMan::NewL( this );
            }
    });
    
    if ( !error ) {
        symBtDevice->SetPaired(EFalse);
        // deleting link key for executing unpair is safe as no 
        // link key shall exist if the device has been unpaired. 
        symBtDevice->DeleteLinkKey();
        error = mBtEngDevMan->ModifyDevice( *symBtDevice );
    }
    delete symBtDevice;
    
    if ( error ) {
        emitCommandComplete(error);
    }
}

void BtDelegateDevSecurity::cancel()
{
    
}

void BtDelegateDevSecurity::disconnectDelegateCompleted( int err )
{
    Q_UNUSED(err);
}

void BtDelegateDevSecurity::HandleDevManComplete( TInt aErr )
{
    emitCommandComplete(aErr);
}

void BtDelegateDevSecurity::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray )
{
    Q_UNUSED(aErr);
    Q_UNUSED(aDeviceArray);
}

void BtDelegateDevSecurity::emitCommandComplete(int error)
{
    // no dialogs here since stack provides "unpaired to %1" dialog
    // and failures are not reported
    
    emit commandCompleted(error);
}

void BtDelegateDevSecurity::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);
    Q_UNUSED(aConflicts);  
}

void BtDelegateDevSecurity::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
}




