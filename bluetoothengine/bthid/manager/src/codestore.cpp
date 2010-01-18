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


#include <e32std.h>
#include <e32keys.h>

#include "codestore.h"
#include "debug.h"

// ----------------------------------------------------------------------

CScanCodeStore::CScanCodeStore()
    {
    // nothing else to do
    }

CScanCodeStore* CScanCodeStore::NewL()
    {
    // no second phase construction necessary:
    return new (ELeave) CScanCodeStore;
    }

CScanCodeStore::~CScanCodeStore()
    {
    iKeys.Reset();
    }

// ----------------------------------------------------------------------

TInt CScanCodeStore::Store(TInt aHidKey, TInt aUsagePage, TInt aScanCode)
    {
    TRACE_INFO( (_L("CScanCodeStore::Store(%d, %d, %d)"),
                 aHidKey, aUsagePage, aScanCode));

    return iKeys.Append(TKeyDownInfo(aHidKey, aUsagePage, aScanCode));
    }

TInt CScanCodeStore::Retrieve(TInt aHidKey, TInt aUsagePage)
    {

    TInt result = EStdKeyNull;

    for (TInt i=0; (result == EStdKeyNull) && (i < iKeys.Count()); ++i)
        {
        if ((iKeys[i].iHidKey == aHidKey)  &&
                (iKeys[i].iUsagePage == aUsagePage))
            {
            result = iKeys[i].iScanCode;
            iKeys.Remove(i);
            }
        }

    TRACE_INFO( (_L("CScanCodeStore::Retrieve(%d, %d) = %d"),
                 aHidKey, aUsagePage, result));

    return result;
    }

// ----------------------------------------------------------------------
