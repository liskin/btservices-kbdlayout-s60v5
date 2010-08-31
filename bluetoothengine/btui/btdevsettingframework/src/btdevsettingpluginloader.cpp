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
#include <btdevsettingpluginloader.h>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <btdevsettinginterface.h>
#include <btdevsettingglobal.h>

/*!
    \class BtDevSettingPluginLoader
    \brief The BtDevSettingPluginLoader class loads BT device setting plugins.
 */

#ifdef WIN32
    #define PLUGINFILE_SUFFIX "dll"
#else
    #define PLUGINFILE_SUFFIX "qtplugin"
#endif


static QStringList getDrives()
{
    static QStringList drives;
    if (drives.empty()) {
#ifdef WIN32
        drives.append("C:");
#else
        QFileInfoList fileInfoList = QDir::drives();
        foreach(const QFileInfo &fileInfo,fileInfoList) {
            QString str = fileInfo.filePath();
            if (str.length() > 2) {
                str = str.left(2);
            }
            drives.append(str);
        }
#endif
    }
    return drives;
}

static QStringList directoriesFromAllDrives(const QString &baseDir)
{
    QStringList dirs;
    QStringList drives = getDrives();
    foreach(const QString &drive,drives) {
        QString dir = drive + baseDir + QDir::separator();
        if (QFileInfo(dir).exists()) {
            dirs.append(dir);
        }
    }
    return dirs;
}

static BtDevSettingInterface* loadPluginInterface(const QFileInfo &pluginFile)
{    
    QPluginLoader loader(pluginFile.absoluteFilePath());
    BtDevSettingInterface *plugin = 
            qobject_cast<BtDevSettingInterface*> (loader.instance());
    if (!plugin) {
        loader.unload();
    }
    return plugin;
}

/*!
   Load all setting interfaces.
   
   \return the list of plugin interface.
 */
QList<BtDevSettingInterface*> BtDevSettingPluginLoader::loadDevSettingInterfaces()
{
    QList<BtDevSettingInterface*> interfaces;
    
    static QStringList pluginDirs;
    if (pluginDirs.empty()) {
        pluginDirs = directoriesFromAllDrives(BTDEVSETTING_PLUGIN_PATH);
    }
    foreach(const QString &dirStr,pluginDirs) {
        QDir pluginDir( dirStr );
        QFileInfoList fileInfoList = pluginDir.entryInfoList( QDir::Files );
        foreach(const QFileInfo &fileInfo,fileInfoList) {
            int diff = fileInfo.suffix().compare(PLUGINFILE_SUFFIX,Qt::CaseInsensitive);
            if (!diff) {
                BtDevSettingInterface* settingIf = loadPluginInterface(fileInfo);
                if ( settingIf ) {
                    interfaces.append( settingIf );
                }
            }
        }
    }
    
    return interfaces;
}

