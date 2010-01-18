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
* Description:  BT sync socket
*  Version     : %version: 4.2.4 %
*
*/


#include <es_sock.h>
#include "btmsyncsock.h"
#include "btmsockobserver.h"
#include "btmdefs.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmSyncSock* CBtmSyncSock::NewL(MBtmSockObserver& aNotifier, RSocketServ& aSockServ)
    {
    CBtmSyncSock* self = new (ELeave) CBtmSyncSock(aNotifier, aSockServ);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmSyncSock::~CBtmSyncSock()
    {
    TRACE_FUNC_ENTRY
    delete iSco;
    delete iEScoLink;
    TRACE_FUNC_EXIT
    }

void CBtmSyncSock::SetupL(const TBTDevAddr& aAddr, const TBTSyncPackets& aPackets)
    {
    TRACE_FUNC_ENTRY
	iSco->SetupConnection(aAddr, aPackets);
    TRACE_FUNC_EXIT
    }

void CBtmSyncSock::CancelSetup()
    {
    TRACE_FUNC_ENTRY
	iSco->CancelSetup();
	iSco->Disconnect();
    TRACE_FUNC_EXIT
    }
    
void CBtmSyncSock::DisconnectL()
    {
    TRACE_FUNC_ENTRY
    if (iConnectedLink)
        iConnectedLink->Disconnect();
    TRACE_FUNC_EXIT
    }

void CBtmSyncSock::AcceptL(TBool aAllowEsco)
    {
    TRACE_FUNC_ENTRY
    iScoAcceptor = TScoAcceptor(this);
    iSco->SetNotifier(iScoAcceptor);
    iSco->AcceptConnection(TBTSyncPackets(KScoPacketTypeSpec));
    if (aAllowEsco)
        {
        iESco_Acceptor = TScoAcceptor(this);
        iEScoLink = CBluetoothSynchronousLink::NewL(iESco_Acceptor, iServer);
        iEScoLink->AcceptConnection(TBTSyncPackets(KEscoPacketTypeSpec));
        }
    TRACE_FUNC_EXIT    
    }

void CBtmSyncSock::CancelAccept()
    {
    TRACE_FUNC_ENTRY
    if (iSco)
        {
        iSco->CancelAccept();
        }
    if (iEScoLink)
        {
        iEScoLink->CancelAccept();
        }
    TRACE_FUNC_EXIT    
    }

void CBtmSyncSock::SetMaxLatency(TUint16 aLatency)
    {
    iSco->SetMaxLatency(aLatency);
    if (iEScoLink)
        iEScoLink->SetMaxLatency(aLatency);
    }
    
void CBtmSyncSock::SetRetransmissionEffort(TBTeSCORetransmissionTypes aRetransmissionEffort)
    {
    iSco->SetRetransmissionEffort(aRetransmissionEffort);
    if (iEScoLink)
        iEScoLink->SetRetransmissionEffort(aRetransmissionEffort);
    }

void CBtmSyncSock::Reset()
    {
    TRACE_FUNC_ENTRY
    
    TRACE_FUNC_EXIT    
    }
    
void CBtmSyncSock::SetObserver(MBtmSockObserver& aObserver)
    {
    iObserver = &aObserver;
    }

TInt CBtmSyncSock::Latency()
    {
    TRACE_FUNC
    return TInt(iSco->Latency());
    }

void CBtmSyncSock::HandleSetupConnectionCompleteL(TInt aErr)
    {
    TRACE_FUNC
    iConnectedLink = iSco;
    iObserver->SyncLinkSetupCompleteL(aErr);
    }

void CBtmSyncSock::HandleDisconnectionCompleteL(TInt aErr)
    {
    TRACE_FUNC_ENTRY
    iConnectedLink = NULL;
    iObserver->SyncLinkDisconnectCompleteL(aErr);
    TRACE_FUNC_EXIT    
    }

void CBtmSyncSock::HandleAcceptConnectionCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

void CBtmSyncSock::HandleSendCompleteL(TInt aErr)
    {
    iObserver->SyncLinkSendCompleteL(aErr);
    }

void CBtmSyncSock::HandleReceiveCompleteL(TInt aErr)
    {
    TRACE_ERROR(_L("CBtmSyncSock::HandleReceiveCompleteL"))
    iObserver->SyncLinkReceiveCompleteL(aErr);
    }

CBtmSyncSock::CBtmSyncSock(MBtmSockObserver& aObserver, RSocketServ& aSockServ)
    : iObserver(&aObserver), iServer(aSockServ)
    {
    }

void CBtmSyncSock::ConstructL()
    {
    TRACE_FUNC
    iSco = CBluetoothSynchronousLink::NewL(*this, iServer);
    }

void CBtmSyncSock::AcceptRedirectL(TInt aErr, TScoAcceptor& aAcceptor)
    {
    TRACE_FUNC
    if (&aAcceptor == &iScoAcceptor)
        {
        TRACE_INFO((_L("Sync link type SCO")))
        iConnectedLink = iSco;
        if (iEScoLink)
            iEScoLink->CancelAccept();
        }
    else
        {
        TRACE_INFO((_L("Sync link type ESCO")))
        iConnectedLink = iEScoLink;
        iSco->CancelAccept();
        }
    iConnectedLink->SetNotifier(*this);
    iObserver->SyncLinkAcceptCompleteL(aErr);
    }

CBtmSyncSock::TScoAcceptor::TScoAcceptor()
    {
    TScoAcceptor(NULL);
    }

CBtmSyncSock::TScoAcceptor::TScoAcceptor(CBtmSyncSock* aParent)
    : iParent(aParent)
    {
    }

void CBtmSyncSock::TScoAcceptor::HandleSetupConnectionCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }
    
void CBtmSyncSock::TScoAcceptor::HandleDisconnectionCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }
void CBtmSyncSock::TScoAcceptor::HandleAcceptConnectionCompleteL(TInt aErr)
    {
    TRACE_FUNC
    iParent->AcceptRedirectL(aErr, *this);
    }
    
void CBtmSyncSock::TScoAcceptor::HandleSendCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }
void CBtmSyncSock::TScoAcceptor::HandleReceiveCompleteL(TInt /*aErr*/)
    {
    TRACE_FUNC
    }
