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

#ifndef BTABSTRACTDELEGATE_H
#define BTABSTRACTDELEGATE_H

#include <QObject>
#include <QVariant>
#include <QModelIndex>

class BtSettingModel;
class BtDeviceModel;

#ifdef BUILD_BTUIDELEGATE
#define BTUIDELEGATE_IMEXPORT Q_DECL_EXPORT
#else
#define BTUIDELEGATE_IMEXPORT Q_DECL_IMPORT
#endif


/*!
    \class BtAbstractDelegate
    \brief the base class for handling user requests from BT application.


    \\sa btuidelegate
 */
class BTUIDELEGATE_IMEXPORT BtAbstractDelegate : public QObject
{
    Q_OBJECT

public:
    explicit BtAbstractDelegate( BtSettingModel* settingModel, 
            BtDeviceModel* deviceModel, QObject *parent = 0 );
    
    virtual ~BtAbstractDelegate();

    virtual void exec( const QVariant &params ) = 0;

    virtual void cancel();
    
signals:
    void commandCompleted(int status, QVariant params = QVariant() );
    
protected:
    
    BtSettingModel *getSettingModel();
    BtDeviceModel *getDeviceModel();
    
    bool isBtPowerOn();
    
public slots:

private:
    
    // pointer to models. do not own. 
    BtSettingModel *mSettingModel;
    BtDeviceModel *mDeviceModel;
    
    Q_DISABLE_COPY(BtAbstractDelegate)

};

Q_DECLARE_METATYPE(QModelIndex)

#endif // BTABSTRACTDELEGATE_H
