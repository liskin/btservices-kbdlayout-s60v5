/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT Bearer plugin implementation.
*
*/



#include "btpluginprovider.h"
#include "btpluginnotifier.h"
#include "debug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTBearerPlugin::CBTBearerPlugin( TLocodBearerPluginParams& aParam )
:   CLocodBearerPlugin( aParam )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//	
void CBTBearerPlugin::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iBTPluginNotifier = CBTPluginNotifier::NewL( Observer(), 
                                                  KCRUidBluetoothPowerState, 
                                                  CBTPluginNotifier::EKeyInt, 
                                                  KBTPowerState );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTBearerPlugin* CBTBearerPlugin::NewL( TLocodBearerPluginParams& aParam )
    {
    CBTBearerPlugin* self = new( ELeave ) CBTBearerPlugin( aParam );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//	
CBTBearerPlugin::~CBTBearerPlugin()
    {
    delete iBTPluginNotifier;
    iBTPluginNotifier = NULL;
    }
