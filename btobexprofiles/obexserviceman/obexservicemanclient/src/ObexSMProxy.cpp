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
* Description:  Export ECom interface implementations
*
*/


#include <implementationproxy.h> //ECom header file
#include "ObexSMPlugin.h"

// ---------------------------------------------------------------------------
// An array of TImplementationProxy objects which onnect each 
// implemeation with its instantiation function
// ---------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] = 
    {
	IMPLEMENTATION_PROXY_ENTRY(KFeatureIdSrcs , CObexSMPlugin::NewL)
    };
    
// ---------------------------------------------------------------------------
// Exported proxy function to resolve instantiation methods for an Ecom plug-in DLL
// ---------------------------------------------------------------------------
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt & aTableCount)
    {
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
	return ImplementationTable; 
    }

