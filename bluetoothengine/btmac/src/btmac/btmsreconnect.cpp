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
* Description:  The connect state definition
*
*/


// INCLUDE FILES
//#include <bteng.h>
#include "btengdiscovery.h"
#include "btengconstants.h"

#include "btmsreconnect.h"
#include "btmslisten.h"
#include "btmstempd.h"
#include "btmsopenaudio.h"
#include "btmsctrl.h"
#include "btmrfcommsock.h"
#include "btmactive.h"
#include "btmsyncsock.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmsReconnect* CBtmsReconnect::NewL(CBtmMan& aParent, TRequestStatus* aRequest, const TBTDevAddr& aAddr)
    {
    CBtmsReconnect* self = new( ELeave ) CBtmsReconnect(aParent, aRequest, aAddr);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsReconnect::~CBtmsReconnect()
    {
    delete iBteng;
    }

void CBtmsReconnect::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] HSP reconnect"))    
    TInt err = iBteng->RemoteProtocolChannelQuery(iSockAddr.BTAddr(), EBTProfileHSP);
    if(err)
    	{
    	CompleteStateRequest(err);
    	}
    }

void CBtmsReconnect::DisconnectL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    aStatus = KRequestPending;
    if (aAddr == iSockAddr.BTAddr())
        {
        if (iRfcomm)
            {
            iRfcomm->CancelConnect();
            }
        if (iBteng)
            {
            iBteng->CancelRemoteSdpQuery();            
            }
        CompleteRequest(&aStatus, KErrNone);
        Parent().ChangeStateL(CBtmsListen::NewL(Parent()));
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsReconnect::OpenAudioLinkL(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    aStatus = KRequestPending;
    if (aAddr == iSockAddr.BTAddr())
        {
        CompleteRequest(&aStatus, KErrAlreadyExists);
        }
    else
        {
        CompleteRequest(&aStatus, KErrNotFound);
        }
    }

void CBtmsReconnect::CancelOpenAudioLinkL(const TBTDevAddr& aAddr)
    {
    if (aAddr == iSockAddr.BTAddr())
        {
        if (iRfcomm)
            {
            iRfcomm->CancelConnect();
            }
        CompleteStateRequest(KErrCancel);
        Parent().ChangeStateL(CBtmsHsTempd::NewL(Parent(), iSockAddr.BTAddr()));
        }
    }

void CBtmsReconnect::RfcommErrorL(TInt /*aErr*/)
    {
    // Not possible to happen
    }


void CBtmsReconnect::AttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, const RSdpResultArray &aAttr, TInt aErr )
    {
    if (aErr == KErrNone)
    	{
        RSdpResultArray attr(aAttr);
        CBTEngDiscovery::ParseRfcommChannel(attr, iRemoteChannel);
        TRACE_INFO((_L("HSP channel %d"), iRemoteChannel))
        TRAP_IGNORE(DoSockConnectL(iRemoteChannel));
        }
    else
        {
        CompleteStateRequest(aErr);
        TRAP_IGNORE(Parent().ChangeStateL(CBtmsHsTempd::NewL(Parent(), iSockAddr.BTAddr())));
        }
    }


CBtmsReconnect::CBtmsReconnect(CBtmMan& aParent, TRequestStatus* aRequest, const TBTDevAddr& aAddr)
    : CBtmsInuse(aParent, aRequest, NULL)
    {
    iSockAddr.SetBTAddr(aAddr);
    }

void CBtmsReconnect::ConstructL()
    {
    iBteng = CBTEngDiscovery::NewL(this);
    }
    
void CBtmsReconnect::DoSockConnectL(TUint aService)
    {
    iRfcomm = CBtmRfcommSock::NewL(*this, Parent().SockServ());
    iSockAddr.SetBTAddr(iSockAddr.BTAddr());
	TBTServiceSecurity sec;
   	sec.SetAuthentication(ETrue);
   	sec.SetEncryption(ETrue);
   	iSockAddr.SetSecurity(sec);
    iSockAddr.SetPort(iRemoteChannel);
    iRfcomm->ConnectL(iSockAddr, aService);
    }

void CBtmsReconnect::RfcommConnectCompletedL(TInt aErr)
    {
    TRACE_FUNC
    if (aErr)
        {
        CompleteStateRequest(aErr);
        Parent().ChangeStateL(CBtmsHsTempd::NewL(Parent(), iSockAddr.BTAddr()));
        }
    else
        {
        Parent().LoadCmdHandlerL(EBtmcHSP, iRfcomm->Remote(), iRfcomm->AccessoryInitiatedConnection());
        Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(), SwapStateRequest(), SwapStateRfcommSock(), NULL));
        }
    }

void CBtmsReconnect::ServiceSearchComplete( const RSdpRecHandleArray& /*aResult*/, TUint /*aTotalRecordsCount*/, TInt /*aErr*/ ) {}

void CBtmsReconnect::DeviceSearchComplete( CBTDevice* /*aDevice*/, TInt /*aErr*/ ) {}

void CBtmsReconnect::ServiceAttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, const RSdpResultArray& aAttr, TInt aErr ) 
	{
  if (aErr == KErrNone || (aErr == KErrEof && aAttr.Count()>0))
    {
    RSdpResultArray attr(aAttr);
    CBTEngDiscovery::ParseRfcommChannel(attr, iRemoteChannel);
    TRACE_INFO((_L("HSP channel %d"), iRemoteChannel))
    TRAP_IGNORE(DoSockConnectL(EBTProfileHSP));
    }
	else
    {
    CompleteStateRequest(aErr);
    TRAP_IGNORE(Parent().ChangeStateL(CBtmsListen::NewL(Parent())));
    }	
	}
  
