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
* Description:  The Control state definition
*
*/


// INCLUDE FILES
//#include <bteng.h>
#include <btaccTypes.h>

#include "btmrfcommsock.h"
#include "btmsyncsock.h"
#include "btmsctrl.h"
#include "btmssniffm.h"
#include "btmsopenaudio.h"
#include "btmsdisconnect.h"
#include "btmsaudio.h"
#include "btmactive.h"
#include "btmstempd.h"
#include "debug.h"
#include "btmchandlerapi.h"
#include "btmcprofileid.h"
#include "atcodec.h"
#include <btengconstants.h>



// ======== MEMBER FUNCTIONS ========

CBtmsCtrl* CBtmsCtrl::NewL(CBtmMan& aParent, CBtmRfcommSock* aSock, CBtmSyncSock* aSco)
    {
    CBtmsCtrl* self = new( ELeave ) CBtmsCtrl(aParent, aSock, aSco);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsCtrl::~CBtmsCtrl()
    {
    TRACE_FUNC_ENTRY
    delete iSco;
    TRACE_FUNC_EXIT
    }

void CBtmsCtrl::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Ctrl"))
    if (NotifyItem() == ENotifyAudioClosedByRemote)
        {
        if (iRfcomm->Service() == EBTProfileHFP)
            {
            Parent().RemoteAudioClosed(iRfcomm->Remote(), EHFP);
            }
        else
            {
            Parent().RemoteAudioClosed(iRfcomm->Remote(), EHSP);
            }
        }
    if (!iSco && Parent().IsAccInuse())
        {
        iSco = CBtmSyncSock::NewL(*this, Parent().SockServ());
        iSco->AcceptL(Parent().IsEdrSupported());
        }
    if (iRfcomm->Service() == EBTProfileHSP)
        {
        Parent().LoadCmdHandlerL(EBtmcHSP, iRfcomm->Remote(), iRfcomm->AccessoryInitiatedConnection());
        }
	}

void CBtmsCtrl::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        if (iSco)
            iSco->CancelAccept();
        aStatus =  KRequestPending;
        Parent().ChangeStateL(CBtmsDisconnect::NewL(Parent(), &aStatus, SwapStateRfcommSock()));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsCtrl::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        CBtmRfcommSock* rfcomm = iRfcomm;
        iRfcomm = NULL;
        CBtmSyncSock* sco = iSco;
        iSco = NULL;
        aStatus = KRequestPending;
        Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), &aStatus, rfcomm, sco));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }
    
void CBtmsCtrl::AccInUse()
	{
	TRACE_FUNC
	if ( iRfcomm->Service() == EBTProfileHSP )
	    {
	    // volume control support status of the HSP in remote device
	    // is only available in the owner of this component.
	    // Ask parent to set the support in cmdhandler.
	    Parent().SetCmdHandlerRvcSupport( iRfcomm->Remote() );
	    }
	
	TInt err;
	TRAP( err,
    if (!iSco)
        {
        iSco = CBtmSyncSock::NewL(*this, Parent().SockServ());
        iSco->AcceptL(Parent().IsEdrSupported());
        }
	)
	}

void CBtmsCtrl::RfcommErrorL(TInt /*aErr*/)
    {
    if (iSco)
        iSco->CancelAccept();
    }

TBool CBtmsCtrl::CanDisableNrec()
    {
    return ETrue;
    }

void CBtmsCtrl::SyncLinkAcceptCompleteL(TInt aErr)
    {
    TRACE_FUNC
    if (!aErr)
        {
        CBtmsAudio* next = CBtmsAudio::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iSco)); 
        next->SetNotifyItemAtEntry(ENotifyAudioOpenedByRemote);
        Parent().ChangeStateL(next);
        }
    }

void CBtmsCtrl::RfcommLinkInSniffModeL()
    {
    if (iRfcomm->Service() == EBTProfileHFP)
        {
        Parent().ChangeStateL(CBtmsSniffm::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iSco)));
        }
    }

void CBtmsCtrl::RequestCompletedL(CBtmActive& /*aActive*/)
    {
    if (iRfcomm->Service() == EBTProfileHSP)
        {
        TBTDevAddr addr = iRfcomm->Remote();
        iRfcomm->Disconnect();
        Parent().ChangeStateL(CBtmsHsTempd::NewL(Parent(), addr));
        }
    else
        {
    	iRfcomm->ActivateSniffRequester();
        }
    }

void CBtmsCtrl::CancelRequest(CBtmActive& /*aActive*/)
    {
    }
    
CBtmsCtrl::CBtmsCtrl(CBtmMan& aParent, CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco)
    : CBtmsInuse(aParent, NULL, aRfcomm), iSco(aSco)
    {
    iRfcomm->SetObserver(*this);
    if (iSco)
        {
        iSco->SetObserver(*this);
        }
    }

void CBtmsCtrl::ConstructL()
    {
    }


