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
* Description:
*
*/


#include "paintcursordocument.h"
#include "paintcursorappui.h"

CPaintCursorDocument::CPaintCursorDocument(CEikApplication& aApp)
        : CAknDocument(aApp)
    {}

CPaintCursorDocument::~CPaintCursorDocument()
    {}

void CPaintCursorDocument::ConstructL()
    {}

CPaintCursorDocument* CPaintCursorDocument::NewL(
    CEikApplication& aApp)     // CPaintCursorApp reference
    {
    CPaintCursorDocument* self = new (ELeave) CPaintCursorDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }

CEikAppUi* CPaintCursorDocument::CreateAppUiL()
    {
    return new (ELeave) CPaintCursorAppUi;
    }

