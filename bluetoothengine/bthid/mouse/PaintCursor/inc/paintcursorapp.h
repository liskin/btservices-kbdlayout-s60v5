/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
// ----------------------------------------------------------------------
//
// Project: NOK090 (Bluetooth HID Profile)
//
// "PaintCursorApp.h" - BT Hid Tester Application Header File
//
//
// ----------------------------------------------------------------------
#ifndef PaintCursorAPP_H
#define PaintCursorAPP_H

#include <aknapp.h>

// UID of the application
const TUid KUidPaintCursor =
    {
    0x2001FE5C
    };

/**
 * CPaintCursorApp application class.
 * Provides factory to create concrete document object.
 * 
 */
class CPaintCursorApp : public CAknApplication
    {

public:
    // Functions from base classes
private:

    /**
     * From CApaApplication, creates CPaintCursorDocument document object.
     * @return A pointer to the created document object.
     */
    CApaDocument* CreateDocumentL();

    /**
     * From CApaApplication, returns application's UID (KUidPaintCursor).
     * @return The value of KUidPaintCursor.
     */
    TUid AppDllUid() const;
    };

#endif

