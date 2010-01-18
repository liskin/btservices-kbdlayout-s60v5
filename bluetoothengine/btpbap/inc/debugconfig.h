/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Project configure file.
*
*/

#ifndef OBEXSM_DEBUGCONFIG_H
#define OBEXSM_DEBUGCONFIG_H

#include "prjconfig.h"


/**
 * Custom logging variations.
 */
#ifdef PRJ_FILE_TRACE
_LIT(KLogFile,"pbap.txt");
_LIT(KLogDir,"pbap");
#endif

#ifdef PRJ_ENABLE_TRACE
_LIT(KTracePrefix16, "[pbap] ");
_LIT8(KTracePrefix8, "[pbap] ");
_LIT8(KFuncFormat8, "><%S");
_LIT8(KFuncThisFormat8, "><%S, [0x%08X]");
_LIT8(KFuncEntryFormat8, ">%S");
_LIT8(KFuncEntryThisFormat8, ">%S, [0x%08X]");
_LIT8(KFuncExitFormat8, "<%S");

_LIT(KPanicCategory, "pbap");    
_LIT8(KPanicPrefix8, "PANIC code ");
_LIT8(KLeavePrefix8, "LEAVE code ");
#endif

#endif // OBEXSM_DEBUGCONFIG_H
