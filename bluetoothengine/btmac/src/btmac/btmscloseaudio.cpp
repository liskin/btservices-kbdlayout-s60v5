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
* Description:  The close audio state definition
*
*/


#include "btmscloseaudio.h"
#include "btmsdisconnect.h"
#include "btmrfcommsock.h"
#include "btmsyncsock.h"
#include "btmslisten.h"
#include "btmsctrl.h"
#include "btmssniffm.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmsCloseAudio* CBtmsCloseAudio::NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco)
    {
    CBtmsCloseAudio* self = new( ELeave ) CBtmsCloseAudio(aParent, aRequest, aRfcomm, aSco);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsCloseAudio::~CBtmsCloseAudio()
    {
    delete iSco;
    }

void CBtmsCloseAudio::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Close Audio"))    
    iSco->DisconnectL();
    }

void CBtmsCloseAudio::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        delete iSco;
        iSco = NULL;
        CompleteStateRequest(KErrNone);
		aStatus = KRequestPending;
        Parent().ChangeStateL(CBtmsDisconnect::NewL(Parent(),&aStatus, SwapStateRfcommSock()));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsCloseAudio::CancelCloseAudioLinkL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        delete iSco;
        iSco = NULL;
        CompleteStateRequest(KErrCancel);
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


void CBtmsCloseAudio::RfcommErrorL(TInt aErr)
    {
    delete iSco;
    
    iSco = NULL;
    CompleteStateRequest(aErr);
    }

void CBtmsCloseAudio::SyncLinkDisconnectCompleteL(TInt aErr)
    {
    CompleteStateRequest(aErr);
    if (iRfcomm->IsInSniff())
        {
        Parent().ChangeStateL(CBtmsSniffm::NewL(Parent(), SwapStateRfcommSock(), NULL));
        }
    else
        {
        Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), SwapStateRfcommSock(), NULL));
        } 
    }
    
CBtmsCloseAudio::CBtmsCloseAudio(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco)
    : CBtmsInuse(aParent, aRequest, aRfcomm), iSco(aSco)
    {
    iRfcomm->SetObserver(*this);
    iSco->SetObserver(*this);
    }

void CBtmsCloseAudio::ConstructL()
    {
    }

