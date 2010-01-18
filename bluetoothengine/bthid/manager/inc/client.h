/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares main application class.
 *
*/


#ifndef __CLIENT_H
#define __CLIENT_H

#include <e32std.h>

#include "hidkeys.h"  // for TLockKeys
#include "modifier.h"   // for THidModifier
// ----------------------------------------------------------------------

enum TLayoutManagerService
    {
    EKeyEvent,
    EResetDecoder,
    ESetLayout,
    EGetLayout,
    ESetInitialLayout,
    EGetInitialLayout,
    EGetDeviceInfo
    };

// ----------------------------------------------------------------------

class TKeyEventInfo
    {
public:
    inline TKeyEventInfo();
    inline TKeyEventInfo(TInt aHidKey, TInt aUsagePage, TBool aIsKeyDown,
            THidModifier aModifiers, TLockKeys aLockKeys);

    TInt iHidKey;
    TInt iUsagePage;
    TBool iIsKeyDown;
    THidModifier iModifiers;
    TLockKeys iLockKeys;
    };

// ----------------------------------------------------------------------

inline TKeyEventInfo::TKeyEventInfo() :
    iHidKey(0), iUsagePage(0), iIsKeyDown(EFalse), iModifiers(0), iLockKeys(
            EFalse, EFalse)
    {
    // nothing else to do
    }

inline TKeyEventInfo::TKeyEventInfo(TInt aHidKey, TInt aUsagePage,
        TBool aIsKeyDown, THidModifier aModifiers, TLockKeys aLockKeys) :
    iHidKey(aHidKey), iUsagePage(aUsagePage), iIsKeyDown(aIsKeyDown),
            iModifiers(aModifiers), iLockKeys(aLockKeys)
    {
    // nothing else to do
    }

// ----------------------------------------------------------------------

#endif

