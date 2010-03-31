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
* Description:  The listen state definition
*
*/


//#include <bteng.h>
#include <btaccTypes.h>
#include "btmslisten.h"
#include "btmsidle.h"
#include "btmsconnect.h"
#include "btmsrfcomm.h"
#include "btmsctrl.h"
#include "btmrfcommsock.h"
#include "debug.h"
#include <btengconstants.h>


// ======== MEMBER FUNCTIONS ========

CBtmsListen* CBtmsListen::NewL(CBtmMan& aParent)
    {
    CBtmsListen* self = new( ELeave ) CBtmsListen(aParent);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsListen::~CBtmsListen()
    {
    delete iHfpSock;
    delete iHspSock;
    Parent().StoppedListenning();
    }

void CBtmsListen::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Listen"))
    iHfpSock = CBtmRfcommSock::NewL(*this, Parent().SockServ());
    iHspSock = CBtmRfcommSock::NewL(*this, Parent().SockServ());
    if ( Parent().IsTrashBinEmpty() )
        {
        StartListenerL();
        }
    }
CBtmState* CBtmsListen::ErrorOnEntryL(TInt /*aReason*/)
    {
    TRACE_FUNC
    return CBtmsIdle::NewL(Parent());
    }

void CBtmsListen::StartListenerL()
    {
    TRACE_STATE(_L("[BTMAC State] Start Listener "))    
    Parent().DeleteCmdHandler();
    TBTServiceSecurity sec;
    sec.SetAuthorisation(ETrue);
    sec.SetAuthentication(ETrue);
    sec.SetEncryption(ETrue);
    TUint ag = KBTSdpHandsfreeAudioGateway;
    TInt lastUsedPort = Parent().GetLastUsedChannel(ag);
    TRACE_STATE((_L("[BTMAC State] Start EBTProfileHFP, last used port %d"), lastUsedPort)) 
    iHfpSock->SetObserver(*this);
    sec.SetUid(TUid::Uid(KBTSdpHandsfreeAudioGateway));
    Parent().RegisterServiceL(ag, iHfpSock->ListenL(ag, sec,lastUsedPort));
    iHfpSock->SetService(EBTProfileHFP); 
    iHspSock->SetObserver(*this);
    sec.SetUid(TUid::Uid(KBTSdpHandsfreeAudioGateway));
    ag = KBTSdpHeadsetAudioGateway;
    lastUsedPort = Parent().GetLastUsedChannel(ag);
    TRACE_STATE((_L("[BTMAC State] Start EBTProfileHSP, last used port %d"), lastUsedPort)) 
    Parent().RegisterServiceL(ag, iHspSock->ListenL(ag, sec,lastUsedPort));
    iHspSock->SetService(EBTProfileHSP);
    Parent().StartedListenning();
    }

void CBtmsListen::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
	TRACE_FUNC
	if ( iHfpSock && iHspSock )
	    {
	    iHfpSock->CancelListen();
	    iHspSock->CancelListen();
	    Parent().ChangeStateL(CBtmsConnect::NewL(Parent(), &aStatus, aAddr, ERequestOpenAudio));
	    }
	}

void CBtmsListen::ConnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    if ( iHfpSock && iHspSock )
        {
        iHfpSock->CancelListen();
        iHspSock->CancelListen();
        Parent().ChangeStateL(CBtmsConnect::NewL(Parent(), &aStatus, aAddr, ERequestConnect));
        }
    }

void CBtmsListen::RfcommAcceptCompletedL(TInt aErr, TUint aService)
    {
    TRACE_FUNC_ENTRY
    if (!aErr)
        {
        CBtmRfcommSock* sock;
        if (aService == EBTProfileHFP)
            {
            iHspSock->CancelListen();
            sock = iHfpSock;
            iHfpSock = NULL;
            Parent().ChangeStateL(CBtmsRfcomm::NewL(Parent(), NULL, sock, ERequestNone));
            }
        else
            {
            iHfpSock->CancelListen();
            sock = iHspSock;
            iHspSock = NULL;
            Parent().NewAccessory(sock->Remote(), EHSP);
            Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), sock, NULL));
            }
        
        }
    TRACE_FUNC_EXIT
    }

CBtmsListen::CBtmsListen(CBtmMan& aParent) : CBtmState(aParent, NULL)
    {
    }

void CBtmsListen::ConstructL()
    {
    }

