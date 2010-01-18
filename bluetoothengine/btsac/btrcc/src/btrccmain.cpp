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
* Description:  Contains the DLL entry point.
*
*/

#ifndef BTRCCPLUGINECOMIMPL_H
#define BTRCCPLUGINECOMIMPL_H

// INCLUDE FILES
#include <e32std.h>
#include <implementationproxy.h>
#include "btrccPlugin.h"
#include "btrccPluginImplUid.h"

// ================= OTHER EXPORTED FUNCTIONS ==============
// Define the private interface UIDs
const TImplementationProxy ImplementationTable[] =
	{
	IMPLEMENTATION_PROXY_ENTRY(KBTRCCPluginImplUid, CBTRCCPlugin::NewL),
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

	return ImplementationTable;
	}

#endif // BTRCCPLUGINECOMIMPL_H

//  End of File  
