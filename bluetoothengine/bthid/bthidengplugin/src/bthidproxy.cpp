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
* Description:  the Plugin entry.
 *
*/


#ifndef BTHIDPROXY_H
#define BTHIDPROXY_H

#include <e32std.h>

#include <ecom/implementationproxy.h>
#include "bthidengplugin.h"
#include "hiduids.h"

const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(KBD_ENGPLUGIN_IMP, CBTHidPlugin::NewL),
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(
        TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

    return ImplementationTable;
    }

#endif // BTHIDPROXY_H
