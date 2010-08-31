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

#ifndef BTUIMODELTYPES_H
#define BTUIMODELTYPES_H

#include <qglobal.h>
#include <QMap>
#include <QVariant>
#include <QList>

// A data item in this model DLL. For example, power state item consists 
// of the information regarding the current Bluetooth power state.
typedef QMap< int, QVariant > BtuiModelDataItem;

// A category of the model data for specific group
typedef QList< BtuiModelDataItem > BtuiModelDataSource;

Q_DECLARE_METATYPE(BtuiModelDataItem)
Q_DECLARE_METATYPE(BtuiModelDataSource)

#ifdef BUILD_BTUIMODEL
#define BTUIMODEL_IMEXPORT Q_DECL_EXPORT
#else
#define BTUIMODEL_IMEXPORT Q_DECL_IMPORT
#endif


#endif // BTUIMODELTYPES_H
