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


#ifndef BTDELEGATEINQUIRY_H
#define BTDELEGATEINQUIRY_H

#include <e32base.h>
#include <btengsettings.h>
#include "btabstractdelegate.h"

class BtuiModel;


class BtDelegateInquiry : public BtAbstractDelegate
{
    Q_OBJECT
    
public:
    explicit BtDelegateInquiry(
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, 
            QObject* parent = 0 );
    
    virtual ~BtDelegateInquiry();
    
    virtual void exec( const QVariant &params );
    
    virtual void cancel();
    
public slots:
    void powerDelegateCompleted(int error);
    
private:
    void exec_inquiry();
    
private:
    BtAbstractDelegate* mAbstractDelegate;
    
};

#endif /* BTDELEGATEINQUIRY_H */
