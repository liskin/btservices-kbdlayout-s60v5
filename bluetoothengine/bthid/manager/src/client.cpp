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
* Description:  This is the implementation of application class
*
*/


#include <e32math.h>

#include "layoutmgr.h"
#include "client.h"
#include "debug.h"
#include "hiduids.h"

// ----------------------------------------------------------------------

EXPORT_C RLayoutManager::RLayoutManager()
    {
    // Nothing else to do
    }

EXPORT_C TInt RLayoutManager::Connect()
    {
    const TVersion KMinServerVersion(1, 0, 0);
    _LIT(KLayoutServerFilename, "LayoutMgr");
    const TUid KServerUid3 =
        {
            LAYOUTMGR_UID
        }
        ;	//This was KServerUid2 before
    const TUidType serverUid(KNullUid, KNullUid, KServerUid3);
    const TUint KDefaultMessageSlots = 3;

    return StartSession(KLayoutServerName, KMinServerVersion,
                        KLayoutServerFilename, serverUid, KDefaultMessageSlots);
    }

// ----------------------------------------------------------------------

EXPORT_C TInt RLayoutManager::KeyEvent(TBool aIsKeyDown,
                                       TInt aHidKey, TInt aUsagePage, TInt aModifiers,
                                       const TLockKeys& aLockKeys, TDecodedKeyInfo& aDecodedKeys) const
    {
    //#ifdef DBG_ACTIVE
    TRACE_INFO( (aIsKeyDown ?
                 _L("RLayoutManager::KeyEvent(down, 0x%x:0x%x, 0x%x, %d, %d]\r\n")
                 : _L("RLayoutManager::KeyEvent(up, 0x%x:0x%x, 0x%x, %d, %d]\r\n"),
                 aIsKeyDown, aUsagePage, aHidKey, aModifiers,
                 aLockKeys.iCapsLock, aLockKeys.iNumLock));
    //#endif

    TPckgBuf<TKeyEventInfo> infoPkg;
    TKeyEventInfo& info = infoPkg();
    info.iIsKeyDown = aIsKeyDown;
    info.iHidKey = aHidKey;
    info.iUsagePage = aUsagePage;
    info.iModifiers = THidModifier(static_cast<TUint8>(aModifiers));
    info.iLockKeys = aLockKeys;

    TPckgBuf<TDecodedKeyInfo> keyPkg;

    TInt result = SendReceive(EKeyEvent, TIpcArgs(&infoPkg, &keyPkg));

    aDecodedKeys.iCount = 0;
    if (result == KErrNone)
        {
        aDecodedKeys = keyPkg();
        }

    return result;
    }

EXPORT_C TInt RLayoutManager::Reset() const
    {
    TRACE_INFO( (_L("RLayoutManager::Reset()")));
    return SendReceive(EResetDecoder);
    }

// ----------------------------------------------------------------------

EXPORT_C TInt RLayoutManager::SetInitialLayout(TInt aCountry,
        TInt aVendor, TInt aProduct) const
    {
    return SendReceive(ESetInitialLayout, TIpcArgs(aCountry, aVendor, aProduct));
    }

// ----------------------------------------------------------------------

EXPORT_C TInt RLayoutManager::SetLayout(TInt aLayoutId) const
    {
    return SendReceive(ESetLayout, TIpcArgs(aLayoutId));
    }

EXPORT_C TInt RLayoutManager::GetLayout(TInt& aLayoutId) const
    {
    TPckg<TInt> layoutPkg(aLayoutId);
    return SendReceive(EGetLayout, TIpcArgs(&layoutPkg));
    }

EXPORT_C TInt RLayoutManager::GetInitialLayout(TInt& aLayoutId) const
    {
    TPckg<TInt> layoutPkg(aLayoutId);
    return SendReceive(EGetInitialLayout, TIpcArgs(&layoutPkg));
    }

// ----------------------------------------------------------------------

EXPORT_C TInt RLayoutManager::GetDeviceInfo(TBool& aIsNokiaSu8,
        TBool& aFoundLayout) const
    {

    TPckg<TBool> isNokiaPkg(aIsNokiaSu8);
    TPckg<TBool> foundLayoutPkg(aFoundLayout);

    return SendReceive(EGetDeviceInfo, TIpcArgs(&isNokiaPkg,&foundLayoutPkg));
    }

