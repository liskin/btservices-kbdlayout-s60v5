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


#ifndef __MOUSECURSORSRV_H__
#define __MOUSECURSORSRV_H__

// INCLUDES
#include <w32adll.h>

// CLASS DECLARATION

/**
 * CMouseCursorSrv.
 * An instance of CMouseCursorSrv is a 'factory' object which creates a new
 * instance of CAnim
 */
class CMouseCursorSrv : public CAnimDll
    {
public:
    // Constructor

    /**
     * CMouseCursorSrv.
     * Construct an Animation Server dll for mouse cursor
     */
    CMouseCursorSrv();

    // Public destructor not created as this class does not own any data

public:
    // From CAnimDll
    /**
     * CreateInstanceL.
     * Create an instance of the Animation Server image aType.
     * @param aType KAnimationSquare implies "construct a bouncing square"
     */
    IMPORT_C CAnim* CreateInstanceL(TInt aType);

    };

#endif // __MOUSECURSORSRV_H__
// End of File
