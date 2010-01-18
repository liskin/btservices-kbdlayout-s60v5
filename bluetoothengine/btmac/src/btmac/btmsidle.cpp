/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The state idle definition
*
*/


// INCLUDE FILES
#include "btmsidle.h"
#include "btmsconnect.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmsIdle* CBtmsIdle::NewL(CBtmMan& aParent)
    {
    CBtmsIdle* self = new( ELeave ) CBtmsIdle(aParent);
    return self;
    }

CBtmsIdle::~CBtmsIdle()
    {
    TRACE_FUNC
    }

void CBtmsIdle::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Idle"))
    }
    
// ---------------------------------------------------------------------------
// Move to Connect state
// ---------------------------------------------------------------------------
//
void CBtmsIdle::ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    Parent().ChangeStateL(CBtmsConnect::NewL(Parent(), &aStatus, aAddr, ERequestConnect));
    }

CBtmsIdle::CBtmsIdle(CBtmMan& aParent) : CBtmState(aParent, NULL)
    {
    }

