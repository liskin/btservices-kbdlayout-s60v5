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
* Description:  implementation of ECom framework
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <implementationproxy.h>
#include "BTAccInfoMainService.h"
#include "BTAccInfoCmdHandler.h"
#include "btaudiomanplugin.h"

// Map the interface implementation UIDs to implementation factory functions
const TImplementationProxy ImplementationTable[] =
    {       
        { { 0x1020897C }, (TProxyNewLPtr) CBTAccInfoMainService::NewL },
        { { 0x1020897D }, (TProxyNewLPtr) CBTAccInfoCmdHandler::NewL },
        { { 0x1020897B }, (TProxyNewLPtr) CBtAudioManPlugin::NewL },
    };

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// ========================== OTHER EXPORTED FUNCTIONS =========================

// Exported proxy for instantiation method resolution.
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }

//  End of File
