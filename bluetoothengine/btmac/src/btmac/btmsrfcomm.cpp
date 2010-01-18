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
* Description:  The RFCOMM connected state definition
*
*/


// INCLUDE FILES
//#include <bteng.h>
#include <btaccTypes.h>

#include "btmrfcommsock.h"
#include "btmsrfcomm.h"
#include "btmsctrl.h"
#include "btmssniffm.h"
#include "btmsdisconnect.h"
#include "btmsopenaudio.h"
#include "debug.h"
#include "btmchandlerapi.h"
#include "btmcprofileid.h"

// ======== MEMBER FUNCTIONS ========

CBtmsRfcomm* CBtmsRfcomm::NewL(CBtmMan& aParent, TRequestStatus* aStatus, 
    CBtmRfcommSock* aSock, TRequestCategory aRequestCat)
    {
    CBtmsRfcomm* self = new( ELeave ) CBtmsRfcomm(aParent, aStatus, aSock, aRequestCat);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsRfcomm::~CBtmsRfcomm()
    {
    }

void CBtmsRfcomm::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Rfcomm"))
    Parent().LoadCmdHandlerL(EBtmcHFP0105, iRfcomm->Remote(), iRfcomm->AccessoryInitiatedConnection());
	}

void CBtmsRfcomm::CancelConnectL()
    {
    TRACE_FUNC
    if (StateRequest() && iRequestCat == ERequestConnect)
        {
        CompleteStateRequest(KErrCancel);
        CBtmsDisconnect* next = CBtmsDisconnect::NewL(Parent(), NULL, SwapStateRfcommSock());
        CleanupStack::PushL(next);
        Parent().ChangeStateL(next);
        CleanupStack::Pop(next);
        }
    }

void CBtmsRfcomm::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        aStatus =  KRequestPending;
        Parent().ChangeStateL(CBtmsDisconnect::NewL(Parent(), &aStatus, SwapStateRfcommSock()));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsRfcomm::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        SetStateRequest(aStatus);
        aStatus = KRequestPending;
        iRequestCat = ERequestOpenAudio;
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsRfcomm::CancelOpenAudioLinkL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote() && iRequestCat == ERequestOpenAudio)
        {
        CompleteStateRequest(KErrCancel);
        iRequestCat = ERequestNone;
        }
    }
    
void CBtmsRfcomm::RfcommErrorL(TInt aErr)
    {
    CompleteStateRequest(aErr);
    }

void CBtmsRfcomm::SlcIndicateL(TBool aSlc)
    {
    TRACE_FUNC
    if (!aSlc)
        {
        CBtmsDisconnect* next = CBtmsDisconnect::NewL(Parent(), SwapStateRequest(), SwapStateRfcommSock());
        next->SetConnectFailReason(KErrCouldNotConnect);
        Parent().ChangeStateL(next);
        }
    else
        {
        if (StateRequest() && iRequestCat == ERequestConnect)
            CompleteStateRequest(KErrNone);
        else
            Parent().NewAccessory(iRfcomm->Remote(), EHFP);
        if (StateRequest() && iRequestCat == ERequestOpenAudio)
            {
            Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), SwapStateRfcommSock(), NULL));
            }
        else
            {
            if (iRfcomm->IsInSniff())
                {
                Parent().ChangeStateL(CBtmsSniffm::NewL(Parent(), SwapStateRfcommSock(), NULL));
                }
            else
                {
                Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), SwapStateRfcommSock(), NULL));
                }
            }
        }
    }

TBool CBtmsRfcomm::CanDisableNrec()
    {
    return ETrue;
    }

CBtmsRfcomm::CBtmsRfcomm(CBtmMan& aParent, TRequestStatus* aStatus, 
    CBtmRfcommSock* aRfcomm, TRequestCategory aRequestCat)
    : CBtmsInuse(aParent, aStatus, aRfcomm), iRequestCat(aRequestCat)
    {
    iRfcomm->SetObserver(*this);
    }

void CBtmsRfcomm::ConstructL()
    {
    }

