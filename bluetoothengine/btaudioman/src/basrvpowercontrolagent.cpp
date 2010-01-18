/*
* Copyright (c) 2005-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


// INCLUDE FILES
#include "basrvpowercontrolagent.h"
#include "debug.h"

enum TActiveRequestId
    {
    ESniffDelayTimer = 80,
    EBbEventSubscribe = 81,
    };

const TInt KSniffRequesterDelay = 5000000; // 5 secs
    
CBasrvPowerControlAgent* CBasrvPowerControlAgent::NewL(const TBTDevAddr& aAddr)
    {
    CBasrvPowerControlAgent* self = new CBasrvPowerControlAgent(aAddr);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }

CBasrvPowerControlAgent::~CBasrvPowerControlAgent()
    {
    TRACE_FUNC
    delete iBbEventer;
    delete iSniffDelayTimer;
    iTimer.Close();
    iPlAda.Close();
    iSockServ.Close();
    }

void CBasrvPowerControlAgent::RequestSniffMode()
    {
    TRACE_FUNC
    if (iState == ESniffModeRequested || iState == ESniffModeTiming)
        {
        return;
        }
    if (iState == EActiveModeRequested)
        {
        iPlAda.CancelLowPowerModeRequester();
        }
    iState = ESniffModeTiming;
    iTimer.After(iSniffDelayTimer->iStatus, KSniffRequesterDelay);
    iSniffDelayTimer->GoActive();
    TRACE_INFO(_L("CBasrvPowerControlAgent, [Sniff mode Timing]"))
    }

void CBasrvPowerControlAgent::RequestActiveMode()
    {
    TRACE_FUNC
    if (iState == EActiveModeRequested)
        {
        return;
        }
    if (iState == ESniffModeTiming)
        {
        iSniffDelayTimer->Cancel();
        }
    TInt err = iPlAda.ActivateActiveRequester();
    if (!err)
        {
        iState = EActiveModeRequested;
        TRACE_INFO(_L("CBasrvPowerControlAgent, [Active mode requested]"))
        }
    else
        {
        TRACE_INFO((_L("CBasrvPowerControlAgent, ActivateActiveRequester %d"), err))
        iState = EPowerModeDefault;
        }
    }

void CBasrvPowerControlAgent::CancelPowerModeControl()
    {
    TRACE_FUNC
    if (iState == ESniffModeTiming)
        {
        iSniffDelayTimer->Cancel();
        }
    else if (iState == ESniffModeRequested || iState == EActiveModeRequested)
        {
        iPlAda.CancelLowPowerModeRequester();
        }
    iState = EPowerModeDefault;
    TRACE_INFO(_L("CBasrvPowerControlAgent, [Power mode default]"))
    }

void CBasrvPowerControlAgent::PreventLowPowerMode()
    {
    TRACE_FUNC
    (void) iPlAda.PreventLowPowerModes(EAnyLowPowerMode);
    }
    
void CBasrvPowerControlAgent::AllowLowPowerMode()
    {
    TRACE_FUNC
    (void) iPlAda.AllowLowPowerModes(EAnyLowPowerMode);
    }


void CBasrvPowerControlAgent::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_FUNC
    switch (aActive.RequestId())
        {
        case ESniffDelayTimer:
            {
            DosniffRequest();
            break;
            }
        case EBbEventSubscribe:
            {
        	TRACE_INFO((_L("CBasrvPowerControlAgent, err %d, event 0x%02X"), aActive.iStatus.Int(), iBbEvent().EventType()))
            if (aActive.iStatus != KErrNone)
                return;
        	switch(iBbEvent().EventType())
        		{
        		case ENotifyActiveMode://0x04
        		    {
                    if (iState == ESniffModeTiming || iState == ESniffModeRequested)
                        {
                        CancelPowerModeControl();
                        }
        			break;
        		    }
        		case ENotifySniffMode: //0x08
        		    {
                    if (iState == ESniffModeTiming)
                        {
                        iSniffDelayTimer->Cancel();
                        DosniffRequest();
                        }
        			break;
        		    }
        		default:
        			{
        			TRACE_INFO((_L("CBasrvPowerControlAgent, Uninteresting event (0x%08X)"), iBbEvent().EventType()))
        			}
        		}
            iPlAda.NotifyNextBasebandChangeEvent(iBbEvent, iBbEventer->iStatus, ENotifyAnyPowerMode);
            iBbEventer->GoActive();
            }
        }
    }
    
void CBasrvPowerControlAgent::CancelRequest(CBasrvActive& aActive)
    {
    TRACE_FUNC
    if (aActive.RequestId() == ESniffDelayTimer)
        {
        iTimer.Cancel();
        }
    else if (aActive.RequestId() == EBbEventSubscribe)
        {
        iPlAda.CancelNextBasebandChangeEventNotifier();
        }
    }

CBasrvPowerControlAgent::CBasrvPowerControlAgent(const TBTDevAddr& aAddr)
    : iAddr(aAddr)
    {
    TRACE_FUNC
    }

void CBasrvPowerControlAgent::ConstructL()
    {
    LEAVE_IF_ERROR(iSockServ.Connect())
    LEAVE_IF_ERROR(iPlAda.Open(iSockServ, iAddr))
    iBbEventer = CBasrvActive::NewL(*this, CActive::EPriorityStandard, EBbEventSubscribe);
    iSniffDelayTimer = CBasrvActive::NewL(*this, CActive::EPriorityStandard, ESniffDelayTimer);
    LEAVE_IF_ERROR(iTimer.CreateLocal())
    iPlAda.NotifyNextBasebandChangeEvent(iBbEvent, iBbEventer->iStatus, ENotifyAnyPowerMode);
    iBbEventer->GoActive();
    }

void CBasrvPowerControlAgent::DosniffRequest()
    {
    TRACE_FUNC  
    (void) iPlAda.AllowLowPowerModes(EAnyLowPowerMode);
    (void) iPlAda.CancelLowPowerModeRequester();
    TInt err = iPlAda.ActivateSniffRequester();
    if (!err)
        {
        TRACE_INFO(_L("CBasrvPowerControlAgent, [Sniff mode requested]"))
        iState = ESniffModeRequested;
        }
    else
        {
        TRACE_INFO((_L("CBasrvPowerControlAgent, ActivateSniffRequester %d"), err))
        iState = EPowerModeDefault;
        }
    }
