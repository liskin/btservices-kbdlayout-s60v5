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
* Description:  Logging definition
*
*/



#ifndef PRJ_LOGGING_H
#define PRJ_LOGGING_H

#include "debugconfig.h"

#ifdef PRJ_ENABLE_TRACE

#ifdef PRJ_FILE_TRACE
#include <flogger.h>
#else
#include <e32debug.h>
#endif

const TInt KMaxLogLineLength = 512;

#define KPRINTERROR        0x00000001 // Tracing level: error
#define KPRINTINFO         0x00000002 // Tracing level: function trace
#define KPRINTSTATE        0x00000004 // Tracing level: state machine info
#define KPRINTWARNING      0x00000008 // Tracing level: warning

const TInt KTraceMask = KPRINTERROR | KPRINTINFO | KPRINTSTATE | KPRINTWARNING;

/**
 *  Owerflow handler for 16 bit descriptor 
 * 
 *  @since S60 S60 v3.2
 */
NONSHARABLE_CLASS( TOverflowTruncate16 ) : public TDes16Overflow
    {
public:
    void Overflow(TDes16& /*aDes*/) {}
    };
    
/**
 *  Owerflow handler for 8bit descriptor 
 * 
 *  @since S60 S60 v3.2
 */
NONSHARABLE_CLASS( TOverflowTruncate8 ) : public TDes8Overflow
    {
public:
    void Overflow(TDes8& /*aDes*/) {}
    };

/**
 * Log requested lines with serial logging or file logging
 *
 * @since S60 S60_3.2
 * @param aFmt a Data to be printed
 * @return None.
 */
inline void Trace( TRefByValue<const TDesC16> aFmt, ... )
    {
    VA_LIST list;
    VA_START(list,aFmt);
#ifdef PRJ_FILE_TRACE
    RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list );
#else
    TBuf16<KMaxLogLineLength> theFinalString;
    theFinalString.Append( KTracePrefix16 );
    TOverflowTruncate16 overflow;
    theFinalString.AppendFormatList( aFmt, list, &overflow );
    RDebug::Print( theFinalString );
#endif
    }


/**
 * Log requested lines with serial logging or file logging
 *
 * @since S60 S60_3.2
 * @param aFmt a Data to be printed
 * @return None.
 */
inline void Trace( TRefByValue<const TDesC8> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
#ifdef PRJ_FILE_TRACE
    RFileLogger::WriteFormat( KLogDir, KLogFile, EFileLoggingModeAppend, aFmt, list );
#else
    TOverflowTruncate8 overflow;
    TBuf8<KMaxLogLineLength> buf8;
    buf8.Append( KTracePrefix8 );
    buf8.AppendFormatList( aFmt, list, &overflow );
    TBuf16<KMaxLogLineLength> buf16( buf8.Length() );
    buf16.Copy(buf8);
    TRefByValue<const TDesC> tmpFmt( _L("%S") );
    RDebug::Print( tmpFmt, &buf16 );
#endif
    }

/**
 * Log panics
 *
 * @since S60 S60_3.2
 * @param aFile a File where panic happens
 * @param aLine a Line where panic happens
 * @param aPanicCode a Panic code
 * @param aPanicCategory a Panic category
 * @return None.
 */
inline void TracePanic(
    char* aFile, 
    TInt aLine,
    TInt aPanicCode,
    const TDesC& aPanicCategory)
    {
    TPtrC8 fullFileName( (const TUint8*)aFile );
    TPtrC8 fileName( fullFileName.Ptr()+fullFileName.LocateReverse('\\')+1 );
    TBuf8<KMaxLogLineLength> buf;
    buf.Append( KPanicPrefix8 );
    buf.AppendFormat( _L8("%d at line %d in file %S"), aPanicCode, aLine, &fileName );
    Trace( buf );
    User::Panic( aPanicCategory, aPanicCode ); 
    }

/**
 * Log panics
 *
 * @since S60 S60_3.2
 * @param aFile a File where leave happens
 * @param aLine a Line where leave happens
 * @param aReason a Reason for leave 
 * @return None.
 */
inline void TraceLeave( char* aFile, TInt aLine, TInt aReason )
    {
    TPtrC8 fullFileName( (const TUint8*)aFile );
    TPtrC8 fileName( fullFileName.Ptr()+fullFileName.LocateReverse('\\')+1 );
    TBuf8<KMaxLogLineLength> buf;
    buf.Append( KLeavePrefix8 );
    buf.AppendFormat( _L8("%d at line %d in file %S"), aReason, aLine, &fileName );
    Trace( buf );
    User::LeaveIfError( aReason );
    }

#define TRACE_INFO(p) {if(KTraceMask & KPRINTINFO) Trace p;}

#define TRACE_ERROR(p) {if(KTraceMask & KPRINTERROR) Trace p;}

#define TRACE_STATE(p) {if(KTraceMask & KPRINTSTATE) Trace p;}

#define TRACE_WARNING(p) {if(KTraceMask & KPRINTWARNING) Trace p;}

#define TRACE_INFO_SEG(p) {if(KTraceMask & KPRINTINFO) p;}

#define TRACE_ASSERT(GUARD, CODE) {if (!(GUARD)) TracePanic( __FILE__, __LINE__, CODE, KPanicCategory); }

#define PANIC(CODE) TracePanic( __FILE__, __LINE__, CODE, KPanicCategory )

#define LEAVE_IF_ERROR(REASON) {if ( REASON ) TraceLeave( __FILE__, __LINE__, REASON );}

#define LEAVE(REASON) {TraceLeave( __FILE__, __LINE__, REASON );}

#define TRACE_FUNC_ENTRY {if(KTraceMask & KPRINTINFO) { TPtrC8 ptr8( (TUint8*)__PRETTY_FUNCTION__ ); Trace( KFuncEntryFormat8, &ptr8 );}}

#define TRACE_FUNC_ENTRY_THIS {if(KTraceMask & KPRINTINFO) { TPtrC8 ptr8((TUint8*)__PRETTY_FUNCTION__); Trace( KFuncEntryThisFormat8, &ptr8, this );}}

#define TRACE_FUNC_EXIT {if(KTraceMask & KPRINTINFO) { TPtrC8 ptr8((TUint8*)__PRETTY_FUNCTION__); Trace( KFuncExitFormat8, &ptr8 );}}

#define TRACE_FUNC {if(KTraceMask & KPRINTINFO) { TPtrC8 ptr8( (TUint8*)__PRETTY_FUNCTION__ ); Trace( KFuncFormat8, &ptr8 );}}

#define TRACE_FUNC_THIS {if(KTraceMask & KPRINTINFO) { TPtrC8 ptr8( (TUint8*)__PRETTY_FUNCTION__ ); Trace( KFuncThisFormat8, &ptr8, this );}}

#define RETURN_IF_ERR(ERR) {if( ERR ) { TPtrC8 ptr8((TUint8*)__FILE__ ); Trace( _L8(" RETURN %d at file %S line %d"), ERR, &ptr8, __LINE__ ); return ERR;}}

#else // PRJ_ENABLE_TRACE not defined

#define TRACE_INFO(p)

#define TRACE_ERROR(p)

#define TRACE_STATE(p)

#define TRACE_WARNING(p)

#define TRACE_INFO_SEG(p)

#define TRACE_ASSERT(GUARD, CODE)

#define PANIC(CODE) {User::Panic(KPanicCategory, CODE);}

#define LEAVE_IF_ERROR(REASON) {static_cast<void>( User::LeaveIfError( REASON ) );}

#define LEAVE(REASON) {static_cast<void>( User::Leave(REASON ) );}

#define TRACE_FUNC_ENTRY

#define TRACE_FUNC_ENTRY_THIS

#define TRACE_FUNC_EXIT

#define TRACE_FUNC

#define TRACE_FUNC_THIS

#define RETURN_IF_ERR( ERR ) {if( ERR ) return ERR;}
#endif // PRJ_ENABLE_TRACE

#endif // PRJ_LOGGING_H
