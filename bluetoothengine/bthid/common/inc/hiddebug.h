/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This header file contains debug macro declarations
 *
*/


#ifndef BTHIDDEBUG_H
#define BTHIDDEBUG_H

//#include <e32debug.h>

// MACROS
#ifdef _DEBUG
#define DBG(a) a
#define DEBUG_PRINT(a) RDebug::Print(a)
#define DEBUG_PRINTF(a,b) RDebug::Print(a,b)
#define DEBUG_PRINTF2(a,b,c) RDebug::Print(a,b,c)
#else
#define DBG(a)
#define DEBUG_PRINT(a)
#define DEBUG_PRINTF(a,b)
#define DEBUG_PRINTF2(a,b,c)
#endif

#endif // BTHIDDEBUG_H
// End of File
