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

#ifndef BTDEVICEMODEL_H
#define BTDEVICEMODEL_H

#include <qglobal.h>
#include <QAbstractItemModel>
#include <QSharedPointer>
#include <btuimodeltypes.h>

class BtDeviceModelPrivate;

/*!
    \class BtDeviceModel
    \brief The data model provided to Bluetooth UIs in QT

    BtDeviceModel provides APIs for accessing the data of remote devices. 
    In addition, signals from this
    model are provided for being informed of data update. 
    
    This model is in one dimension (n rows * 1 columns), i.e.,
    
          row 0 ( a remote device)
          row 1 ( another device)
          ...
    
    The data in this model is non-modifiable from the user interface (except device
    search may add a number of in-range devices temporarily) , 
    determined by the characteristics of Bluetooth, the underline BT software 
    services and the BT application requirements. 
    
    Whenever feasible, the detailed description should contain a simple
    example code example:
    \code
    // ...
    \endcode

    \sa \link model-view-programming.html Model/View Programming\endlink
 */

class BTUIMODEL_IMEXPORT BtDeviceModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS( DevDataRole DevMajorProperty AVDevMinorProperty PeripheralMinorProperty )

public:

    // the roles for catogerizing Bluetooth device properties
    enum DevDataRole {
        NameAliasRole = Qt::DisplayRole, // QVariant::String, the name showing in UI
        ReadableBdaddrRole = Qt::UserRole, // QString, the readable format of a BD_ADDR (BT Device address)
        LastUsedTimeRole, // QDateTime
        RssiRole,         // QVariant::Int
        MajorPropertyRole,  // QVariant::Int, bits of DevMajorProperty
        MinorPropertyRole,  // QVariant::Int, bits of DevMinorProperty
        CoDRole,  // QVariant::Int, the value of Class of Device
        SeqNumRole    // sequence number indicating order in which device was found
    };
    
public:
    
    explicit BtDeviceModel( QObject *parent = 0 );
    
    explicit BtDeviceModel( const BtDeviceModel &model, QObject *parent = 0 );
    
    virtual ~BtDeviceModel();
    
    bool searchDevice();
    
    void cancelSearchDevice();
    
    void removeTransientDevices();
    
    // from QAbstractItemModel
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QModelIndex parent( const QModelIndex &child ) const;
    
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual QMap<int, QVariant> itemData( const QModelIndex & index ) const;
    
signals:

    void deviceSearchCompleted(int error);

private slots:

    void deviceDataChanged( int row, void *parent );
    
    void deviceDataChanged( int first, int last, void *parent );
    
    void beginInsertDevices(int first, int last, void *parent);
    void endInsertDevices();
    
    void beginRemoveDevices(int first, int last, void *parent);
    void endRemoveDevices();

    void emitDeviceSearchCompleted( int error );

private:
    
    void connectModelSignals();
    
private:
    QSharedPointer<BtDeviceModelPrivate> d;
};

#endif // BTUIMODEL_H
