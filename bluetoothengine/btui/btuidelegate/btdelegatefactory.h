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

#ifndef BTDELEGATEFACTORY_H
#define BTDELEGATEFACTORY_H

#include <QObject>
#include "btdelegateconsts.h"

class BtAbstractDelegate;
class BtSettingModel;
class BtDeviceModel;

#ifdef BUILD_BTUIDELEGATE
#define BTUIDELEGATE_IMEXPORT Q_DECL_EXPORT
#else
#define BTUIDELEGATE_IMEXPORT Q_DECL_IMPORT
#endif

/*!
    \class BtDelegateFactory
    \brief the base class for creating concrete delegate instances

    \\sa btuidelegate
 */
class BTUIDELEGATE_IMEXPORT BtDelegateFactory
{

public:
    static BtAbstractDelegate *newDelegate( 
            BtDelegate::Command cmd, 
            BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, 
            QObject *parent = 0 );

};

#endif // BTDELEGATEFACTORY_H
