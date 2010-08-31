/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#ifndef BTSETTINGMODEL_P_H
#define BTSETTINGMODEL_P_H

#include <btsettingmodel.h>

#include <e32base.h>
#include <e32property.h>
#include <btengsettings.h>
#include <btservices/btsimpleactive.h>

/*!
    \class BtuimSettings
    \brief class for handling local Bluetooth setting updates.

    BtLocalSetting class is responsible for providing the latest information
    regarding the local Bluetooth settings such as device name and power state.

    \\sa bluetoothuimodel
 */
class BtSettingModelPrivate : public QObject,
                      public MBTEngSettingsObserver,
                      public MBtSimpleActiveObserver
{
    Q_OBJECT

public:
    explicit BtSettingModelPrivate( BtSettingModel& model, QObject *parent = 0 );
    
    virtual ~BtSettingModelPrivate();
    
    bool isValid( int row, int col ) const;
    
    int rowCount() const;
    
    int columnCount() const;
        
    void data(QVariant& val, int row, int col, int role ) const;
    
    BtuiModelDataItem itemData( int row, int col ) const;
signals:

    void settingDataChanged( int row, void *parent );
    
    void settingDataChanged( int first, int last, void *parent );
    
private:
    // from MBTEngSettingsObserver
    
    void PowerStateChanged( TBTPowerStateValue state );
    
    void VisibilityModeChanged( TBTVisibilityMode state );
    
    // from MBtSimpleActiveObserver
    
    void RequestCompletedL( CBtSimpleActive* active, TInt status );

    void CancelRequest( TInt requestId );

    void HandleError( CBtSimpleActive* active, TInt error );

private:

    void setVisibilityMode( TBTVisibilityMode state );
    void updateDeviceName( const QString &name );
    
    void setPowerSetting( TBTPowerStateValue state );
    
    //void setOfflineSetting( bool state );
    //void setBtConnectionsSetting( int connections );
    
    void getNameFromRegistry( QString &name );

private:
    
    BtuiModelDataSource mData;
    
    BtSettingModel& mModel;
    
    CBTEngSettings *mBtengSetting;
    
    // For monitoring local device name change
    RProperty mLocalDeviceKey;
    CBtSimpleActive *mLocalDeviceWatcher;
    
    //RProperty mBtLinkCountKey;
    //CBTEngActive *mBtLinkCountWatcher;
    Q_DISABLE_COPY(BtSettingModelPrivate)

};

#endif // BTSETTINGMODEL_P_H
