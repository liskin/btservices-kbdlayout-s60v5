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

#ifndef BTSETTINGMODEL_H
#define BTSETTINGMODEL_H

#include <qglobal.h>
#include <QAbstractItemModel>
#include <QSharedPointer>
#include <btuimodeltypes.h>

class BtSettingModelPrivate;

/*!
    \class BtSettingModel
    \brief The data model provided to Bluetooth UIs in QT

    BtSettingModel provides APIs for accessing BT local Bluetooth
    settings. In addition, signals from this
    model are provided for being informed of data update. 
    
    This model is in one dimension (n rows * 1 column), i.e.,

          row 0 (local device name)
          row 1 (power state)
          ...
    
    The data in this model is non-modifiable from the user interface, 
    determined by the characteristics of Bluetooth, the underline BT software 
    services and the BT application requirements. 
 */

class BTUIMODEL_IMEXPORT BtSettingModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS( LocalSettingRowId LocalSettingDataRole )

public:

    //Q_DECLARE_FLAGS(Rows, BtSettingModelRow)

    /**
     * child row identifiers of the local setting row
     */
    enum LocalSettingRowId {
        LocalBtNameRow = 0,
        PowerStateRow ,
        VisibilityRow,
        SimAccessProfileRow,
        AllowedInOfflineRow,
        LocalSettingRowCount,
    };
    //Q_DECLARE_FLAGS(BtSettingModelLocalSettings, BtSettingModelLocalSettingColumn)
    
    /**
     * Data roles of the items in the local setting row
     */
    enum LocalSettingDataRole {
        SettingNameRole = Qt::WhatsThisRole,
        settingDisplayRole = Qt::DisplayRole,
        SettingValueRole = Qt::EditRole,
        SettingValueParamRole = Qt::UserRole + 1,  // e.g., temp visibility time     
    };
    
public:
    
    explicit BtSettingModel( QObject *parent = 0 );
    
    explicit BtSettingModel( const BtSettingModel &model, QObject *parent = 0 );
    
    virtual ~BtSettingModel();
    
    // from QAbstractItemModel
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QModelIndex parent( const QModelIndex &child ) const;
    
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual QMap<int, QVariant> itemData( const QModelIndex & index ) const;
    
private slots:

    void settingDataChanged( int row, void *parent );
    
    void settingDataChanged( int first, int last, void *parent );
    
private:
    
    void connectModelSignals();
    
private:
    QSharedPointer<BtSettingModelPrivate> d;

};

#endif
