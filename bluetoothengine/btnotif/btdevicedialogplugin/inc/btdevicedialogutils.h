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

#ifndef BTDEVICESDIALOGUTILS_H
#define BTDEVICESDIALOGUTILS_H

#include <QMap>
#include <QList>
#include <QVariant>

// A data item in this model DLL. For example, power state item consists 
// of the information regarding the current Bluetooth power state.
typedef QMap< int, QVariant > BtSendDataItem;

// A category of the model data for specific group
typedef QList< BtSendDataItem > BtSendDataSource;


Q_DECLARE_METATYPE(BtSendDataItem)
Q_DECLARE_METATYPE(BtSendDataSource)

enum DevDataRole {
    NameAliasRole = Qt::DisplayRole, // QVariant::String, the name showing in UI
    ReadableBdaddrRole = Qt::UserRole, // QString, the readable format of a BD_ADDR (BT Device address)
    MajorPropertyRole,  // QVariant::Int, bits of DevMajorProperty
    MinorPropertyRole,  // QVariant::Int, bits of DevMinorProperty
    CoDRole,  // QVariant::Int, the value of Class of Device
    DeviceTypeRole //QVairant::Int, the value of Device Type
};


inline void setMajorProperty(
        BtSendDataItem& qtdev, int prop, bool addto)
{
    if ( addto ) {
         qtdev[MajorPropertyRole] = 
            QVariant( qtdev[MajorPropertyRole].toInt() | prop);
    }
    else {
         qtdev[MajorPropertyRole] = 
            QVariant( qtdev[MajorPropertyRole].toInt() & ~prop);
    }
}


#endif /*BTDEVICESDIALOGUTILS_H*/
