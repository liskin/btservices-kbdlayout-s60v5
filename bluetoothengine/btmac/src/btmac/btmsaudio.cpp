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
* Description:  The audio opened state definition
*  Version     : %version: 4.2.3 %
*
*/


#include "btmsaudio.h"
#include "btmrfcommsock.h"
#include "btmsyncsock.h"
#include "btmslisten.h"
#include "btmsctrl.h"
#include "btmssniffm.h"
#include "btmsdisconnect.h"
#include "btmscloseaudio.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmsAudio* CBtmsAudio::NewL(CBtmMan& aParent, CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco)
    {
    CBtmsAudio* self = new( ELeave ) CBtmsAudio(aParent, aRfcomm, aSco);
    return self;
    }

CBtmsAudio::~CBtmsAudio()
    {
    delete iSco;
    }

void CBtmsAudio::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Audio"))    
    if (NotifyItem() == ENotifyAudioOpenedByRemote)
        {
        if (iRfcomm->Service() == EBTProfileHFP)
            {
            Parent().RemoteAudioOpened(iRfcomm->Remote(), EHFP);
            }
        else
            {
            Parent().RemoteAudioOpened(iRfcomm->Remote(), EHSP);
            }
        }
    }

void CBtmsAudio::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
		iSco->DisconnectL();
		delete iSco;
		iSco = NULL;
		aStatus = KRequestPending;
        Parent().ChangeStateL(CBtmsDisconnect::NewL(Parent(),&aStatus, SwapStateRfcommSock()));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsAudio::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    aStatus = KRequestPending;
    if (aAddr == iRfcomm->Remote())
        {
        CompleteRequest(&aStatus, KErrInUse);
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsAudio::CloseAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        aStatus = KRequestPending;
        Parent().ChangeStateL(CBtmsCloseAudio::NewL(Parent(),&aStatus, SwapStateRfcommSock(), SwapSyncSock(iSco)));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsAudio::RfcommErrorL(TInt /*aErr*/)
    {
    iSco->DisconnectL();
    delete iSco;
    iSco = NULL;
    }

void CBtmsAudio::SyncLinkDisconnectCompleteL(TInt /*aErr*/)
    {
    CBtmsInuse* next;
    if (iRfcomm->IsInSniff())
        next = CBtmsSniffm::NewL(Parent(), SwapStateRfcommSock(), NULL); 
    else
        next = CBtmsCtrl::NewL(Parent(), SwapStateRfcommSock(), NULL);
    next->SetNotifyItemAtEntry(ENotifyAudioClosedByRemote);
    Parent().ChangeStateL(next);
    }

TInt CBtmsAudio::AudioLinkLatency()
    {
    TRACE_FUNC
    return iSco->Latency();
    }
CBtmsAudio::CBtmsAudio(CBtmMan& aParent, CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco)
    : CBtmsInuse(aParent, NULL, aRfcomm), iSco(aSco)
    {
    iRfcomm->SetObserver(*this);
    iSco->SetObserver(*this);
    }
