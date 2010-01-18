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
// Project: Bluetooth HID Profile
//
// "PaintCursorApp.cpp" - BT Hid PaintCursor Application Source File
//
//
// ----------------------------------------------------------------------

#include    "paintcursorapp.h"
#include    "paintcursordocument.h"

TUid CPaintCursorApp::AppDllUid() const
    {
    return KUidPaintCursor;
    }

CApaDocument* CPaintCursorApp::CreateDocumentL()
    {
    return CPaintCursorDocument::NewL(*this);
    }

#include <eikstart.h>

EXPORT_C CApaApplication* NewApplication()
    {
    return new CPaintCursorApp;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }

