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
* Description:  The base state definition
*  Version     : %version: 2.1.5 %
*
*  Copyright © 2005-2006 Nokia.  All rights reserved.
*/


// INCLUDE FILES
#include "btmstate.h"
#include "btmsidle.h"
#include "btmslisten.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBtmState::~CBtmState()
    {
    CompleteStateRequest(KErrAbort);
    }

void CBtmState::SetNotifyItemAtEntry(TNotifyItem aNotify)
    {
    iNotify = aNotify;
    }

TNotifyItem CBtmState::NotifyItem()
    {
    return iNotify;
    }
    
// ---------------------------------------------------------------------------
// returns CBtmsListen
// ---------------------------------------------------------------------------
//
CBtmState* CBtmState::ErrorOnEntryL(TInt /*aReason*/)
    {
    TRACE_FUNC
    return CBtmsListen::NewL(iParent);
    }

// ---------------------------------------------------------------------------
// Completes with KErrInUse by default
// ---------------------------------------------------------------------------
//
void CBtmState::ConnectL(const TBTDevAddr& /*aAddr*/, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    CompleteRequest(&aStatus, KErrInUse);
    }

void CBtmState::CancelConnectL()
    {
    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// Completes with KErrNotFound by default
// ---------------------------------------------------------------------------
//
void CBtmState::DisconnectL(const TBTDevAddr& /*aAddr*/, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    CompleteRequest(&aStatus, KErrNotFound);
    }

// ---------------------------------------------------------------------------
// Completes with KErrNotReady by default
// ---------------------------------------------------------------------------
//
void CBtmState::OpenAudioLinkL(const TBTDevAddr& /*aAddr*/, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    CompleteRequest(&aStatus, KErrNotFound);
    }

void CBtmState::CancelOpenAudioLinkL(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
    }

// ---------------------------------------------------------------------------
// Completes with KErrNotFound by default
// ---------------------------------------------------------------------------
//
void CBtmState::CloseAudioLinkL(const TBTDevAddr& /*aAddr*/, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    CompleteRequest(&aStatus, KErrNotFound);
    }

void CBtmState::CancelCloseAudioLinkL(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
    }

void CBtmState::AccInUse()
    {
    TRACE_FUNC
    }
    
void CBtmState::SlcIndicateL(TBool /*aSlc*/)
	{
	TRACE_FUNC
	}
	
void CBtmState::SendProtocolDataL(const TDesC8& /*aData*/)
	{
	TRACE_FUNC
	}

TBool CBtmState::CanDisableNrec()
    {
    return EFalse;
    }

TBTDevAddr CBtmState::Remote()
    {
    return TBTDevAddr();
    }

CBtmState::CBtmState(CBtmMan& aParent, TRequestStatus* aRequest)
    : iParent(aParent), iRequest(aRequest)
    {
    }

CBtmMan& CBtmState::Parent()
    {
    return iParent;
    }

void CBtmState::CompleteStateRequest(TInt aErr)
    {
    if (iRequest)
        {
        TRACE_INFO((_L("State request 0x%08X completed with %d"), iRequest, aErr))
        User::RequestComplete(iRequest, aErr);
        }
    }
    
void CBtmState::CompleteRequest(TRequestStatus* aStatus, TInt aErr)
    {
    if (aStatus)
        {
        TRACE_INFO((_L("Request 0x%08X completed with %d"), aStatus, aErr))
        User::RequestComplete(aStatus, aErr);
        }
    }

TRequestStatus* CBtmState::StateRequest() 
    { 
    return iRequest; 
    }

void CBtmState::SetStateRequest(TRequestStatus& aStatus)
    {
    iRequest = &aStatus;
    }
    
TRequestStatus* CBtmState::SwapStateRequest() 
    { 
    TRequestStatus* swap = iRequest;
    iRequest = NULL;
    return swap; 
    }

void CBtmState::StartListenerL()
    {
    TRACE_FUNC
    }

TInt CBtmState::AudioLinkLatency()
    {
    TRACE_FUNC
    return KErrNotFound;
    }

	
