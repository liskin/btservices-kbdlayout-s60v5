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


#ifndef __CODESTORE_H
#define __CODESTORE_H

#include <e32std.h>
#include <e32base.h>

// ----------------------------------------------------------------------

class CScanCodeStore : public CBase
    {
public:
    static CScanCodeStore* NewL();

    TInt Store(TInt aHidKey, TInt aUsagePage, TInt aScanCode);
    TInt Retrieve(TInt aHidKey, TInt aUsagePage);

    virtual ~CScanCodeStore();

private:
    CScanCodeStore();

private:
    class TKeyDownInfo
        {
    public:
        inline TKeyDownInfo();
        inline TKeyDownInfo(TInt aHidKey, TInt aUsagePage, TInt aScanCode);

        TInt iHidKey;
        TInt iUsagePage;
        TInt iScanCode;
        };

    RArray<TKeyDownInfo> iKeys;
    };

// ----------------------------------------------------------------------

inline CScanCodeStore::TKeyDownInfo::TKeyDownInfo() :
    iHidKey(0), iUsagePage(0), iScanCode(0)
    {
    // nothing else to do
    }

inline CScanCodeStore::TKeyDownInfo::TKeyDownInfo(TInt aHidKey,
        TInt aUsagePage, TInt aScanCode) :
    iHidKey(aHidKey), iUsagePage(aUsagePage), iScanCode(aScanCode)
    {
    // nothing else to do
    }

// ----------------------------------------------------------------------

#endif

