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
#include "btengdiscovery.h"
#include "btengconstants.h"
#include "btmsconnect.h"
#include "btmslisten.h"
#include "btmsrfcomm.h"
#include "btmsctrl.h"
#include "btmsopenaudio.h"
#include "btmrfcommsock.h"
#include "btmactive.h"
#include "debug.h"

const TInt KSearchHandsfreeChannel = 1;
const TInt KSearchHeadsetChannel = 2;
const TInt KSearchCompleted = 3;

// ======== MEMBER FUNCTIONS ========

CBtmsConnect* CBtmsConnect::NewL(CBtmMan& aParent, TRequestStatus* aRequest, 
    const TBTDevAddr& aAddr, TRequestCategory aRequestCat)
    {
    CBtmsConnect* self = new( ELeave ) CBtmsConnect(aParent, aRequest, aAddr, aRequestCat);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmsConnect::~CBtmsConnect()
    {
    TRACE_FUNC_ENTRY
    delete iBteng;
    delete iRfcomm;
    delete iDevice;
    TRACE_FUNC_EXIT
    }

void CBtmsConnect::EnterL()
    {
    TRACE_STATE(_L("[BTMAC State] Connect"))    
    *StateRequest() = KRequestPending;
    iSearchState = KSearchHandsfreeChannel;
    TInt err = iBteng->RemoteProtocolChannelQuery(iSockAddr.BTAddr(), EBTProfileHFP);
    if(err)
    	{
    	CompleteStateRequest(err);
    	}
    }

void CBtmsConnect::CancelConnectL()
    {
    TRACE_FUNC
    if (iRequestCat == ERequestConnect)
        {
        if (iRfcomm)
            {
            iRfcomm->CancelConnect();
            }
        if (iBteng)
            {
            iBteng->CancelRemoteSdpQuery();
            }
        CompleteStateRequest(KErrCancel);
        Parent().ChangeStateL(CBtmsListen::NewL(Parent()));
        }
    }

void CBtmsConnect::CancelOpenAudioLinkL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    if (aAddr == iSockAddr.BTAddr() && iRequestCat == ERequestOpenAudio)
        {
        CompleteStateRequest(KErrCancel);
        iRequestCat = ERequestNone;
        }
    }

CBtmRfcommSock* SwapRfcommSock(CBtmRfcommSock*& aSock)
    {
    CBtmRfcommSock* sock = aSock;
    aSock = NULL;
    return sock;
    }

void CBtmsConnect::RfcommConnectCompletedL(TInt aErr)
    {
    TRACE_FUNC
    if (aErr)
        {
        CompleteStateRequest(aErr);
        Parent().ChangeStateL(CBtmsListen::NewL(Parent()));
        }
    else
        {
        if (iRfcomm->Service() == EBTProfileHFP)
            {
            Parent().ChangeStateL(CBtmsRfcomm::NewL(Parent(),SwapStateRequest(), SwapRfcommSock(iRfcomm), iRequestCat));
            }
        else
            {
            if (iRequestCat == ERequestConnect)
                {
                CompleteStateRequest(aErr);
                Parent().ChangeStateL(CBtmsCtrl::NewL(Parent(), SwapRfcommSock(iRfcomm), NULL));
                }
            else if (iRequestCat == ERequestOpenAudio)
                {
                Parent().ChangeStateL(CBtmsOpenAudio::NewL(Parent(),SwapStateRequest(), SwapRfcommSock(iRfcomm), NULL));
                }
            }
        }
    }

void CBtmsConnect::AttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, const RSdpResultArray &aAttr, TInt aErr )
    {
    TRACE_FUNC
    if (iSearchState == KSearchHandsfreeChannel)
        {
        if (aErr == KErrNone || (aErr == KErrEof && aAttr.Count()>0))
            {
            RSdpResultArray attr(aAttr);
            CBTEngDiscovery::ParseRfcommChannel(attr, iRemoteChannel);
            TRACE_INFO((_L("HFP channel %d"), iRemoteChannel))
            TRAP_IGNORE(DoSockConnectL(EBTProfileHFP));
            iSearchState = KSearchCompleted;
            }
        else if (aErr == KErrEof && aAttr.Count()== 0)
            {
            iSearchState = KSearchHeadsetChannel;
						iBteng->RemoteProtocolChannelQuery(iSockAddr.BTAddr(), EBTProfileHSP);
            }
        else
            {
            CompleteStateRequest(aErr);
            TRAP_IGNORE(Parent().ChangeStateL(CBtmsListen::NewL(Parent())));
            }
        }
    else if (iSearchState == KSearchHeadsetChannel)
        {
        if (aErr == KErrNone || (aErr == KErrEof && aAttr.Count()>0))
            {
            RSdpResultArray attr(aAttr);
            CBTEngDiscovery::ParseRfcommChannel(attr, iRemoteChannel);
            TRACE_INFO((_L("HSP channel %d"), iRemoteChannel))
            TRAP_IGNORE(DoSockConnectL(EBTProfileHSP));
            iSearchState = KSearchCompleted;
            }
        else
            {
            CompleteStateRequest(aErr);
            TRAP_IGNORE(Parent().ChangeStateL(CBtmsListen::NewL(Parent())));
            }
        }
    }


CBtmsConnect::CBtmsConnect(CBtmMan& aParent, TRequestStatus* aRequest, 
    const TBTDevAddr& aAddr, TRequestCategory aRequestCat)
    : CBtmState(aParent, aRequest), iRequestCat(aRequestCat)
    {
    iSockAddr.SetBTAddr(aAddr);
    }

void CBtmsConnect::ConstructL()
    {
    TRACE_FUNC
    iBteng = CBTEngDiscovery::NewL(this);
    iRfcomm = CBtmRfcommSock::NewL(*this, Parent().SockServ());
    }
    
void CBtmsConnect::DoSockConnectL(TUint aService)
    {
    TRACE_FUNC
    iSockAddr.SetBTAddr(iSockAddr.BTAddr());
	TBTServiceSecurity sec;
   	sec.SetAuthentication(ETrue);
   	sec.SetEncryption(ETrue);
   	iSockAddr.SetSecurity(sec);
    iSockAddr.SetPort(iRemoteChannel);
    iRfcomm->ConnectL(iSockAddr, aService);
    }
void CBtmsConnect::ServiceSearchComplete( const RSdpRecHandleArray& /*aResult*/, TUint /*aTotalRecordsCount*/, TInt /*aErr*/) {}

void CBtmsConnect::DeviceSearchComplete( CBTDevice* /*aDevice*/, TInt /*aErr*/ ) {}

void CBtmsConnect::ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, const RSdpResultArray& aAttr, TInt aErr ) 
	{
	AttributeSearchComplete(aHandle, aAttr, aErr);
	}

