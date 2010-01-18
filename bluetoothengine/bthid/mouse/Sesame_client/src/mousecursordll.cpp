/*
* Copyright (c) 2004 - 2006 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


// INCLUDE FILES
#include "mousecursordll.h"

#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// RMouseCursorDll::RMouseCursorDll()
// Animation client dll construction code all encapsulated in original class.
// -----------------------------------------------------------------------------
//
EXPORT_C RMouseCursorDll::RMouseCursorDll( RWsSession& aSession )
        : RAnimDll( aSession )
    {
    DBG(RDebug::Print(_L("RMouseCursorDll::RMouseCursorDll")));
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// End of File
