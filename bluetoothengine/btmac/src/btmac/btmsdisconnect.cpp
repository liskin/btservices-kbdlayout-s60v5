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
* Description:  The disconnect state definition
*
*/


#include "btmsdisconnect.h"
#include "btmsyncsock.h"
#include "btmrfcommsock.h"
#include "btmslisten.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmsDisconnect* CBtmsDisconnect::NewL(CBtmMan& aParent, TRequestStatus* aRequest, CBtmRfcommSock* aRfcomm)
    {
    CBtmsDisconnect* self = new( ELeave ) CBtmsDisconnect(aParent, aRequest, aRfcomm);
    return self;
    }

CBtmsDisconnect::~CBtmsDisconnect()
    {
    TRACE_FUNC
    }

void CBtmsDisconnect::SetConnectFailReason(TInt aReason)
    {
    iConnectFailReason = aReason;
    }

void CBtmsDisconnect::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Disconnect"))
    Parent().DeleteCmdHandler();
    iRfcomm->SetObserver(*this);
    iRfcomm->Disconnect();
    }

void CBtmsDisconnect::RfcommErrorL(TInt aErr)
    {
    TRACE_FUNC
    delete iRfcomm;
    iRfcomm = NULL;
    CompleteStateRequest(aErr);
    }

void CBtmsDisconnect::RfcommShutdownCompletedL(TInt aErr)
    {
    TRACE_FUNC
    if (iConnectFailReason)
        {
        CompleteStateRequest(iConnectFailReason);
        }
    else
        {
        CompleteStateRequest(aErr);
        }
    Parent().ChangeStateL(CBtmsListen::NewL(Parent()));
    }
    
CBtmsDisconnect::CBtmsDisconnect(CBtmMan& aParent, TRequestStatus* aRequest, 
    CBtmRfcommSock* aRfcomm) : CBtmsInuse(aParent, aRequest, aRfcomm)
    {
    }
