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

#ifndef BTDELEGATEDISCONNECT_H
#define BTDELEGATEDISCONNECT_H

#include <e32base.h>
#include <btengconnman.h>
#include "btqtconstants.h"
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegateDisconnect
    \brief the base class for Disconnecting Bluetooth Connection.
 */
class BtDelegateDisconnect : public BtAbstractDelegate,
        public MBTEngConnObserver, public MBluetoothPhysicalLinksNotifier
{
    Q_OBJECT

public:
    explicit BtDelegateDisconnect(            
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, QObject *parent = 0 );
    
    virtual ~BtDelegateDisconnect();

    virtual void exec( const QVariant &params );
    
    virtual void cancel();
    


protected:
    //From MBTEngConnObserver
    virtual void ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts );
    virtual void DisconnectComplete( TBTDevAddr& aAddr, TInt aErr );
    
    // from MBluetoothPhysicalLinksNotifier
    virtual void HandleCreateConnectionCompleteL( TInt err );

    virtual void HandleDisconnectCompleteL( TInt err );

    virtual void HandleDisconnectAllCompleteL( TInt err );

    void DisplayCommandCompleteNotif(int error);
    
private:
    
    void disconnectAllConnections_service();
    
    void disconnectAllConnections_physical();
    
    void disconnectSeviceLevel();
        
    void disconnectPhysicalLink();
    
    void disconnectServiceLevelCompleted(int err);

    void disconnectPhysicalLinkCompleted(int err);
    
    
    
private:

    CBTEngConnMan *mBtengConnMan;

    CBluetoothPhysicalLinks *mPhyLinks;

    int mMajorRole;
    bool mActiveHandling;
    
    int mAddrArrayIndex;
    DisconnectOption mDisconOpt;

    RBTDevAddrArray mDevAddrArray;
    TBTDevAddr mBtEngAddr;
    
    QString mDeviceName;
    int mCod;
      
    RSocketServ mSocketServ;
       
    Q_DISABLE_COPY(BtDelegateDisconnect)

};



#endif /* BTDELEGATEDISCONNECT_H */
