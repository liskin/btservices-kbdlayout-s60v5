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
* Description:  Declares authorisation notifier class.
*
*/

#ifndef BTACCPANIC_H
#define BTACCPANIC_H

_LIT(KBtAccPanicName, "BtAcc Panic");

enum TBtAccPanic
    {
    ENoShutdownTimer = 0,
    EMaxNumberOfConflictsExceeded = 1,
    };

template <typename XAny>
struct TBtAccPanicCodeTypeChecker
    {
    inline static void Check(XAny) { }
    };

#define BTACC_PANIC(CODE) \
    TBtAccPanicCodeTypeChecker<TBtAccPanic>::Check(CODE), \
    User::Panic(KBtAccPanicName, CODE)


#endif // BTACCPANIC_H

