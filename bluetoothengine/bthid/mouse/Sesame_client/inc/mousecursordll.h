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


#ifndef __MOUSECURSOR_DLL_H__
#define __MOUSECURSOR_DLL_H__

// INCLUDES
#include <w32adll.h>

// CLASS DECLARATION

/**
 * RMouseCursorDll.
 * An instance of RMouseCursor is an Animation Client DLL, used to load
 * and destroy the Animation Server
 */
class RMouseCursorDll : public RAnimDll
    {
public:

    /**
     * RMouseCursorDll.
     * Construct an Animation Client DLL object for use with the
     * aSession window server session.
     * @param aSession the window server session to use
     */
    IMPORT_C RMouseCursorDll(RWsSession& aSession);
    };

#endif //__MOUSECURSOR_DLL_H__
// End of File
