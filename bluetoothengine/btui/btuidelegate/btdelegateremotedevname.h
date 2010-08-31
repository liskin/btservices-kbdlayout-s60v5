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

#ifndef BTDELEGATEREMOTEDEVNAME_H
#define BTDELEGATEREMOTEDEVNAME_H

#include <e32base.h>
#include <btmanclient.h>
//#include <btengconnman.h>
//#include <btengdevman.h>
#include <btservices/btsimpleactive.h>
#include "btabstractdelegate.h"

class BtuiModel;

enum RequestIdentifiers {
    Unknown = 0,  // using a different number space than TBTVisibilityMode
    AddDevice,
    ModifyFriendlyName
};

/*!
    \class BtDelegateRemoteDevName
    \brief the base class for handling Bluetooth Local Name.
 */
class BtDelegateRemoteDevName : public BtAbstractDelegate, public MBtSimpleActiveObserver
{
    Q_OBJECT

public:
    explicit BtDelegateRemoteDevName( BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, QObject *parent = 0 );
    
    virtual ~BtDelegateRemoteDevName();

    virtual void exec( const QVariant &params );
   
    // from MBtSimpleActiveObserver
    virtual void RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus );
    
    virtual void CancelRequest( TInt aRequestId );
    
    virtual void HandleError( CBtSimpleActive* aActive, TInt aError );


private:
    bool validateName(QString &name );
    
    RBTRegistry mBtRegistry;
    
    RBTRegServ mBtRegServ;
    
    CBtSimpleActive* mRegistryActive;    
    
    QString mNewName;
    
    RBuf16 mSymName;
    
    TBTDevAddr mSymaddr;
    
    bool mRegistryOpened;
    

private:

    Q_DISABLE_COPY(BtDelegateRemoteDevName)

};

#endif // BTDELEGATEREMOTEDEVNAME_H
