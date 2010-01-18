/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT HID Sockets Panic codes
 *
*/

#ifndef __SOCKETS_PAN__
#define __SOCKETS_PAN__
#include <e32std.h>
_LIT(KPanicBTConnection, "BTConnection");
_LIT(KPanicSocketsEngineWrite, "SocketsEngineWrite");
_LIT(KPanicSocketsEngineRead, "SocketsEngineRead");

/*! Sockets panic codes */
enum TSocketsPanics
    {
    ESocketsBasicUi = 1,
    ESocketsBadStatus,
    ESocketsAppView,
    ESocketsBadState,
    ESocketsUnknownID
    };

#endif // __SOCKETS_PAN__
