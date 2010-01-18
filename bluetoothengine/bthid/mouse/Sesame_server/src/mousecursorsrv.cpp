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
#include "mousecursorsrv.h"
#include "mousecursorimage.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMouseCursorSrv::CMouseCursorSrv()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CMouseCursorSrv::CMouseCursorSrv()
        : CAnimDll()
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CMouseCursorSrv::CreateInstanceL()
// Used to create an instance of animation server object.
// -----------------------------------------------------------------------------
//
CAnim* CMouseCursorSrv::CreateInstanceL( TInt /* aType */ )
    {
    // The aType variable can be used to set what type of animation object
    // should be created but this example only has 1 type so it is ignored.
    return ( new ( ELeave ) CImage );
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// CreateCAnimDllL()
// Create instance of the animation server dll. Called by windows server
// framework
// -----------------------------------------------------------------------------
//
EXPORT_C CAnimDll* CreateCAnimDllL()
    {
    return ( new ( ELeave ) CMouseCursorSrv );
    }

// End of File
