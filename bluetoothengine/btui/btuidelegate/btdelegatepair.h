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

#ifndef BTDELEGATEPAIR_H
#define BTDELEGATEPAIR_H

#include <e32base.h>
#include <btengconnman.h>
#include <hbdialog.h>
#include <hbdocumentloader.h>
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegatePair
    \brief the base class for handling Bluetooth Pair functionality.
 */
class BtDelegatePair : public BtAbstractDelegate,
        public MBTEngConnObserver
{
    Q_OBJECT

public:
    explicit BtDelegatePair(             
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, QObject *parent = 0 );
    
    virtual ~BtDelegatePair();

    virtual void exec( const QVariant &params );
    
    virtual void cancel();
    
public slots:
    void powerDelegateCompleted(int error);
protected:
    //From MBTEngConnObserver
    virtual void ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                                   RBTDevAddrArray* aConflicts );
    virtual void DisconnectComplete( TBTDevAddr& aAddr, TInt aErr );
    virtual void PairingComplete( TBTDevAddr& aAddr, TInt aErr );

private:
    void launchWaitDialog();
    void emitCommandComplete(int error);
    void exec_pair();
    
private:

    CBTEngConnMan *mBtengConnMan;
    HbDialog *mWaitDialog;
    QString mdeviceName;
    
    HbDocumentLoader *mLoader;
    BtAbstractDelegate* mAbstractDelegate;
    QModelIndex deviceIndex;
    
    Q_DISABLE_COPY(BtDelegatePair)

};

#endif /* BTDELEGATEPAIR_H */
