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

#ifndef BTDELEGATEDEVNAME_H
#define BTDELEGATEDEVNAME_H

#include <e32base.h>
#include <btengsettings.h>
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegateDevName
    \brief the base class for handling Bluetooth Local Name.
 */
class BtDelegateDevName : public BtAbstractDelegate
{
    Q_OBJECT

public:
    explicit BtDelegateDevName( QObject *parent = 0 );
    
    virtual ~BtDelegateDevName();

    virtual void exec( const QVariant &params );
    
public slots:

private:
    bool validateName(QString &name );

    CBTEngSettings* mBtEngSettings;

private:

    Q_DISABLE_COPY(BtDelegateDevName)

};

#endif // BTDELEGATEDEVNAME_H
