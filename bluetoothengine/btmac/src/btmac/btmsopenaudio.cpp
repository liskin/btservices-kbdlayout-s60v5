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
* Description:  The open audio state definition
*
*/

#include "btmsopenaudio.h"
#include "btmsdisconnect.h"
#include "btmrfcommsock.h"
#include "btmsyncsock.h"
#include "btmslisten.h"
#include "btmsctrl.h"
#include "btmssniffm.h"
#include "btmsaudio.h"
#include "debug.h"
#include <btengconstants.h>

// ======== MEMBER FUNCTIONS ========

CBtmsOpenAudio* CBtmsOpenAudio::NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco, TBool aUseEsco, EESCOParam aESCOParameter)
    {
    CBtmsOpenAudio* self = new( ELeave ) CBtmsOpenAudio(aParent, aRequest, aRfcomm, aSco, aUseEsco, aESCOParameter);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsOpenAudio::~CBtmsOpenAudio()
    {
    delete iAcceptSco;
    delete iSetupSco;
    }

void CBtmsOpenAudio::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Open Audio"))    
    if (!iAcceptSco)
        {
        iAcceptSco = CBtmSyncSock::NewL(*this, Parent().SockServ());
        iAcceptSco->AcceptL(Parent().IsEdrSupported());
        }
    iSetupSco = CBtmSyncSock::NewL(*this, Parent().SockServ());
    if(Parent().IsEdrSupported() && iUseEsco)
    	{
    	iSetupSco->SetMaxLatency(KMaxLatencySpec);
    	iSetupSco->SetRetransmissionEffort(EeSCORetransmitDontCare);
    	iSetupSco->SetupL(iRfcomm->Remote(), TBTSyncPackets(iESCOParameter));
    	}
    else
		{
		iSetupSco->SetupL(iRfcomm->Remote(), TBTSyncPackets(KScoPacketTypeSpec));
		}

    if (iRfcomm->Service() == EBTProfileHSP)
        {
        Parent().LoadCmdHandlerL(EBtmcHSP, iRfcomm->Remote(), iRfcomm->AccessoryInitiatedConnection());
        }    
    }

void CBtmsOpenAudio::Disconnect(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        CompleteRequest(&aStatus, KErrInUse);
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsOpenAudio::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    aStatus = KRequestPending;
    if (aAddr == iRfcomm->Remote())
        {
        CompleteRequest(&aStatus, KErrAlreadyExists);
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsOpenAudio::CancelOpenAudioLinkL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    if (aAddr == iRfcomm->Remote())
        {
        iSetupSco->CancelSetup();
        CompleteStateRequest(KErrCancel);
        if (iRfcomm->IsInSniff())
            {
            Parent().ChangeStateL(CBtmsSniffm::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco)));
            }
        else
            {
            Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco)));
            }
        }
    }
    
void CBtmsOpenAudio::RfcommErrorL(TInt aErr)
    {
    TRACE_FUNC
    if (iAcceptSco)
        {
        iAcceptSco->CancelAccept();
        }
    if (iSetupSco)
        {
        iSetupSco->CancelSetup();
        }
    CompleteStateRequest(aErr);
    }

void CBtmsOpenAudio::SyncLinkSetupCompleteL(TInt aErr)
    {
    TRACE_FUNC
    if (aErr)
        {
        if(Parent().IsEdrSupported() && iESCOParameter != 0)
        	{
        	switch(iESCOParameter)
        		{
        		case EEV5:
        			Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco), ETrue, EEV4));
        			break;
        		case EEV4:
        			Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco), ETrue, EEV3));
        			break;
        		case EEV3:
        			Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco), EFalse, ENoeSCO));
        			break;
        		}
        		return;
        	}
        else
            {
	        TInt err4openaudio  = (aErr == KErrInUse) ? KErrCouldNotConnect : aErr;
	        CompleteStateRequest(err4openaudio);
            if (iRfcomm->IsInSniff())
                {
                Parent().ChangeStateL(CBtmsSniffm::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco)));
                }
            else
                {
                Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco)));
                }
            } 
        }
    else
        {
        iAcceptSco->CancelAccept();
        CompleteStateRequest(aErr);
        Parent().ChangeStateL(CBtmsAudio::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iSetupSco)));
        }
    }

void CBtmsOpenAudio::SyncLinkAcceptCompleteL(TInt aErr)
    {
    TRACE_FUNC
    if (!aErr)
        {
        CompleteStateRequest(aErr);
        iSetupSco->CancelSetup();
        Parent().ChangeStateL(CBtmsAudio::NewL(Parent(), SwapStateRfcommSock(), SwapSyncSock(iAcceptSco)));
        }
    }
    
CBtmsOpenAudio::CBtmsOpenAudio(CBtmMan& aParent, TRequestStatus* aRequest, 
        CBtmRfcommSock* aRfcomm, CBtmSyncSock* aSco, TBool aUseEsco, EESCOParam aESCOParameter)
    : CBtmsInuse(aParent, aRequest, aRfcomm), iAcceptSco(aSco), iUseEsco(aUseEsco), iESCOParameter(aESCOParameter)
    {
    if (iAcceptSco)
        {
        iAcceptSco->SetObserver(*this);
        }
    iRfcomm->SetObserver(*this);
    }

void CBtmsOpenAudio::ConstructL()
    {
    }

