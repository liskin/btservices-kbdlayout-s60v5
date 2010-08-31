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
* Description:  The plugin interface of Bluetooth device setting ui 
*
*/

#ifndef BTDEVSETTINGINTERFACE_H
#define BTDEVSETTINGINTERFACE_H

#include <QtPlugin>
#include <btdevsettingglobal.h>
#include <btabstractdevsetting.h>

/*!
    \class BtDeviceSettingInterface
    \brief The class BtDeviceSettingInterface is an interface 
    for creating a BT device setting plugin instance.
 */
class BTDEVSETTINGINTERFACE_EXPORT BtDevSettingInterface
{
public:
    
    /*!
     Destructor of BtDeviceSettingPlugin.
     */
    virtual ~BtDevSettingInterface() {}

    /*!
     Creates a \code BtDeviceSettingPlugin \endcode instance from a 
     specific plugin. 
    
     \param address the BD_ADDR of the remote device which this setting 
     plugin is provided for. Its format is XXXXXXXXXXXX, where X is a 
     hexadecimal digit. Case is insensitive.
     
     \parent the parent of this object.
            
     \return an \code BtDeviceSettingPlugin \endcode instance the ownership of which is 
               transferred to the plugin loader.
     */
    virtual BtAbstractDevSetting *createDevSetting(
            const QString &address, QObject *parent = 0) = 0;
};

Q_DECLARE_INTERFACE(BtDevSettingInterface, "com.nokia.bt.devsettinginterface/1.0");

#endif /* BTDEVSETTINGINTERFACE_H */
