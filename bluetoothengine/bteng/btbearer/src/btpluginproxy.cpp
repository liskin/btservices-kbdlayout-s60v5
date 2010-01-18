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
* Description:  ECOM proxy table for this plugin
*
*/



#include <implementationproxy.h>
#include <bldvariant.hrh>
#include "btpluginprovider.h"


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Definition of ECOM interface UID
// ---------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( KFeatureIdBt, CBTBearerPlugin::NewL )
    };


// ---------------------------------------------------------------------------
// ECOM factory method
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }
