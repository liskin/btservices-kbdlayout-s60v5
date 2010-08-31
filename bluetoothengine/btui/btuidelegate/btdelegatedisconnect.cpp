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
#include "btdelegatedisconnect.h"
#include "btuiutil.h"
#include "btuiiconutil.h"
#include <QModelIndex>
#include <hblabel.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbnotificationdialog.h>

BtDelegateDisconnect::BtDelegateDisconnect(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent) :
    BtAbstractDelegate(settingModel, deviceModel, parent), mBtengConnMan(0), mPhyLinks(0),
	 mMajorRole(0), mActiveHandling(false), mAddrArrayIndex(0), mDisconOpt(DisconUnknown)
{
    
}

BtDelegateDisconnect::~BtDelegateDisconnect()
{
    delete mBtengConnMan;
    delete mPhyLinks;
    mSocketServ.Close();
}

void BtDelegateDisconnect::exec( const QVariant &params )
{
    int err;
    if ( ! mBtengConnMan ){
        TRAP( err, mBtengConnMan = CBTEngConnMan::NewL(this) );
    }
    if(err) {
        emit commandCompleted(err);
        return;
    }
    if (params.canConvert<int>()){
        mDisconOpt = (DisconnectOption)params.toInt();
        mActiveHandling = true;
        
        if (mDisconOpt == AllOngoingConnections){
            err = mBtengConnMan->GetConnectedAddresses(mDevAddrArray);
            disconnectAllConnections_service();
        }
        if(err) {
            emit commandCompleted(err);
        }
    }
    else{
        QList<QVariant> paramList = params.value< QList<QVariant> >(); 
        QVariant indexVariant = paramList.at(0); 
        QModelIndex index = indexVariant.value<QModelIndex>();
        QVariant optionVariant = paramList.at(1); 
        mDisconOpt = (DisconnectOption)optionVariant.toInt();
        int error = KErrNone;
        
        mActiveHandling = true;
        mDeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
        mMajorRole = (index.data(BtDeviceModel::MajorPropertyRole)).toInt();
        
        QString strBtAddr = getDeviceModel()->data(index,BtDeviceModel::ReadableBdaddrRole).toString();
        
        // todo: address converting should be simplified. check other delegates for example.
        
        TPtrC ptrName(reinterpret_cast<const TText*>(strBtAddr.constData()));
            
        RBuf16 btName;
        error = btName.Create(ptrName.Length());
        
        if(error == KErrNone) {
            btName.Copy(ptrName);
            mBtEngAddr.SetReadable(btName);
            if (mDisconOpt == ServiceLevel){
                disconnectSeviceLevel();
            }
            else if (mDisconOpt == PhysicalLink){
                disconnectPhysicalLink();       
            }
        }
        btName.Close();
        
        if(error) {
            emit commandCompleted(error);
        }
    }  
}


void BtDelegateDisconnect::disconnectAllConnections_service(){
    
        TBuf<KBTDevAddrSize*3> addrBuf;
        mDevAddrArray[mAddrArrayIndex].GetReadable(addrBuf);
        QString btStringAddr= QString::fromUtf16( addrBuf.Ptr(), addrBuf.Length());
        QModelIndex start = getDeviceModel()->index(0,0);
        QModelIndexList indexList = getDeviceModel()->match(start,BtDeviceModel::ReadableBdaddrRole, btStringAddr);
        QModelIndex index = indexList.at(0);
        
        mDeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
        mBtEngAddr = mDevAddrArray[mAddrArrayIndex];
        mMajorRole = (index.data(BtDeviceModel::MajorPropertyRole)).toInt();
        
        disconnectSeviceLevel();        
}

void BtDelegateDisconnect::disconnectAllConnections_physical(){
    
        TBuf<KBTDevAddrSize*3> addrBuf;
        mDevAddrArray[mAddrArrayIndex].GetReadable(addrBuf);
        QString btStringAddr= QString::fromUtf16( addrBuf.Ptr(), addrBuf.Length());
        QModelIndex start = getDeviceModel()->index(0,0);
        QModelIndexList indexList = getDeviceModel()->match(start,BtDeviceModel::ReadableBdaddrRole, btStringAddr);
        QModelIndex index = indexList.at(0);
        
        mDeviceName = getDeviceModel()->data(index,BtDeviceModel::NameAliasRole).toString();
        mBtEngAddr = mDevAddrArray[mAddrArrayIndex];
        
        disconnectPhysicalLink();
        
}
void BtDelegateDisconnect::disconnectSeviceLevel(){
    int err;
    TBTEngConnectionStatus connStatus = EBTEngNotConnected;
    err = mBtengConnMan->IsConnected(mBtEngAddr, connStatus);
    if (connStatus == EBTEngConnected){
        err = mBtengConnMan->Disconnect(mBtEngAddr, EBTDiscGraceful);
    }
    if(err) {
        if (mDisconOpt == AllOngoingConnections){
            disconnectServiceLevelCompleted(err);
        }
        else{ 
            emit commandCompleted(err);
        }
    }
}
        
void BtDelegateDisconnect::disconnectPhysicalLink(){
    int err;
    if ( !mSocketServ.Handle() ) {
        err = mSocketServ.Connect();
    }
    if ( !err && !mPhyLinks ) {
        TRAP( err, 
            mPhyLinks = CBluetoothPhysicalLinks::NewL( *this, mSocketServ ) );
        Q_CHECK_PTR( mPhyLinks );
    }
    err = mPhyLinks->Disconnect( mBtEngAddr );
    if(err) {
        if (mDisconOpt == AllOngoingConnections){
            disconnectPhysicalLinkCompleted(err);
        }
        else{ 
            emit commandCompleted(err);
        }
    }
    
}

void BtDelegateDisconnect::disconnectServiceLevelCompleted(int err){
    if (mDisconOpt == ServiceLevel){
        mActiveHandling = false;
        emit commandCompleted(err);
    }
    else if (mDisconOpt == AllOngoingConnections){
        if (err){
            mActiveHandling = false;
            emit commandCompleted(err);
        }
        else{
            mAddrArrayIndex++;
            if ( mAddrArrayIndex < mDevAddrArray.Count()){
                disconnectAllConnections_service();
            }
            else{
                mDevAddrArray.Reset();
                err = mBtengConnMan->GetConnectedAddresses(mDevAddrArray);
                if(err) {
                    emit commandCompleted(err);
                    return;
                }
                mAddrArrayIndex = 0;
                //connect( mDisconnectDelegate, SIGNAL(commandCompleted(int)), this, SLOT(disconnectPhysicalLinkCompleted(int)) );         
                disconnectAllConnections_physical();
            }
        }
    }
}

void BtDelegateDisconnect::disconnectPhysicalLinkCompleted(int err){
    if (mDisconOpt == PhysicalLink){
        //emitCommandComplete(err);
        mActiveHandling = false;
        emit commandCompleted(err);
    }
    else if (mDisconOpt == AllOngoingConnections){
        if (err){
            mActiveHandling = false;
            emit commandCompleted(err);
        }
        else{
            mAddrArrayIndex++;
            if ( mAddrArrayIndex < mDevAddrArray.Count()){
                disconnectAllConnections_physical();
            }
            else{
                //TODO: check if there is still ongoing connection from BTEngVonnMan. and close them again if there is any new 
                mActiveHandling = false;
                emit commandCompleted(err);
            }
        }
        
    }
    
}
void BtDelegateDisconnect::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aConflicts);  
    Q_UNUSED(aErr);
}

void BtDelegateDisconnect::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    if ( mBtEngAddr != aAddr || !mActiveHandling ) {  
        return;
    }
    DisplayCommandCompleteNotif(aErr);
    disconnectServiceLevelCompleted(aErr);    
}


void BtDelegateDisconnect::cancel()
{
    
}

void BtDelegateDisconnect::HandleCreateConnectionCompleteL( TInt err ){
    Q_UNUSED( err );
}

void BtDelegateDisconnect::HandleDisconnectCompleteL( TInt err ){
    if ( !mActiveHandling ) {  
        return;
    } 
    disconnectPhysicalLinkCompleted(err);
          
}

void BtDelegateDisconnect::HandleDisconnectAllCompleteL( TInt err ){
    Q_UNUSED( err );
}

void BtDelegateDisconnect::DisplayCommandCompleteNotif(int error)
{
    
    if(error == KErrNone) {
        // success, show indicator with connection status
        HbIcon icon = getBadgedDeviceTypeIcon( mCod, mMajorRole, 0 );  // no badging required, only icon
        QString str( hbTrId("txt_bt_dpopinfo_disconnected_from_1") );
        HbNotificationDialog::launchDialog( icon, hbTrId("txt_bt_dpophead_disconnected"), 
            str.arg(mDeviceName) );  
    }
	
}


