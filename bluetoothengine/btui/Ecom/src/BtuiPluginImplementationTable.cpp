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
* Description:   ECOM proxy table for this plugin
*
*/

#include <e32std.h>
#include <implementationproxy.h>

#include "BTUIMainView.h"
#include "BTUIPairedDevicesView.h"
#include "BTUIBlockedDevicesView.h"


const TImplementationProxy KBtuiPluginImplementationTable[] = 
	{
	// UPDATE THIS - Uid for plugin implementation:
	//This impl is for GS FW, in this case mainView will construct BTUI Model.
	IMPLEMENTATION_PROXY_ENTRY( 0x1020742C,	CBTUIMainView::NewGsPluginL ), 	

	//Following two are for BTUI App, in this case mainView won't construct BTUI Model, 
	//BTUI's AppUi will do it and pass the pointer of Model to both views.
	IMPLEMENTATION_PROXY_ENTRY( 0x1020745C,	CBTUIMainView::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( 0x1020745D,	CBTUIPairedDevicesView::NewL ),
	IMPLEMENTATION_PROXY_ENTRY( 0x20002780,	CBTUIBlockedDevicesView::NewL )
	};

// ---------------------------------------------------------------------------
// ImplementationGroupProxy
// Gate/factory function
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(KBtuiPluginImplementationTable) 
        / sizeof(TImplementationProxy);
	return KBtuiPluginImplementationTable;
	}
