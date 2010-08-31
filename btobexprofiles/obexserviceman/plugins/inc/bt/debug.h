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
* Description: 
*     Central place for nice debug-type macros & functions
*
*/


#ifndef SRCS_DEBUG_H
#define SRCS_DEBUG_H

#ifdef _DEBUG

// Enable this to enable memory tracing to SRCS
//#define MEMTRACE

// Following define is to enable OOM situations in SRCS
// NOTE! SHOULD NEVER BE IN RELEASES
//#define TEST_OOM

#ifdef __WINS__

// File logging for WIS
#define __FLOGGING__

#else

// Logging with RDebug for target HW
#define __CLOGGING__
//#define __FLOGGING__


#endif //__WINS__

#endif

#if defined ( __FLOGGING__ )

_LIT(KLogFile,"Srcslog.txt");
_LIT(KLogDir,"SRCS");

#include <f32file.h>
#include <flogger.h>

#define FLOG(a) {FPrint(a);}

#define FLOGHEX(value, len) {RFileLogger::HexDump(KLogDir, KLogFile, EFileLoggingModeAppend, "", " ",value, len);}

#define FTRACE(a) {a;}
// Declare the FPrint function

inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
{
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);

    // If memory tracing is activated.
#ifdef MEMTRACE
    TInt size;
    User::Heap().AllocSize(size);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, _L("[SRCS]\tmemory\tMemory usage: %d high: %d"), size, User::Heap().Size());
#endif
}

inline void FHex(const TUint8* aPtr, TInt aLen)
{
    RFileLogger::HexDump(KLogDir, KLogFile, EFileLoggingModeAppend, 0, 0, aPtr, aLen);
}

inline void FHex(const TDesC8& aDes)
{
    FHex(aDes.Ptr(), aDes.Length());
}

// RDebug logging
#elif defined(__CLOGGING__)

#include <e32svr.h>

#define FLOG(a) {RDebug::Print(a);}

#define FLOGHEX(a)

#define FTRACE(a) {a;}

// Declare the FPrint function

inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
{
    VA_LIST list;
    VA_START(list,aFmt);
    TInt tmpInt = VA_ARG(list, TInt);
    TInt tmpInt2 = VA_ARG(list, TInt);
    TInt tmpInt3 = VA_ARG(list, TInt);
    VA_END(list);
    RDebug::Print(aFmt, tmpInt, tmpInt2, tmpInt3);
}


#else   // No loggings --> reduced code size
#define FLOG(a)
#define FLOGHEX(a)
#define FTRACE(a)

#endif //_DEBUG

#endif // SRCS_DEBUG_H
