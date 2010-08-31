/*
* ============================================================================
*  Name        : traceconfig_template.h
*  Part of     : BluetoothUI / bluetoothuimodel       *** Info from the SWAD
*  Description : Configuration of debug tracing in btnotif
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.2
*/
#ifndef BLUETOOTHTRACECONFIG_H
#define BLUETOOTHTRACECONFIG_H

/*
Sample usage:
    void testTracing() 
    {
        BOstrace0( TRACE_FATAL, TNAME_DEVLIST_1, "BOstrace0" );
        BOstrace1( TRACE_IMPORTANT, TNAME_DEVLIST_2, "BOstrace1 %d", 123 );
        _LIT(String, "\"Symbian Descriptor\"");
        TPtrC ptr(String);
        TBuf<20> buf(ptr);
        BOstraceExt1( TRACE_NORMAL, TNAME_DEVLIST_3, "BOstraceExt1 %S",  &ptr);
        BOstraceExt2( TRACE_API, TNAME_DEVLIST_4, "BOstraceExt2 %d %S", 456, &ptr );
        BOstraceExt3( TRACE_FLOW, TNAME_DEVLIST, "BOstraceExt3 0x%x %d %S", 128, 256, &ptr );
        BOstraceExt4( TRACE_DETAILED, TNAME_DEVL_5IST, "BOstraceExt4 0x%x %d %S %S", 128, 256, &ptr, &buf );
        BOstraceExt5( TRACE_DEBUG, TNAME_DEVLIST_6, "BOstraceExt5 0x%x %d %S %S, %b", 128, 256, &ptr, &buf, 512 );
        BOstraceFunctionEntry0( TNAME_DEVLIST_7 );
        BOstraceFunctionEntry1( TNAME_DEVLIST_8, 0x00abcdef );
        BOstraceFunctionEntryExt(TNAME_DEVLIST_9, 0xdeadbeef, 123);
        BOstraceFunctionExit0( TNAME_DEVLIST_9 );
        BOstraceFunctionExit1( TNAME_DEVLIST_10, 0x00beebee );
        BOstraceFunctionExitExt(TNAME_DEVLIST_11, 0x00badbed, -1);
        BOstraceEventStart0( TNAME_DEVLIST_12, "BOstraceEventStart0" );
        BOstraceEventStart1( TNAME_DEVLIST_13, "BOstraceEventStart1", 789 );
        BOstraceEventStop( TNAME_DEVLIST_14, "BOstraceEventStop" );
        BOstraceState0( TNAME_DEVLIST_15, "connection state", 1 );
        BOstraceState1( TNAME_DEVLIST_16, "audio state", 2, 0xdeadbeef );
        BtTraceBlock( 
                for (int i = 0; i < 5; ++i) { 
                    BOstrace1( TRACE_IMPORTANT, TNAME_DEVLIST_, "BtTraceBlock counter(1-5): %d", i+1 );
                });
        QString str("\"Qt String\"");
        BtTraceQString0( TRACE_NORMAL, TNAME_DEVLIST_17, str);
        BtTraceQString1( TRACE_NORMAL, TNAME_DEVLIST_18, "additional text;", str);
        TBTDevAddr addr;
        addr.SetReadable(_L("0060576ff376"));
        BtTraceBtAddr0( TRACE_NORMAL, TNAME_DEVLIST_19, addr );
        BtTraceBtAddr1( TRACE_NORMAL, TNAME_DEVLIST_20, "additional trace;", addr );
    }

 */

// At early development phase, tracing is activated 
#ifdef _DEBUG
#define BLUETOOTHTRACE_ENABLED
#endif //_DEBUG

/*
 * Tracing media configuration
 */  
#ifdef BLUETOOTHTRACE_ENABLED
    //#ifdef __WINS__
    //#define BLUETOOTHTRACE_MEDIA_FILE
    //#else 
    // RDEBUG is used for tracing output before we migrate to OST tracing.
    #define BLUETOOTHTRACE_MEDIA_RDEBUG
    // #define BLUETOOTHTRACE_MEDIA_OST 
    //#endif // __WINS__
#endif //BLUETOOTHTRACE_ENABLED

/*
 * Configuration of tracing to file
 */
#ifdef BLUETOOTHTRACE_MEDIA_FILE

_LIT( KLogFile, "btenglog.txt" );
_LIT( KLogDir, "bt" );

#endif //BLUETOOTHTRACE_MEDIA_FILE

/*
 * Configuration of tracing using RDebug
 */
#ifdef BLUETOOTHTRACE_MEDIA_RDEBUG

#endif //BLUETOOTHTRACE_MEDIA_RDEBUG

/*
 * Configuration of tracing using OST
 */
#ifndef BLUETOOTHTRACE_MEDIA_OST

/**
 * Group-mapping aligning with OST groups.
 * The purpose of using groups is to ease migrating tracing from legacy logging to OST.
 */
#define TRACE_FATAL       "[FATAL]"
#define TRACE_IMPORTANT   "[IMPTT]"
#define TRACE_NORMAL      "[NORML]"
#define TRACE_API         "[ API ]"
#define TRACE_FLOW        "[FLOW ]"
#define TRACE_STATE       "[STATE]"
#define TRACE_DETAILED    "[DETLD]"
#define TRACE_DEBUG       "[DEBUG]"
#define TRACE_PERFORMANCE "[PFMAN]"

/**
 * Component Identifier to be written into traces:
 */
_LIT(KComponentName, "[BtNotif]");

#endif //BLUETOOTHTRACE_MEDIA_OST

#endif // BLUETOOTHTRACECONFIG_H
