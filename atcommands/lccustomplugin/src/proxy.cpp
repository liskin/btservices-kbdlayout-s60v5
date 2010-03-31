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
* Description:  Export ECom interface implementations
*
*/


#ifndef REFERENCE_ATEXT_H
#define REFERENCE_ATEXT_H

#include <e32base.h>
#include <ecom/implementationproxy.h>
#include "lccustomplugin.h"

// ---------------------------------------------------------------------------
// An array of TImplementationProxy objects which onnect each
// implemeation with its instantiation function
// ---------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
	{
	IMPLEMENTATION_PROXY_ENTRY(0x20021375, CLcCustomPlugin::NewL),  // DUN
	IMPLEMENTATION_PROXY_ENTRY(0x20021376, CLcCustomPlugin::NewL)   // HFP
	};

// ---------------------------------------------------------------------------
// Exported proxy function to resolve instantiation methods for an Ecom
// plug-in DLL
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

	return ImplementationTable;
	}

#endif
