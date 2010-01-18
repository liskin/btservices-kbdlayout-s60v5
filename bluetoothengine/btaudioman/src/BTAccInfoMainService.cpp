/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Loaded by Accessory Server to allow this component to initialize
*
*/


// INCLUDE FILES
#include "BTAccInfoMainService.h"
#include "debug.h"

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTAccInfoMainService::CBTAccInfoMainService
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTAccInfoMainService::CBTAccInfoMainService()
    {
    TRACE_FUNC;
    }

// -----------------------------------------------------------------------------
// CBTAccInfoMainService::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTAccInfoMainService* CBTAccInfoMainService::NewL()
    {
    return new (ELeave) CBTAccInfoMainService();
    }
    
// Destructor
CBTAccInfoMainService::~CBTAccInfoMainService()
    {
    TRACE_FUNC;
    }

// -----------------------------------------------------------------------------
// CBTAccInfoMainService::StartL
// -----------------------------------------------------------------------------
//
TInt CBTAccInfoMainService::StartL()
    {
    TRACE_FUNC;
    // Synchronous ASY : Signal
    Signal();
    return KErrNone;
    }

//  End of File



