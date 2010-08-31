/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BtDeviceDialogWidget class declaration.
*
*/


#ifndef BTDEVICEDIALOGNOTIFWIDGET_H
#define BTDEVICEDIALOGNOTIFWIDGET_H

#include <QVariantMap>

#include <hbpopup.h>
#include <hbdevicedialoginterface.h>
#include <hbdevicedialog.h>
#include <hbnotificationdialog.h>

/*!
    \class BtDeviceDialogNotifWidget
    \brief Widget class with properties setting. 

    BtDeviceDialogQueryWidget, inherited from HbNotificationDialog, 
    implements interface HbDeviceDialogInterface. The Q_Properties
    here are interfaces for caller of HbDeviceDialog to configue
    what to be shown in the widget. 
    
 */
class BtDeviceDialogNotifWidget :
    public QObject, public HbDeviceDialogInterface
{
    Q_OBJECT
    
public:
    BtDeviceDialogNotifWidget(const QVariantMap &parameters);
    ~BtDeviceDialogNotifWidget();
    
    // From base class HbDeviceDialogInterface
    virtual bool setDeviceDialogParameters(const QVariantMap &parameters);
    virtual int deviceDialogError() const;
    virtual void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;

signals: 
    // Required by the framework
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);
    
private:
    
    void processParam(const QVariantMap &parameters);
    bool constructNotifDialog(const QVariantMap &parameters);
    void resetProperties();
        
public slots:
    void NotifClosed(HbAction *action);
    
private:
    Q_DISABLE_COPY(BtDeviceDialogNotifWidget)

    int mLastError;
    int mSendAction;
    bool mShowEventReceived;
    HbNotificationDialog* mNotificationDialog;
};

#endif // BTDEVICEDIALOGNOTIFWIDGET_H
