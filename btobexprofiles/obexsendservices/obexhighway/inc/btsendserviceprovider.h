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
 * Description:
 * class to manage differnt messaging views.
 *
 */

#ifndef BTSENDSERVICEPROVIDER_H
#define BTSENDSERVICEPROVIDER_H

#include <e32base.h>
#include <qlist.h>
#include <qvariant.h>

class CBTServiceAPI;
class CBTSSSendListHandler;


class CBtSendServiceProvider: public CBase
    {
public:
    /**
     * constructor
     */
    static CBtSendServiceProvider* NewL();
    
    /**
     * Destructor.
     */
    ~CBtSendServiceProvider();
    
    
    int send(const QList<QVariant> &arguments);
private:
    CBtSendServiceProvider();    
    void ConstructL();
    
public:
    CBTSSSendListHandler*    iConverter;
    CBTServiceAPI*  iBTSendingService;
     };

#endif /* BTSENDSERVICEPROVIDER_H */
