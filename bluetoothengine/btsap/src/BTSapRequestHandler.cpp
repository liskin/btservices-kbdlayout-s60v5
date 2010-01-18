/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     This class handles data exchange between BTSap server and client
*
*
*/


// INCLUDE FILES
#include "BTSapRequestHandler.h"
#include "BTSapServerState.h"
#include "BTSapSocketHandler.h"
#include "debug.h"

const TInt KSegmentTimeout = 1000000; // 1s

CBTSapRequestHandler::CBTSapRequestHandler(CBTSapServerState& aServerState)
    : CActive(EPriorityNormal), 
      iServerState(aServerState),
      iRequestMessage(aServerState.BTSapRequestMessage())
    {
    CActiveScheduler::Add(this);
    }

CBTSapRequestHandler::~CBTSapRequestHandler()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  ~CBTSapRequestHandler")));

    Cancel();
    delete iSegmentTimer;
    }

// ---------------------------------------------------------
// NewL()
// ---------------------------------------------------------
//
CBTSapRequestHandler* CBTSapRequestHandler::NewL(CBTSapServerState& aServerState)
    {
    CBTSapRequestHandler* self = new (ELeave) CBTSapRequestHandler(aServerState);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CBTSapRequestHandler::ConstructL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapRequestHandler: ConstructL")));

    iSegmentTimer = CSegmentTimer::NewL(this);

    iStatus = KRequestPending;
    SetActive();
    }

// ---------------------------------------------------------
// DoCancel
// ---------------------------------------------------------
//
void CBTSapRequestHandler::DoCancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapRequestHandler: DoCancel")));

    TRequestStatus* status = &iStatus;
	User::RequestComplete(status, KErrCancel);
    }

// ---------------------------------------------------------
// RunL
// ---------------------------------------------------------
//
void CBTSapRequestHandler::RunL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapRequestHandler: RunL")));
    TInt status = iStatus.Int();
    
    if ( status != KErrNone )
        {
        HandleSapMessage(EInvalidSegmented);
        }
    else
        {
        while (!iRawMessage.IsEmpty())
            {
            TValidationResult result = iRawMessage.Validate();
           
            if (result == EInvalidSegmented)
	            {
	            // start timer if not started
                iSegmentTimer->Cancel();
                iSegmentTimer->After(KSegmentTimeout);
                break;
                }
            else
                {
                // complete message
                HandleSapMessage(result);
                }
            }
        }
    iStatus = KRequestPending;
    SetActive();
    }

void CBTSapRequestHandler::HandleSapMessage(const TValidationResult aResult)
    {
    // stop timer
    iSegmentTimer->Cancel();

    iRequestMessage.SetData(iRawMessage.Data());
    //DEB(Hex(_L8("[BTSap]  Request: %S"), &iRequestMessage.Data()));

    iRawMessage.Reset();

    TBTSapServerState nextState = EStateIdle;

    if (aResult == EValidFormat)
        {
        nextState = KStateByRequest[(TInt)iRequestMessage.MsgID()];
        }

    if (iServerState.HandleStateChangeRequest(nextState) != KErrNone)
        {
        iServerState.SendErrorResponse();

        if (nextState != iServerState.CurrentState())
            {
            iServerState.ChangeState(nextState);
            }
        }
    else
        {
        // always change state if request is acceptable
        iServerState.ChangeState(nextState);
        }
    }

void CBTSapRequestHandler::HandleSapData(const TDes8& aData)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapRequestHandler: HandleSapData")));

    TInt result = iRawMessage.AppendData(aData);
    
    if (IsActive() && iStatus == KRequestPending)
        {
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, result);
        }

    }

void CBTSapRequestHandler::HandleSegmentTimeout()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapRequestHandler: HandleIncompleteMessage")));

    TValidationResult result = iRawMessage.Validate();
    HandleSapMessage(result);
    }

CBTSapRequestHandler::CSegmentTimer::CSegmentTimer(CBTSapRequestHandler* aRequestHandler)
    : CTimer(EPriorityStandard), iRequestHandler(aRequestHandler)
	{
	CActiveScheduler::Add(this);
	}

CBTSapRequestHandler::CSegmentTimer* CBTSapRequestHandler::CSegmentTimer::NewL(
    CBTSapRequestHandler* aRequestHandler)
	{
	CSegmentTimer* self = new (ELeave) CSegmentTimer(aRequestHandler);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CBTSapRequestHandler::CSegmentTimer::RunL()
	{
	iRequestHandler->HandleSegmentTimeout();
	}

// End of file
