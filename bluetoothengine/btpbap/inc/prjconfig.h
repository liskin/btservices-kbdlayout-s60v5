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

#ifndef OBEXSM_PRJCONFIG_H
#define OBEXSM_PRJCONFIG_H

/**
 * Traces are enabled via RDebug::Print() in UDEB build, by default.
 */
#ifdef _DEBUG
#define PRJ_ENABLE_TRACE
#endif

/**
 * Traces will be written to file if this is defined.
 */
//#define PRJ_FILE_TRACE


/**
 * build the project for module test purpose if this is defined
 */
//#define PRJ_MODULETEST_BUILD

/**
 * build the project using stubs to replace the dependencies if this is defined
 */
//#define PRJ_USE_STUB

#endif // OBEXSM_PRJCONFIG_H
