/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Tracing macros
*
*/

#ifndef BTDEVICEDIALOGPLUGINTRACE_H
#define BTDEVICEDIALOGPLUGINTRACE_H

#include <QDebug>   // QDebug
#include <QtGlobal> // qDebug()


#ifdef ENABLE_TRACE_OUTPUT
    #define INSTALL_MESSAGE_HANDLER HbDeviceDialogMessageHandler::installMessageHandler(QString(TRACE_OUTPUT_FILE), TRACE_OUTPUT_FILE_REQUIRED);
    #define UNINSTALL_MESSAGE_HANDLER HbDeviceDialogMessageHandler::uninstallMessageHandler();
    #define TRACE_UNUSED(name)
    #define TRACE_STATIC_ENTRY qDebug() << __PRETTY_FUNCTION__ << "entry";
    #define TRACE_STATIC_ENTRY_ARGS(args) qDebug() << __PRETTY_FUNCTION__ << "entry," << args;
    #define TRACE_ENTRY qDebug() << __PRETTY_FUNCTION__ << "this" << (void *)this << "entry";
    #define TRACE_ENTRY_ARGS(args) qDebug() << __PRETTY_FUNCTION__ << "this" << (void *)this << "entry," << args;
    #define TRACE_EXIT qDebug() << __PRETTY_FUNCTION__ << "exit";
    #define TRACE_EXIT_ARGS(args) qDebug() << __PRETTY_FUNCTION__ << "exit," << args;
    #define TRACE(args) qDebug() << __PRETTY_FUNCTION__ << args;
#else
    #define INSTALL_MESSAGE_HANDLER
    #define UNINSTALL_MESSAGE_HANDLER
    #define TRACE_UNUSED(name) Q_UNUSED(name)
    #define TRACE_STATIC_ENTRY
    #define TRACE_STATIC_ENTRY_ARGS(args)
    #define TRACE_ENTRY
    #define TRACE_ENTRY_ARGS(args)
    #define TRACE_EXIT
    #define TRACE_EXIT_ARGS(args)
    #define TRACE(args)
#endif // ENABLE_TRACE_OUTPUT

#endif // BTDEVICEDIALOGPLUGINTRACE_H
