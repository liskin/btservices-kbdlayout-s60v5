/*
* Copyright (c) 2004-2005 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef BT_SAP_DEBUG_H
#define BT_SAP_DEBUG_H

#ifdef _DEBUG

// Debugging options

#define KBTSAP_TRACE_ERROR     0x00000001 // Print errors traces
#define KBTSAP_TRACE_FUNCTIONS 0x00000002 // Print function traces
#define KBTSAP_TRACE_STM       0x00000004 // Print state machine traces
#define KBTSAP_TRACE_MEMORY    0x00000008 // Print memory traces
#define KBTSAP_TRACE_INFO      0x00000010 // Print informational traces

// Add desired tracing levels below
const TInt KBTSapDebugMask = KBTSAP_TRACE_ERROR     |
                             KBTSAP_TRACE_FUNCTIONS |
                             KBTSAP_TRACE_STM       |
                             KBTSAP_TRACE_MEMORY    |
                             KBTSAP_TRACE_INFO;

#ifdef __WINS__
    // File logging for WINS
    #define __FLOGGING__
#else
    // RDebug logging for target HW
    #define __CLOGGING__
#endif

#ifdef __FLOGGING__

#include <e32std.h>
#include <f32file.h>
#include <flogger.h>

_LIT(KLogFile,"BTSapLog.txt");
_LIT(KLogDir,"BT");

inline void TraceMem()
    {
#ifdef MEMTRACE
    TInt size;
    User::Heap().AllocSize(size);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, _L("Memory usage: %d high: %d"), size, User::Heap().Size());
#endif
    }

inline void BTSapPrintTrace(const TRefByValue<const TDesC8> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);
    TraceMem();
    }

inline void BTSapPrintTrace(const TRefByValue<const TDesC16> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);
    TraceMem();
    }

#else    // __CLOGGING__

#include <e32svr.h>

inline void BTSapPrintTrace(const TRefByValue<const TDesC16> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    TInt tmpInt = VA_ARG(list, TInt);
    TInt tmpInt2 = VA_ARG(list, TInt);
    TInt tmpInt3 = VA_ARG(list, TInt);
    VA_END(list);
    RDebug::Print(aFmt, tmpInt, tmpInt2, tmpInt3);
    }

inline void BTSapPrintTrace(TRefByValue<const TDesC8> aFmt, ...)
    {
	VA_LIST list;
	VA_START(list, aFmt);
    TBuf8<256> buf8;
	buf8.AppendFormatList(aFmt, list);

    TBuf16<256> buf16(buf8.Length());
    buf16.Copy(buf8);

    TRefByValue<const TDesC> tmpFmt(_L("%S"));
    RDebug::Print(tmpFmt, &buf16);
    }

#endif // __CLOGGING__

inline void BTSapPrintHex(const TRefByValue<const TDesC8> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list, aFmt);
    TDesC8* aValue = VA_ARG(list, TDesC8*);
    VA_END(list);

    TBuf8<85> buf8;
    for (TInt i = 0; i < aValue->Length(); i ++)
        {
        if (buf8.Length() > 80) break;
        buf8.AppendNumFixedWidth((*aValue)[i], EHex, 2);
        buf8.Append(0x20);
        }

    BTSapPrintTrace(aFmt, &buf8);
    }

#define BTSAP_TRACE_OPT(a,p) { if((KBTSapDebugMask) & (a)) (p); }

#else // NO DEBUG

#define BTSAP_TRACE_OPT(a,p)

#endif // _DEBUG

#endif // BT_SAP_DEBUG_H

// End of File
