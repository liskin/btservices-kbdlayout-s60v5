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
* Description:  ?Description
*
*/

 
#ifndef BTSENDSERVICEINTERFACE_H
#define BTSENDSERVICEINTERFACE_H

#include <xqserviceprovider.h>

class BtSendManager;

class BTSendServiceInterface : public XQServiceProvider
{
    Q_OBJECT
    
public:
    BTSendServiceInterface( QObject *parent = 0);
    inline ~BTSendServiceInterface();

public slots:
    void send(QVariant data);
    
    
};

inline BTSendServiceInterface::~BTSendServiceInterface()
    {
    }

#endif // BTSENDSERVICEINTERFACE_H
