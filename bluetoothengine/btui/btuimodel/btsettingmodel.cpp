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

#include <btsettingmodel.h>
#include "btsettingmodel_p.h"
#include "bluetoothuitrace.h"

/*!
    This Constructor creates new instances of model data structure.
 */
BtSettingModel::BtSettingModel( QObject *parent )
    : QAbstractItemModel( parent )
{
   d = QSharedPointer<BtSettingModelPrivate>( new BtSettingModelPrivate( *this ) );
   connectModelSignals();
}

/*!
    This Constructor shares the private implementation of the setting model.
 */
BtSettingModel::BtSettingModel( const BtSettingModel &model, QObject *parent )
    : QAbstractItemModel( parent )
{
    d = model.d;
    connectModelSignals();
}

/*!
    Destructor.
 */
BtSettingModel::~BtSettingModel()
{
}

/*!
    \reimp
 */
QModelIndex BtSettingModel::index( int row, int column, const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    if ( d->isValid( row, column ) ) {
        return createIndex( row, column, d.data() );
    }
    // invalid row and column:
    return QModelIndex();
}

/*!
    \reimp
 */
QModelIndex BtSettingModel::parent( const QModelIndex &child ) const
{
    Q_UNUSED( child );
    // root level, no parent.
    return QModelIndex();
}

/*!
    \reimp
 */
int BtSettingModel::rowCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return d->rowCount();
}

/*!
    \reimp
 */
int BtSettingModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return d->columnCount();
}

/*!
    \reimp
 */
QVariant BtSettingModel::data( const QModelIndex &index, int role ) const
{
    QVariant val( QVariant::Invalid );
    d.data()->data( val, index.row(), index.column(), role );
    return val;
}

QMap<int, QVariant> BtSettingModel::itemData( const QModelIndex & index ) const
{
    return d.data()->itemData( index.row(), index.column() );
}

/*!
    emits dataChanged signal.
 */
void BtSettingModel::settingDataChanged( int row, void *parent )
{
    QModelIndex idx = createIndex( row, 0, parent );
    emit dataChanged( idx, idx );
}

/*!
    emits dataChanged signal.
 */
void BtSettingModel::settingDataChanged(int first, int last, void *parent )
    {
    QModelIndex top = createIndex( first, 0, parent );
    QModelIndex bottom = createIndex( last, 0, parent );
    emit dataChanged( top, bottom );
    }

/*!
 connects all signals of private impl to slots of this
 */
void BtSettingModel::connectModelSignals()
{
    bool ok = connect(d.data(), SIGNAL(settingDataChanged(int,void*)), SLOT(settingDataChanged(int,void*)));
    BTUI_ASSERT_X( ok, "BtSettingModel", "settingDataChanged can't connect" );
    ok = connect(d.data(), SIGNAL(settingDataChanged(int,int,void*)), SLOT(settingDataChanged(int,int,void*)));
    BTUI_ASSERT_X( ok, "BtSettingModel", "settingDataChanged can't connect 2" );
}
