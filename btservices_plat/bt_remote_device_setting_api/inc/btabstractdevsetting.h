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

#ifndef BTABSTRACTDEVSETTING_H
#define BTABSTRACTDEVSETTING_H

#include <HbDataForm>
#include <btdevsettingglobal.h>

/*!
    \class BtAbstractDevSetting
    \brief The class BtAbstractDevSetting is the base class 
    for offering a UI to manage certain settings of 
    a remote Bluetooth device, e.g., accessory and keyboard settings. 
 */
class BTDEVSETTINGINTERFACE_EXPORT BtAbstractDevSetting : public QObject
{
    Q_OBJECT
    
public:
    
    /*!
     Constructs a device setting plugin entry.
     
     \param address the BD_ADDR of the remote device which this setting 
     plugin is provided for. Its format is XXXXXXXXXXXX, where X is a 
     hexadecimal digit. Case is insensitive.
     \parent the parent of this object.
     */
    explicit BtAbstractDevSetting(const QString &address, QObject *parent = 0 );
    
    /*!
     Destructor of BtAbstractDeviceSetting.
     */
    virtual ~BtAbstractDevSetting();
    
    /*!
     Gets the BD_ADDR of the remote device which this setting 
     plugin is provided for.
     
     \return the device address of the remote device in hex string
     */
    const QString &getRemote() const;
    
    /*!
     Tells if this setting plugin can provide a setting UI
     for the target device. If the answer is true, this plugin may be
     requested to create a setting widget by function 
     \code createSettingWidget() \endcode.
     
     \return true if a setting is currently available. false, otherwise.
     
     The default implementation returns false.
     */
    virtual bool isSettingAvailable();
    
    /*!
     Gets a \code HbDataForm \endcode instance which consists of the
     setting UI from this specific plugin. The loader of this plugin will
     add this dataform into its setting view if its setting is available at
     the time.
     
     The ownership of this widget is transferred to the plugin loader.
     
     The default implementation returns 0. 

     \return an \code HbDataForm \endcode instance.
     */
    virtual HbDataForm *getSettingWidget();
    
    /*!
     Informs that device setting is going to foreground. This 
     implies that the \code HbWidget \endcode from function 
     \code createSettingWidget \endcode will be shown if the setting
     from this plugin is available.
     
     The default implementation does nothing.
     */
    virtual void aboutToForeground();
    
    /*!
     Informs that device setting is going to background.
     
     The default implementation does nothing.
     */
    virtual void aboutToBackground();

    /*!
     Informs that device setting is going to close.
     
     The default implementation does nothing.
     */
    virtual void aboutToClose();
    
signals:
    
    /*!
    The signal the plugin should emit for informing that the
    availability of this setting is changed.
    
    If the setting becomes available, the owner of this plugin may
    request the plugin to create a setting widget.
    
    If the setting becomes unavailable, the owner of this plugin may close and 
    destroy the setting widget.
    
    \param setting the pointer of the setting whose status has changed.
    
    \param available true if the setting is available; false otherwise.
    
    \sa isSettingAvailable().
     */
    void settingAvailabilityChanged(BtAbstractDevSetting *setting, bool available);
    
private:
    
    /*!
     The remote device address in format XXXXXXXXXXXX where X is a 
     hexadecimal digit.
     */
    QString mRemote;
};

#endif /* BTABSTRACTDEVSETTING_H */
