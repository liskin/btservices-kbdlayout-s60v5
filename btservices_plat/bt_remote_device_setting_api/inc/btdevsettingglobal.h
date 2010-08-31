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

#ifndef BTDEVSETTINGGLOBAL_H
#define BTDEVSETTINGGLOBAL_H

#include <QtGlobal>

#ifdef BUILD_BTDEVSETTINGFRAMEWORK
#define BTDEVSETTINGINTERFACE_EXPORT Q_DECL_EXPORT
#else
#define BTDEVSETTINGINTERFACE_EXPORT Q_DECL_IMPORT
#endif

#define BTDEVSETTING_PLUGIN_PATH QLatin1String("/resource/qt/plugins/btdevsetting")

#endif /* BTDEVSETTINGGLOBAL_H */
