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


#include "btdelegateinquiry.h"
#include "btdelegatefactory.h"
#include "btqtconstants.h"

#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <bluetoothuitrace.h>


BtDelegateInquiry::BtDelegateInquiry(
        BtSettingModel* settingModel, 
        BtDeviceModel* deviceModel, QObject* parent )
    :BtAbstractDelegate( settingModel, deviceModel, parent )
{
    mAbstractDelegate = NULL;
}

BtDelegateInquiry::~BtDelegateInquiry()
{

}

void BtDelegateInquiry::exec( const QVariant& params )
{
    Q_UNUSED(params);
    
    if(isBtPowerOn()) {
        exec_inquiry();
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

void BtDelegateInquiry::powerDelegateCompleted(int error)
{
    if (mAbstractDelegate) {
        disconnect(mAbstractDelegate);
        delete mAbstractDelegate;
        mAbstractDelegate = 0;
    }
    if ( error == KErrNone ) {
        exec_inquiry();
    } 
    else {
        // error
        emit commandCompleted(error);
    }
}

void BtDelegateInquiry::exec_inquiry()
{
    bool err = getDeviceModel()->searchDevice();

    emit commandCompleted(err);  // in case of error, passing original error back
}

void BtDelegateInquiry::cancel()
{
    getDeviceModel()->cancelSearchDevice();
}
