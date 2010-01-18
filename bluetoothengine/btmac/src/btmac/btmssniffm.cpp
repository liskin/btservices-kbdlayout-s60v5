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
* Description:  The sniff mode state definition
*
*/


// INCLUDE FILES
#include "btmrfcommsock.h"
#include "btmsyncsock.h"
#include "btmsctrl.h"
#include "btmssniffm.h"
#include "btmslisten.h"
#include "btmsopenaudio.h"
#include "btmsdisconnect.h"
#include "btmsaudio.h"
#include "btmactive.h"
#include "debug.h"

const TInt KTimerRequest = 20;

const TInt KLinkActivatePeriodic = 550 * 1000;
const TInt KLinkActivateRetries = 4;

// ======== MEMBER FUNCTIONS ========

CBtmsSniffm* CBtmsSniffm::NewL(CBtmMan& aParent, CBtmRfcommSock* aSock, CBtmSyncSock* aSco)
    {
    CBtmsSniffm* self = new( ELeave ) CBtmsSniffm(aParent, aSock, aSco);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsSniffm::~CBtmsSniffm()
    {
    TRACE_FUNC_ENTRY
    delete iActive;
    iTimer.Close();
    delete iSco;
    TRACE_FUNC_EXIT
    }

void CBtmsSniffm::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Sniffm"))    
    if (NotifyItem() == ENotifyAudioClosedByRemote)
        {
        Parent().RemoteAudioClosed(iRfcomm->Remote(), EHFP);
        }
    if (!iSco && Parent().IsAccInuse())
        {
        iSco = CBtmSyncSock::NewL(*this, Parent().SockServ());
        iSco->AcceptL(Parent().IsEdrSupported());
        }
	}

void CBtmsSniffm::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        delete iActive;
        iActive = NULL;
        if (iSco)
            iSco->CancelAccept();
        iRfcomm->RequestLinkToActiveMode();
        CBtmRfcommSock* rfcomm = iRfcomm;
        iRfcomm = NULL;
        aStatus =  KRequestPending;
        Parent().ChangeStateL(CBtmsDisconnect::NewL(Parent(), &aStatus, rfcomm));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsSniffm::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        SetStateRequest(aStatus);
        aStatus = KRequestPending;
        iRfcomm->RequestLinkToActiveMode();
        iActive = CBtmActive::NewL(*this, CActive::EPriorityStandard, KTimerRequest);
        iTimer.After(iActive->iStatus, KLinkActivatePeriodic);
        iActive->GoActive();
        iLinkActivateRetries = 1;
        }
    else
        {
        CompleteRequest(&aStatus, KErrGeneral);
        }
    }

void CBtmsSniffm::CancelOpenAudioLinkL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        delete iActive;
        iActive = NULL;
        iSco->CancelAccept();
        CompleteStateRequest(KErrCancel);
        }
    }

void CBtmsSniffm::RfcommErrorL(TInt /*aErr*/)
    {
    delete iActive;
    iActive = NULL;
    if (iSco)
        iSco->CancelAccept();
    }

void CBtmsSniffm::AccInUse()
	{
	TRACE_FUNC
	TInt err;
	TRAP(err,
		if (!iSco)
			{
			iSco = CBtmSyncSock::NewL(*this, Parent().SockServ());
			iSco->AcceptL(Parent().IsEdrSupported());
			}
		)
	}

TBool CBtmsSniffm::CanDisableNrec()
    {
    return ETrue;
    }

void CBtmsSniffm::SyncLinkAcceptCompleteL(TInt aErr)
    {
    TRACE_FUNC
    if (!aErr)
        {
        delete iActive;
        iActive = NULL;
        CBtmRfcommSock* rfcomm = iRfcomm;
        iRfcomm = NULL;
        CBtmSyncSock* sco = iSco;
        iSco = NULL;
        CBtmsAudio* next = CBtmsAudio::NewL(Parent(), rfcomm, sco); 
        if (StateRequest())
            {
            CompleteStateRequest(KErrNone);
            }
        else
            {
            next->SetNotifyItemAtEntry(ENotifyAudioOpenedByRemote);
            }
        Parent().ChangeStateL(next);
        }
    }

void CBtmsSniffm::RfcommLinkInActiveModeL()
    {
    TRACE_FUNC
    delete iActive;
    iActive = NULL;
    CBtmRfcommSock* rfcomm = iRfcomm;
    iRfcomm = NULL;
    CBtmSyncSock* sco = iSco;
    iSco = NULL;
    if (StateRequest())
        {
        Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), rfcomm, sco));
        }
    else
        {
        Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), rfcomm, sco));
        }
    }

void CBtmsSniffm::RequestCompletedL(CBtmActive& /*aActive*/)
    {
    if (iLinkActivateRetries < KLinkActivateRetries)
        {
        iRfcomm->RequestLinkToActiveMode();
        iTimer.After(iActive->iStatus, KLinkActivatePeriodic);
        iActive->GoActive();
        iLinkActivateRetries++;
        }
    else if (StateRequest())
        {
        CBtmRfcommSock* rfcomm = iRfcomm;
        iRfcomm = NULL;
        CBtmSyncSock* sco = iSco;
        iSco = NULL;
        Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), rfcomm, sco));
        }
    }

void CBtmsSniffm::CancelRequest(CBtmActive& /*aActive*/)
    {
    iTimer.Cancel();
    }
    
CBtmsSniffm::CBtmsSniffm(CBtmMan& aParent, CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco)
    : CBtmsInuse(aParent, NULL, aRfcomm), iSco(aSco)
    {
    iRfcomm->SetObserver(*this);
    if (iSco)
        {
        iSco->SetObserver(*this);
        }
    }

void CBtmsSniffm::ConstructL()
    {
    iTimer.CreateLocal();
    }

