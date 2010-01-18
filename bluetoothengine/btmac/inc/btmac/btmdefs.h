/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  type declaration.
*
*/


#ifndef BTMDEFS_H
#define BTMDEFS_H

#include <btsdp.h>

// BT Accessory Types
/*
enum TBtmAccType
    {
    EBtmHandsfree,
    EBtmHeadset,
    };
*/
class TBtmService
    {
    public:
        TUint iChannel;
        TUint iService;
        TSdpServRecordHandle iHandle;
    };

const TInt KPowerSaveTimeout = 8500000;

const char KCharCr = '\r';

//Panic codes
enum TBTPanicCode 
    {
    EBTPanicNullPointer = 33300,
    EBTPanicRfcommAlreadyListen,
    EBTPanicRfcommSockInuse,
    EBTPanicObjectInUse,
    EBTPanicObjectUninitialized,
    EBTPanicObjectMismatch,
    EBTPanicRequestOutstanding,
    EBTPanicNullClientRequest,
    EBtPanicPubSubError,
    EBTPanicFeatureNotSupported,
    EBTPanicNullCmdHandler
    };


#endif      // BTMDEFS_H
            