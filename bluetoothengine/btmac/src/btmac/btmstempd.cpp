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
* Description:  The temporarily disconnect state definition
*
*/

//#include <bteng.h>
#include "btmstempd.h"
#include "btmsctrl.h"
#include "btmsyncsock.h"
#include "btmrfcommsock.h"
#include "btmslisten.h"
#include "btmsreconnect.h"
#include "btmsdisconnect.h"
#include "debug.h"
#include <btengconstants.h>

// ======== MEMBER FUNCTIONS ========

CBtmsHsTempd* CBtmsHsTempd::NewL(CBtmMan& aParent, const TBTDevAddr& aConnectedAddr)
    {
    CBtmsHsTempd* self = new( ELeave ) CBtmsHsTempd(aParent, aConnectedAddr);
    return self;
    }

CBtmsHsTempd::~CBtmsHsTempd()
    {
    }

void CBtmsHsTempd::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Tempd"))    
    Parent().DeleteCmdHandler();
    
	TBTServiceSecurity sec;
   	sec.SetAuthorisation(ETrue);
   	sec.SetAuthentication(ETrue);
   	sec.SetEncryption(ETrue);
   	TUint ag = KBTSdpHeadsetAudioGateway;
    iRfcomm = CBtmRfcommSock::NewL(*this, Parent().SockServ());
   	sec.SetUid(TUid::Uid(KBTSdpHeadsetAudioGateway));
   	iRfcomm->SetService(EBTProfileHSP);
   	TInt lastUsedPort = Parent().GetLastUsedChannel(KBTSdpHeadsetAudioGateway);
   	Parent().RegisterServiceL(ag, iRfcomm->ListenL(ag, sec,lastUsedPort));
    }
    
void CBtmsHsTempd::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
	TRACE_FUNC
    aStatus = KRequestPending;
    if (aAddr == iConnectedAddr)
        {
        CompleteRequest(&aStatus, KErrNone);
    	Parent().ChangeStateL(CBtmsListen::NewL(Parent()));
        }
	else
	    {
	    CompleteRequest(&aStatus, KErrNotFound);
	    }
	}

void CBtmsHsTempd::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
	TRACE_FUNC
	aStatus = KRequestPending;
    if (aAddr == iConnectedAddr)
        {
        iRfcomm->CancelListen();
        Parent().ChangeStateL(CBtmsReconnect::NewL(Parent(), &aStatus, aAddr));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
	}
	
void CBtmsHsTempd::RfcommErrorL(TInt /*aErr*/)
    {
    // Not possible to happen
    }

void CBtmsHsTempd::RfcommAcceptCompletedL(TInt /*aErr*/, TUint /*aService*/)
    {
    TRACE_FUNC_ENTRY
    if (iConnectedAddr == iRfcomm->Remote())
        {
        CBtmRfcommSock* rfcomm = iRfcomm;
        iRfcomm = NULL;
        Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), rfcomm, NULL));
        }
    else
        {
        iRfcomm->Disconnect();
        Parent().ChangeStateL(CBtmsHsTempd::NewL(Parent(), iConnectedAddr));
        }
    TRACE_FUNC_EXIT
    }


CBtmsHsTempd::CBtmsHsTempd(CBtmMan& aParent, const TBTDevAddr& aConnectedAddr)
    : CBtmsInuse(aParent, NULL, NULL), iConnectedAddr(aConnectedAddr)
    {
    }
