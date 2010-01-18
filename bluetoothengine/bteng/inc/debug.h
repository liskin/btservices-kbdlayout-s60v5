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
* Description:  This file defines logging macros for bteng
*
*/



#ifndef BTENG_DEBUG_H
#define BTENG_DEBUG_H


#include "prjconfig.h"
#include <bttypes.h>
#ifdef PRJ_ENABLE_TRACE

#ifdef PRJ_FILE_TRACE
#include <flogger.h>
#else   //PRJ_FILE_TRACE
#include <e32debug.h>
#endif  PRJ_FILE_TRACE


/**
 * Tracing levels definitions.
 * This enables tracing of different levels of information.
 */
_LIT( KBTEngLogFile, "btenglog.txt" );  // Log file
_LIT( KBTEngLogDir, "bt" );             // Log file location


// Print options:
#define KPRINTERROR	 0x00000001         // Print error
#define KPRINTFTRACE 0x00000002         // Print function trace
const TInt KBTEngDebugMask = 0x00000001 | 0x00000002;


/**
 * Handlers below are used for formatting debug prints,
 * to trucate rather than panic the caller.
 */
NONSHARABLE_CLASS( TBTEngOverflowTruncate8 ) : public TDes8Overflow
    {
public:
    void Overflow( TDes8& /*aDes*/ ) {}
    };


NONSHARABLE_CLASS( TBTEngOverflowTruncate16 ) : public TDes16Overflow
    {
public:
    void Overflow( TDes16& /*aDes*/ ) {}
    };


inline void DebugPrint( TRefByValue<const TDesC8> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
#ifdef PRJ_FILE_TRACE
    RFileLogger::WriteFormat( KBTEngLogDir, KBTEngLogFile, EFileLoggingModeAppend, aFmt, list);
#ifdef MEMTRACE
    TInt size;
    User::Heap().AllocSize( size );
    RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, _L( "[BTEng]\t Memory usage: %d; high: %d" ), size, User::Heap().Size() );
#endif  // MEMTRACE
#else   // PRJ_FILE_TRACE
    TBuf8<0x100> logString;
    TBTEngOverflowTruncate8 overflow;
    logString.AppendFormatList( aFmt, list, &overflow );
    TBuf16<0x100> buf16;
    buf16.Copy( logString );
    TRefByValue<const TDesC16> tmpFmt( _L( "%S" ) );
    RDebug::Print( tmpFmt, &buf16 );
#endif  // PRJ_FILE_TRACE
    }


inline void DebugPrint( TRefByValue<const TDesC16> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
#ifdef PRJ_FILE_TRACE
    RFileLogger::WriteFormat( KBTEngLogDir, KBTEngLogFile, EFileLoggingModeAppend, aFmt, list );
#ifdef MEMTRACE
    TInt size;
    User::Heap().AllocSize( size );
    RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, _L( "[BTEng]\t Memory usage: %d; high: %d" ), size, User::Heap().Size() );
#endif  // MEMTRACE
#else   // PRJ_FILE_TRACE
    TBuf16<0x100> logString;
    TBTEngOverflowTruncate16 overflow;
    logString.AppendFormatList( aFmt, list, &overflow );
    RDebug::Print( logString );
#endif   // PRJ_FILE_TRACE
    }


inline TBuf8<0x100> DebugFormat( TRefByValue<const TDesC16> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    TBuf<0x100> buf;
    TBTEngOverflowTruncate16 overflow;
    buf.AppendFormatList( aFmt, list, &overflow );
    TBuf8<0x100> buf8;
    buf8.Copy( buf );
    return buf8;
    }


inline void DebugPrintToHex( const TDesC8& aDes )
    {
    for( TInt i = 0; i < aDes.Length(); ++i )
        {
        DebugPrint( _L( "[BTENG]\t %02x" ), aDes[i] );
        }
    }

inline void DebugPrintBDAddr(const TBTDevAddr& aAddr)
    {
    TBuf<16> addrdes;
    aAddr.GetReadable(addrdes);
    DebugPrint(_L("[BTENG]\t BD addr %S") , &addrdes);   
    }

_LIT( KTracePrefix, "[BTENG]\t " );
_LIT8( KFuncEntryFormat8, "[BTENG]\t %S >>" );
_LIT8( KFuncEntryArgFormat8, "[BTENG]\t %S >> %S" );
_LIT8( KFuncExitFormat8, "[BTENG]\t %S <<" );
_LIT8( KFuncExitResFormat8, "[BTENG]\t %S << %S" );


#define TRACE_INFO( a ) { if( KBTEngDebugMask & KPRINTFTRACE ) DebugPrint a; }

#define TRACE_ERROR( a ) { if( KBTEngDebugMask & KPRINTERROR ) DebugPrint a; }

#define TRACE_FUNC_ENTRY { if( KBTEngDebugMask & KPRINTFTRACE ) { \
                                TPtrC8 ptr( (TUint8*) __PRETTY_FUNCTION__ ); \
                                DebugPrint( KFuncEntryFormat8, &ptr ); } }


#define TRACE_FUNC_EXIT { if( KBTEngDebugMask & KPRINTFTRACE ) { \
                                TPtrC8 ptr( (TUint8*) __PRETTY_FUNCTION__ ); \
                                DebugPrint( KFuncExitFormat8, &ptr ); } }


#define TRACE_FUNC_ARG( a ) { if( KBTEngDebugMask & KPRINTFTRACE ) { \
                                TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
                                TPtrC8 arg( DebugFormat a ); \
                                DebugPrint( KFuncEntryArgFormat8, &func, &arg ); } }


#define TRACE_FUNC_RES( a ) { if( KBTEngDebugMask & KPRINTFTRACE ) { \
                                TPtrC8 func( (TUint8*) __PRETTY_FUNCTION__ ); \
                                TPtrC8 res( DebugFormat a ); \
                                DebugPrint( KFuncExitResFormat8, &func, &res ); } }

#define TRACE_BDADDR( a ) { if( KBTEngDebugMask & KPRINTFTRACE ) DebugPrintBDAddr(a); }

#else // PRJ_ENABLE_TRACE

#define TRACE_INFO( a )

#define TRACE_ERROR( a )

#define TRACE_FUNC_ENTRY

#define TRACE_FUNC_EXIT

#define TRACE_FUNC_ARG( a )

#define TRACE_FUNC_RES( a )

#define TRACE_BDADDR( a )

#endif // PRJ_ENABLE_TRACE

#endif // BTENG_DEBUG_H
