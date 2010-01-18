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
* Description:    This is the implementation of document class.
 *
*/

 
// INCLUDE FILES

#include "BTUIDocument.h"
#include "debug.h"
#include "BTUIAppUi.h"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------
// Destructor
// ----------------------------------------------------
//
CBTUIDocument::~CBTUIDocument()
    {
    TRACE_FUNC
  //  delete iModel;
    }

// ----------------------------------------------------
// Symbian OS default constructor can leave.
// ----------------------------------------------------
//
void CBTUIDocument::ConstructL()
    {
    TRACE_FUNC
    }

// ----------------------------------------------------
// Two-phased constructor
// ----------------------------------------------------
//
CBTUIDocument* CBTUIDocument::NewL( CAknApplication& aApp ) // CBTUIApp reference
    {
    CBTUIDocument* self = new (ELeave) CBTUIDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self);

    return self;
    }

// ----------------------------------------------------
// CBTUIDocument::CreateAppUiL()
// constructs CBTUIAppUi
// ----------------------------------------------------
//
CEikAppUi* CBTUIDocument::CreateAppUiL()
    {
    return new (ELeave) CBTUIAppUi;
    }
