/*
* Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


// INCLUDE FILES
#include <hal.h>
#include "btrfsplugin.h"
#include "debug.h" // bteng debug utility

// LOCAL CONSTANTS
_LIT(KScriptPath, "z:\\resource\\btrfs.txt");

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CBTRFSPlugin::CBTRFSPlugin() : CRFSPlugin()
    {
    }

CBTRFSPlugin::CBTRFSPlugin(TAny* /*aInitParams*/) : CRFSPlugin()
    {
    }

// Destructor
CBTRFSPlugin::~CBTRFSPlugin()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CBTRFSPlugin* CBTRFSPlugin::NewL(TAny* aInitParams)
    {
    TRACE_FUNC_ENTRY
    CBTRFSPlugin* self = new (ELeave) CBTRFSPlugin(aInitParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    TRACE_FUNC_EXIT
    return self;
    }


// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CBTRFSPlugin::ConstructL()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CBTRFSPlugin::RestoreFactorySettingsL( const TRfsReason /*aType*/ )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CBTRFSPlugin::GetScriptL( const TRfsReason aType, TDes& aPath )
    {
    TRACE_FUNC_ENTRY
    ASSERT(aType == EDeepRfs || aType == ENormalRfs); // ECOM default data specifies only these two types
    if(aType == EDeepRfs)
        {
        aPath.Copy(KScriptPath);
        }
    TRACE_FUNC_EXIT
    }

void CBTRFSPlugin::ExecuteCustomCommandL( const TRfsReason /*aType*/,
                                        TDesC& /*aCommand*/ )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// End of file
