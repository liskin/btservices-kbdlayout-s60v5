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
* Description: 
*        ECOM proxy table for this plugin
*
*/


// System includes
//
#include <e32std.h>
#include <ecom/implementationproxy.h>

// User includes
//
#include "bthidsettingview.h"
#include "hiduids.h"

// Constants
//
const TImplementationProxy KBtHidPluginImplementationTable[] =
    {
        // UPDATE THIS - Uid for plugin implementation:

        //This impl is for BtuiPluginInterface.
        IMPLEMENTATION_PROXY_ENTRY( KBDSETTINGS_UIPLUGIN_IMP,	CBtHidSettingView::NewBtuiPluginL ),
    };


// ---------------------------------------------------------------------------
// ImplementationGroupProxy
// Gate/factory function
//
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(KBtHidPluginImplementationTable)
                  / sizeof(TImplementationProxy);
    return KBtHidPluginImplementationTable;
    }
