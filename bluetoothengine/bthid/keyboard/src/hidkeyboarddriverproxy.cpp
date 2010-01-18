/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
#include "hiduids.h"

// User includes
//
#include "keyboard.h"

// Constants
//
const TImplementationProxy KHidKeyboardDriverProxy[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( KEYBOARD_DRIVER_IMP, CHidKeyboardDriver::NewL )
    };

// ---------------------------------------------------------------------------
// ImplementationGroupProxy
// Gate/factory function
//
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
        TInt& aTableCount)
    {
    aTableCount = sizeof(KHidKeyboardDriverProxy)
            / sizeof(TImplementationProxy);
    return KHidKeyboardDriverProxy;
    }
