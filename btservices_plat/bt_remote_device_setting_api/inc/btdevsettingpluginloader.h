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

#ifndef BTDEVSETTINGPLUGINLOADER_H
#define BTDEVSETTINGPLUGINLOADER_H

#include <QtGlobal>
#include <btdevsettingglobal.h>
#include <QList>

class QString;
class QStringList;
class BtDevSettingInterface;

class BTDEVSETTINGINTERFACE_EXPORT BtDevSettingPluginLoader
{
public:

    /*!
       Load all setting interfaces.
       
       \return the list of plugin interface.
     */
    static QList<BtDevSettingInterface*> loadDevSettingInterfaces();

};

#endif /* BTDEVSETTINGPLUGINLOADER_H */
