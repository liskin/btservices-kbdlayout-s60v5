/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This file defines logging macros for ObexUtils
*
*/


#ifndef OBEXUTILS_DEBUG_H
#define OBEXUTILS_DEBUG_H


#ifdef _DEBUG

#include <e32svr.h>
#include <e32std.h>
#include <f32file.h>
#include <flogger.h>

_LIT(KLogFile,"obexutils.txt");
_LIT(KLogDirFullName,"c:\\logs\\");
_LIT(KLogDir,"obexutils");

// Declare the FPrint function
inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);
    }

// ===========================================================================
#ifdef __WINS__     // File logging for WINS
// ===========================================================================
#define FLOG(a)   { FPrint(a); }
#define FTRACE(a) { a; }
// ===========================================================================
#else               // RDebug logging for target HW
// ===========================================================================
#define FLOG(a) { RDebug::Print(a);  }
#define FTRACE(a) { a; }
#endif //__WINS__

// ===========================================================================
#else // // No loggings --> Reduced binary size
// ===========================================================================
#define FLOG(a)
#define FTRACE(a)

#endif // _DEBUG


#endif // OBEXUTILS_DEBUG_H

// End of File
