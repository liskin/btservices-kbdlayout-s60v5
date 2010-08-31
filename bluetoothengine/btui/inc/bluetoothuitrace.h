/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  
 *   
 */

#ifndef BLUETOOTHUITRACE_H
#define BLUETOOTHUITRACE_H

#include <bluetoothtrace.h>
#include <QDateTime>

#ifdef BLUETOOTHTRACE_ENABLED

/*
 * trace macro for Qt code with an additional trace text. 
 * param must be QString type.
 */
#define BtTraceQString1( groupName, traceName, traceText, param ) \
{\
    _LIT(TrTxt, traceText); TPtrC p(TrTxt); \
    TPtrC textPtr(reinterpret_cast<const TUint16*>(param.utf16()), param.length() ); \
    BOstraceExt2( groupName, traceName, "%S%S", &p, &textPtr ); \
}

/*
 * trace macro for Qt code with no additional trace text. 
 * param must be QString type.
 */
#define BtTraceQString0( groupName, traceName, param ) \
{\
    TPtrC textPtr(reinterpret_cast<const TUint16*>(param.utf16()), param.length() ); \
    BOstraceExt1( groupName, traceName, "%S", &textPtr ); \
}

/*
 * Macro for tracing a Bluetooth device entry in btuimdevlist
 * with no additional trace text. 
 */
#define BtTraceDevListEntry0( groupName, traceName, dev ) \
{\
    QString info("["); \
    info += dev[Btuim::DevAddrReadableRole].toString() + "]"; \
    QString filterBinary; \
    filterBinary.setNum( dev[Btuim::MajorFilterRole].toInt(), 16); \
    info += "[" + filterBinary + "]"; \
    info += "[" \
        + dev[Btuim::LastUsedTimeRole].value<QDateTime>().toString(Qt::ISODate ) \
        + "]" ; \
    info += "[" + dev[Btuim::DevNameRoleRole].toString() + "]" ; \
    TPtrC textPtr(reinterpret_cast<const TUint16*>(info.utf16()), info.length() ); \
    BOstraceExt1( groupName, traceName, "%S", &textPtr ); \
}

/*
 * Macro for tracing a Bluetooth device entry in btuim
 * with an additional trace text. 
 */
#define BtTraceDevListEntry1( groupName, traceName, traceText, dev ) \
{\
    QString info("["); \
    info += dev[Btuim::DevAddrReadableRole].toString() + "]"; \
    QString cod; \
    cod.setNum( dev[Btuim::ClassOfDeviceRole].toInt(), 16); \
    info += "[" + cod + "]"; \
    QString filterBinary; \
    filterBinary.setNum( dev[Btuim::MajorFilterRole].toInt(), 16); \
    info += "[" + filterBinary + "]"; \
    info += "[" \
        + dev[Btuim::LastUsedTimeRole].value<QDateTime>().toString(Qt::ISODate ) \
        + "]" ; \
    info += "[" + dev[Btuim::DevNameRole].toString() + "]" ; \
    TPtrC textPtr(reinterpret_cast<const TUint16*>(info.utf16()), info.length() ); \
    _LIT(TrTxt, traceText); TPtrC p(TrTxt); \
    BOstraceExt2( groupName, traceName, "%S%S", &p, &textPtr ); \
}

/*
 * Macro for Qt code with additional trace text. 
 * list must be QStringList type.
 */
#define BtTraceQStringList1( groupName, traceName, traceText, list ) \
{\
    QString info(": ["); \
    info += list.join(".") + "]"; \
    TPtrC textPtr(reinterpret_cast<const TUint16*>(info.utf16()), info.length() ); \
    _LIT(TrTxt, traceText); TPtrC p(TrTxt); \
    BOstraceExt2( groupName, traceName, "%S%S", &p, &textPtr ); \
}

/*
 * Macro for Qt code with no additional trace text. 
 * list must be QStringList type.
 */
#define BtTraceQStringList0( groupName, traceName, list ) \
{\
    QString info(": ["); \
    info += list.join(".") + "]"; \
    TPtrC textPtr(reinterpret_cast<const TUint16*>(info.utf16()), info.length() ); \
    BOstraceExt1( groupName, traceName, "%S", &textPtr ); \
}


/*
 * Macro for tracing Bluetooth DevData data source in btuimdevdata
 * with no additional trace text. 
*/
#define BtTraceDevDataEntry0( groupName, traceName, devData ) \
{\
    QString info("["); \
    QMap< int, QVariant > val = devData.at( Btuim::DevDataIndexName ); \
    info += val.value(Qt::EditRole).toString() + "]" ; \
    val = devData.at( Btuim::DevDataIndexStatus ); \
    int statusBits = val.value(Qt::EditRole).toInt(); \
    info += "["; \
    info += QString::number(statusBits, 16 ); \
    info += "]"; \
    QStringList strl = val.value(Qt::DisplayRole).toStringList(); \
    info += "[" ; \
    for ( int i = 0; i < strl.count(); ++i ) { \
        info += strl.at(i) + ","; \
    } \
    info += "]" ; \
    val = devData.at( Btuim::DevDataIndexOptionsMenu ); \
    info += " opts:"; \
    QList<QVariant> cmdItems = val.value( Btuim::DevDataCmdItemsRole ).toList(); \
    for ( int i = 0; i < cmdItems.count(); i++ ) { \
        const Btuim::DevDataCmdItem& item = cmdItems.at(i).value<Btuim::DevDataCmdItem>(); \
        info += "[" + QString::number(item.mCmdId) + "," + QString::number(item.mEnabled) + "," + "]"; \
    } \
    val = devData.at( Btuim::DevDataIndexCmdList ); \
    info += " cmds:"; \
    cmdItems = val.value( Btuim::DevDataCmdItemsRole ).toList(); \
    for ( int i = 0; i < cmdItems.count(); i++ ) { \
        const Btuim::DevDataCmdItem& item = cmdItems.at(i).value<Btuim::DevDataCmdItem>(); \
        info += "[" + QString::number(item.mCmdId) + "," + QString::number(item.mEnabled) + "," + "]"; \
    } \
    TPtrC textPtr(reinterpret_cast<const TUint16*>(info.utf16()), info.length() ); \
    BOstraceExt1( groupName, traceName, "%S", &textPtr ); \
}
#else

#define BtTraceQString1( aGroupName, aTraceName, aTraceText, aParam ) 
#define BtTraceQString0( aGroupName, aTraceName, aParam )
#define BtTraceDevListEntry0( groupName, traceName, dev )
#define BtTraceDevListEntry1( groupName, traceName, traceText, dev )
#define BtTraceQStringList1( groupName, traceName, traceText, list )
#define BtTraceQStringList0( groupName, traceName, list )
#define BtTraceDevDataEntry0( groupName, traceName, devData )
#endif // BLUETOOTHTRACE_ENABLED

// At early development phase, we force assertion in release build to find out 
// design and implementation issues.
//#ifndef QT_NO_DEBUG
#define BTUI_DEBUG
//#endif

#if !defined(BTUI_ASSERT_X)
#  ifdef BTUI_DEBUG
#define BTUI_ASSERT_X(cond, where, what) ((!(cond)) ? qt_assert_x(where, what,__FILE__,__LINE__) : qt_noop())
#  else
#    define BTUI_ASSERT_X(cond, where, what) qt_noop()
#  endif
#endif

#endif // BLUETOOTHUITRACE_H
