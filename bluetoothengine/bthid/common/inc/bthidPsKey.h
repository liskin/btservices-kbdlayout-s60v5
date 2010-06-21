/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*
*
*/


#ifndef BTHIDPSKEY_H_
#define BTHIDPSKEY_H_



const TUid KPSUidBthidSrv = { 0x2001FE5C };
 
const TUint KBTMouseCursorState = 0x00000001;

enum THidMouseCursorState
    {
    ECursorNotInitialized = 0, 
    ECursorShow = 1, 
    ECursorHide = 2,
    ECursorRedraw = 3,
    ECursorReset = 4
    };

#endif /* BTHIDPSKEY_H_ */
