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
* Description:  Declares authorisation notifier class.
*
*/

#ifndef BTNOTIFPANIC_H
#define BTNOTIFPANIC_H

_LIT(KBtNotifPanicName, "BtNotif Panic");

enum TBtNotifPanic
    {
    EiDeviceNullWhenCallingGetDeviceFromRegL    = 0,
    EiParamBufferLeakedFromPreviousActivation   = 1,
    EiParamBufferNullInProcessStartParams       = 2,
    EiMessageNullInProcessStartParams           = 3,
    };

template <typename XAny>
struct TBtNotifPanicCodeTypeChecker
    {
    inline static void Check(XAny) { }
    };

#define BTNOTIF_PANIC(CODE) \
    TBtNotifPanicCodeTypeChecker<TBtNotifPanic>::Check(CODE), \
    User::Panic(KBtNotifPanicName, CODE)


#endif // BTNOTIFPANIC_H

