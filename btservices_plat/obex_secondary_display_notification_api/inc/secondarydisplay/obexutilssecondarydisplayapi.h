/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Contains dialog index for cover UI.
*
*/


#ifndef OBEXUTILS_SECONDARY_DISPLAY_API_H
#define OBEXUTILS_SECONDARY_DISPLAY_API_H

//  INCLUDES
#include  <e32std.h>

// Category
const TUid KObexUtilsCategory = { 0x101F86AF };

/**
* Enumerates types.
*/
enum TSecondaryDisplayObexUtilsDialogs
    {
    ECmdNone,
    ECmdIrConnecting,
    ECmdIrCannotEstablish,
    ECmdIrSendingData,
    ECmdIrSendingFailed,
    ECmdIrDataSent,
    ECmdBtConnecting,
    ECmdBtSendingData,
    ECmdBtDevNotAvail,
    ECmdBtFailedToSend,
    ECmdBtDataSent,
    ECmdBtNotReceiveOne,
    ECmdOutOfMemory,
    ECmdBtSentImageNumber,
    ECmdBtNotReceiveAny,
    ECmdBtNotSendAll,
    ECmdBtNotAllSupported,
    ECmdBtNotAllSupportedMixed,
    ECmdBtConnectingWait,
    ECmdIrConnectingWait,
    ECmdBtPrintingWait,
    ECmdSendingProgress
    };
    
#endif      // OBEXUTILS_SECONDARY_DISPLAY_API_H

// End of File
