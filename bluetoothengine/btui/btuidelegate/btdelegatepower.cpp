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


#include "btdelegatepower.h"
#include "btqtconstants.h"
#include <btabstractdelegate.h>
#include <btdelegatefactory.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <hbmessagebox.h>
#include <bluetoothuitrace.h>
#include <hbaction.h>

/*!
    Constructor.
 */
BtDelegatePower::BtDelegatePower(            
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject *parent )
    : BtAbstractDelegate( settingModel, deviceModel, parent ),
      mDisconnectDelegate(0)
{
    TRAP_IGNORE( mBtengSettings = CBTEngSettings::NewL(this) );
    Q_CHECK_PTR( mBtengSettings );
    mActiveHandling = false;
}

/*!
    Destructor.
 */
BtDelegatePower::~BtDelegatePower()
{
    delete mDisconnectDelegate;
    delete mBtengSettings;
}

/*!
    Turns BT power on/off
    param powerState is the desired power state and is of type PowerStateQtValue
 */
void BtDelegatePower::exec( const QVariant &powerState )
{   
    mReqPowerState = BtEngPowerState((PowerStateQtValue)powerState.toInt());
    BTUI_ASSERT_X( (mReqPowerState == EBTPowerOff) || (mReqPowerState == EBTPowerOn), 
            "BtDelegatePower::exec()", "wrong power state value" );
    
    // get current power status
    TBTPowerStateValue curPowerState(EBTPowerOff);
    mBtengSettings->GetPowerState( curPowerState );
    
    // verify requested power is not the same as current status
    if ( mReqPowerState == curPowerState ) {
        // no need to do anything
        emit commandCompleted( KErrNone );
        return;
    }
    
    // perform power on/off operation
    if ( mReqPowerState == EBTPowerOff ){ 
        switchBTOff();     
    }
    else if ( mReqPowerState == EBTPowerOn ) {
        switchBTOn();
    }
}
       
    

void BtDelegatePower::switchBTOn()
{
    int err = 0;
    
    //check if device is in OFFLINE mode first
    bool btEnabledInOffline = false;
    if (checkOfflineMode(btEnabledInOffline)){  // offline mode is active
        if (btEnabledInOffline){
            // BT is allowed to be enabled in offline mode, show query.
            HbMessageBox::question( hbTrId("txt_bt_info_trun_bluetooth_on_ini_offline_mode" ),this, 
							SLOT(btOnQuestionClose(int)), HbMessageBox::Yes | HbMessageBox::No );

        }
        else{
            //if BT is not allowed to be enabled in offline mode, show message and complete
            HbMessageBox::warning( hbTrId("txt_bt_info_bluetooth_not_allowed_to_be_turned_on" ),this, 
				SLOT(btOnWarningClose()));
        }
        
    }
    else { // offline mode is not active
        mActiveHandling = true;
        err = mBtengSettings->SetPowerState(EBTPowerOn);
    }
    
    if ( err ) {
        //TODO: handle the error here
        emit commandCompleted(KErrGeneral);
    }
    
}

void BtDelegatePower::btOnQuestionClose(int action)
{
    int err = 0;
    if(action == HbMessageBox::Yes) 
    {
        //user chooses "yes" for using BT in offline 
        mActiveHandling = true;
        err = mBtengSettings->SetPowerState(EBTPowerOn);
    }
    else
    {
        //if user chooses "NO", emits the signal
        emit commandCompleted(KErrNone);
           
    }     
    if ( err ) {
        //TODO: handle the error here
        emit commandCompleted(KErrGeneral);
    }
}

void BtDelegatePower::btOnWarningClose()
{
    emit commandCompleted(KErrNone);        
}



void BtDelegatePower::switchBTOff()
{
    int err = 0;
    
    CBTEngConnMan *btengConnMan = 0;
    TRAP(err, btengConnMan = CBTEngConnMan::NewL(this));
    Q_CHECK_PTR( btengConnMan );
    RBTDevAddrArray devAddrArray;
    err = btengConnMan->GetConnectedAddresses(devAddrArray);
    if ( err != KErrNone) {
       //TODO: handle the error here
       emit commandCompleted(err);
       return;
    }
    int count = devAddrArray.Count();
    devAddrArray.Close();
    delete btengConnMan;
    if( count> 0 ){
        mActiveHandling = true;
        disconnectOngoingConnections(); 
    }
    else{
        mActiveHandling = true;
        err = mBtengSettings->SetPowerState(EBTPowerOff);
        
        if ( err ) {
           //TODO: handle the error here
           emit commandCompleted(KErrGeneral);
        }
        
    }    
}
/*
void BtDelegatePower::btOffDialogClose(HbAction *action)
{
    HbMessageBox *dlg = static_cast<HbMessageBox*>(sender());
    if(action == dlg->actions().at(0)) 
    {
        //user chooses "yes" for closing active connection before power off
        mActiveHandling = true;
        disconnectOngoingConnections();
    }
    else
    {
        //if user chooses "NO", emits the signal
        emit commandCompleted(KErrNone);
           
    }     
    
}
*/
void BtDelegatePower::disconnectOngoingConnections(){
    if (! mDisconnectDelegate){
        mDisconnectDelegate = BtDelegateFactory::newDelegate(
                                                BtDelegate::Disconnect, getSettingModel(), getDeviceModel()); 
        connect( mDisconnectDelegate, SIGNAL(commandCompleted(int)), this, SLOT(disconnectDelegateCompleted(int)) );
            
    
    DisconnectOption discoOpt = AllOngoingConnections;
    QVariant param;
    param.setValue((int)discoOpt);
    mDisconnectDelegate->exec(param);
    }
}

void BtDelegatePower::disconnectDelegateCompleted(int err)
{
    Q_UNUSED( err );
    //TODO: handle the return error here
    
    int error = mBtengSettings->SetPowerState(EBTPowerOff);
    if ( error ) {
        //TODO: handle the error here
        emit commandCompleted(KErrGeneral);
    }
    
    
}


void BtDelegatePower::PowerStateChanged( TBTPowerStateValue aPowerState )
{
    // It is possible that others change power: no handling for these cases.
    if ( !mActiveHandling ) {
        return;
    } 
    mActiveHandling = false;
    
    if ( mReqPowerState == aPowerState ) {
        // power state changed successfully
        emit commandCompleted( KErrNone );
    }
    else {
        // the actual power state is not the same as we requested,
        // command failed:
        // ToDo:  show error note?
        emit commandCompleted( KErrGeneral );
    }
}

//Method derived from MBTEngSettingsObserver, no need to be implemented here
void BtDelegatePower::VisibilityModeChanged( TBTVisibilityMode aState )
{
    Q_UNUSED( aState );
}

void BtDelegatePower::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);
    Q_UNUSED(aConflicts);  
    /*
    if ( mBtEngAddr != aAddr ) {  // callback coming for some other device
        return;
    }
    emitCommandComplete(aErr);
    */
}

void BtDelegatePower::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
{
    Q_UNUSED(aAddr);
    Q_UNUSED(aErr);    
}

/*!
   Returns true if offline mode is on, parameter returns true if BT is allowed 
   in offline mode
 */
bool BtDelegatePower::checkOfflineMode(bool& btEnabledInOffline)
{
    TCoreAppUIsNetworkConnectionAllowed offLineMode; 
    TBTEnabledInOfflineMode btEnabled;
   
    mBtengSettings->GetOfflineModeSettings(offLineMode, btEnabled);
    
    btEnabledInOffline = (btEnabled == EBTEnabledInOfflineMode);
    return (offLineMode == ECoreAppUIsNetworkConnectionNotAllowed);
}

