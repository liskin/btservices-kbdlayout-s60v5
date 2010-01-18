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
* Description:  This file defines logging macros for BT Notifiers
*
*/


#ifndef BTNOTIF_DEBUG_H
#define BTNOTIF_DEBUG_H


#ifdef _DEBUG

#include <e32svr.h>
#include <e32std.h>

// ===========================================================================
#ifdef __WINS__     // File logging for WINS
// ===========================================================================
#define FLOG(a)   { FPrint(a); }
#define FTRACE(a) { a; }

#include <f32file.h>
#include <flogger.h>

_LIT(KLogFile,"btnotiflog.txt");
_LIT(KLogDirFullName,"c:\\logs\\bt\\");
_LIT(KLogDir,"BT");

// Declare the FPrint function
inline void FPrint(const TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list);
    }

inline void FPrint(TRefByValue<const TDesC8> aFmt, ...)
    {

    VA_LIST list;
    VA_START(list, aFmt);
    TBuf8<256> buf8;
    buf8.AppendFormatList(aFmt, list);

    TBuf16<256> buf16(buf8.Length());
    buf16.Copy(buf8);
    RFileLogger::WriteFormat(KLogDir, KLogFile, EFileLoggingModeAppend, buf16);
    }

// ===========================================================================
#else               // RDebug logging for target HW
// ===========================================================================
#define FLOG(a) { RDebug::Print(a);  }
#define FTRACE(a) { a; }

inline void FPrint(const TRefByValue<const TDesC16> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list,aFmt);
    TInt tmpInt = VA_ARG(list, TInt);
    TInt tmpInt2 = VA_ARG(list, TInt);
    TInt tmpInt3 = VA_ARG(list, TInt);
    VA_END(list);
    RDebug::Print(aFmt, tmpInt, tmpInt2, tmpInt3);
    }

inline void FPrint(TRefByValue<const TDesC8> aFmt, ...)
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

#endif //__WINS__

// ===========================================================================
#else // // No loggings --> Reduced binary size
// ===========================================================================
#define FLOG(a)
#define FTRACE(a)

#endif // _DEBUG


#endif // BTNOTIF_DEBUG_H

// End of File
