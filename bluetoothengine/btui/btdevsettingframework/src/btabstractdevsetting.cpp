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

#include <btabstractdevsetting.h>

BtAbstractDevSetting::BtAbstractDevSetting(
        const QString &address, QObject *parent)
    : QObject( parent )
{
    mRemote = address;
}

/*!
 Destructor of BtAbstractDevSetting.
 */
BtAbstractDevSetting::~BtAbstractDevSetting()
{
}

/*!
 Gets the BD_ADDR of the remote device which this setting 
 plugin is provided for.
 
 \return the device address of the remote device in hex string
 */
const QString &BtAbstractDevSetting::getRemote() const
{
    return mRemote;
}

/*!
 Tells if this setting plugin can provide a setting UI
 for the target device. If the answer is true, this plugin may be
 requested to create a setting widget by function 
 \code createSettingWidget() \endcode.
 
 \return true if a setting is currently available. false, otherwise.
 
 The default implementation returns false.
 */
bool BtAbstractDevSetting::isSettingAvailable()
{
    return false;
}

/*!
 Gets a \code HbDataForm \endcode instance which consists of the
 setting UI from this specific plugin. The loader of this plugin will
 add this dataform into its setting view if its setting is available at
 the time.
 
 The ownership of this widget is not transferred. That is, the plugin 
 is responsible to free it at plugin destruction.
 
 The default implementation returns 0. 

 \return an \code HbDataForm \endcode instance.
 */
HbDataForm *BtAbstractDevSetting::getSettingWidget()
{
    return 0;
}

/*!
 Informs that device setting is going to foreground. This 
 implies that the \code HbWidget \endcode from function 
 \code createSettingWidget \endcode will be shown if the setting
 from this plugin is available.
 
 The default implementation does nothing.
 */  
void BtAbstractDevSetting::aboutToForeground()
{
}

/*!
 Informs that device setting is going to background.
 
 The default implementation does nothing.
 */
void BtAbstractDevSetting::aboutToBackground()
{
}

/*!
 Informs that device setting is going to close.
 
 The default implementation does nothing.
 */
void BtAbstractDevSetting::aboutToClose()
{
}
