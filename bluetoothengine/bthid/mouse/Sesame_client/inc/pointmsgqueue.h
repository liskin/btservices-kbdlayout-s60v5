/*
* Copyright (c) 2006-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Peninput message queue header
 *
*/

#ifndef __POINT_MSGQUE__
#define __POINT_MSGQUE__
#include <e32msgqueue.h>
#define KPointQueueLen (1000)
#define KPMaxEvent (5)
_LIT(KMsgBTMouseBufferQueue,"BTMouseanimqueue");

const TInt KResetBuffer = -1;

enum
    {
    KBufferPlainPointer = 0, KBufferPenDown, KBufferPenUp
    };

/**
 * KAnimationCommands.
 * Constant used to request a reset of the bouncing square,
 * enumeration can be expanded
 */
enum KAnimationCommands
    {
    KStartBTCursorAnim = 1,
    KStopBTCursorAnim = 2,
    KChangeCursor = 3,
    KSendRawEvent = 4
    };

class TPointBuffer
    {
public:
    TInt iNum;
    TInt iType[2 * KPMaxEvent];
    TPoint iPoint[2 * KPMaxEvent];
    };
#endif
