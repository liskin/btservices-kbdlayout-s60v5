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
* Description:  The in use state definition
*
*/


// INCLUDE FILES
//#include <bteng.h>
#include <btaccTypes.h>
#include "atcodec.h"
#include "btmsinuse.h"
#include "btmslisten.h"
#include "btmrfcommsock.h"
#include "btmsyncsock.h"
#include "btmsctrl.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmsInuse::~CBtmsInuse()
    {
    delete iRfcomm;
    }


void CBtmsInuse::ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    aStatus = KRequestPending;
    if (iRfcomm->Remote() == aAddr)
        {
        CompleteRequest(&aStatus, KErrAlreadyExists);
        }
    else
        {
        CompleteRequest(&aStatus, KErrInUse);
        }
    }

void CBtmsInuse::SendProtocolDataL(const TDesC8& aData)
    {
    TRACE_FUNC
    iRfcomm->WriteL(aData);
    }

TBTDevAddr CBtmsInuse::Remote()
    {
    return (iRfcomm) ? iRfcomm->Remote() : TBTDevAddr();
    }


void CBtmsInuse::RfcommConnectCompletedL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void CBtmsInuse::RfcommAcceptCompletedL(TInt /*aErr*/, TUint /*aService*/)
    {
    TRACE_FUNC
    }

void CBtmsInuse::RfcommShutdownCompletedL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }


void CBtmsInuse::RfcommSendCompletedL(TInt aErr)
    {
    TRACE_FUNC
    if (aErr)
        {
        if (iRfcomm->Service() == EBTProfileHFP)
            {
            Parent().AccessoryDisconnected(iRfcomm->Remote(), EHFP);
            }
        else
            {
            Parent().AccessoryDisconnected(iRfcomm->Remote(), EHSP);
            }
        RfcommErrorL(aErr);
        Parent().ChangeStateL(CBtmsListen::NewL(Parent()));
        }
    }

void CBtmsInuse::RfcommReceiveCompletedL(TInt aErr, const TDesC8& aData)
    {
    TRACE_FUNC
    if (!aErr)
        {
        TRAP(aErr, Parent().NewProtocolDataL(aData));
        }

    if (aErr)
        {
        RfcommSendCompletedL(aErr);
        }
    }

CBtmsInuse::CBtmsInuse(CBtmMan& aParent, TRequestStatus* aRequest, CBtmRfcommSock* aRfcomm)
    : CBtmState(aParent, aRequest), iRfcomm(aRfcomm)
    {
    }

CBtmRfcommSock* CBtmsInuse::SwapStateRfcommSock()
    {
    CBtmRfcommSock* sock = iRfcomm;
    iRfcomm = NULL;
    return sock;
    }

CBtmSyncSock* CBtmsInuse::SwapSyncSock(CBtmSyncSock*& aSyncSock)
    {
    CBtmSyncSock* sock = aSyncSock;
    aSyncSock = NULL;
    return sock;
    }
