/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   This is the implementation of application class.
 *
*/


// INCLUDE FILES

#include "btui.h"       // Global declarations
#include "BTUIApp.h"
#include "BTUIDocument.h"

// ---------------------------------------------------------
// CBTUIApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CBTUIApp::AppDllUid() const
    {
    return KBtuiUid3;
    }

// ---------------------------------------------------------
// CBTUIApp::CreateDocumentL()
// Creates CBTUIDocument object
// ---------------------------------------------------------
//
CApaDocument* CBTUIApp::CreateDocumentL()
    {
    return CBTUIDocument::NewL( *this );
    }

#include <eikstart.h>

// ---------------------------------------------------------
// NewApplication()
// ---------------------------------------------------------
LOCAL_C CApaApplication* NewApplication()
    {
    return new CBTUIApp;
    }
// ---------------------------------------------------------
// E32Main()
// ---------------------------------------------------------
GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }    

  


