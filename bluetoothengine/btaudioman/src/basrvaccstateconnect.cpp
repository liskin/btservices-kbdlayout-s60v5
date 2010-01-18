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
* Description:  Implementation of Connect state.
*
*/


// INCLUDE FILES
#include "basrvaccstateconnect.h"
#include "basrvaccstateattach.h"
#include "basrvaccstateattached.h"
#include "debug.h"

const TInt KConnectRequestId = 1;

// ================= MEMBER FUNCTIONS =======================

CBasrvAccStateConnect* CBasrvAccStateConnect::NewL(CBasrvAcc& aParent)
    {
    CBasrvAccStateConnect* self = new (ELeave) CBasrvAccStateConnect(aParent);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBasrvAccStateConnect::~CBasrvAccStateConnect()
    {
    delete iActive;
    TRACE_FUNC
    }

void CBasrvAccStateConnect::EnterL()
    {
    StatePrint(_L("Connect"));    
    CBTAccPlugin* plugin = NULL;
    if (AccInfo().iSuppProfiles & EHFP)
        {
        iConnectingProfile = EHFP;
        }
    else if (AccInfo().iSuppProfiles & EHSP)
        {
        iConnectingProfile = EHSP;
        }
    else if (AccInfo().iSuppProfiles & EStereo)
        {
        iConnectingProfile = EStereo;
        }
    plugin = Parent().AccMan().PluginMan().Plugin(iConnectingProfile);
    LEAVE_IF_NULL(plugin)
    plugin->ConnectToAccessory(AccInfo().iAddr, iActive->iStatus);
    iActive->GoActive();
    }

CBasrvAccState* CBasrvAccStateConnect::ErrorOnEntry(TInt aReason)
    {
    TRACE_FUNC
    TRAP_IGNORE(Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, aReason, AccInfo().iSuppProfiles));
    return NULL;
    }

TBTEngConnectionStatus CBasrvAccStateConnect::ConnectionStatus() const
    {
    return EBTEngConnecting;
    }


void CBasrvAccStateConnect::CancelConnect()
    {
    TRACE_FUNC
    iActive->Cancel();
    TRAP_IGNORE(Parent().ChangeStateL(NULL));
    }

void CBasrvAccStateConnect::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_FUNC
    if (aActive.iStatus == KErrNone)
        {
        AccInfo().iConnProfiles |= iConnectingProfile;
        Parent().ChangeStateL(CBasrvAccStateAttach::NewL(Parent(), ETrue));
        }
    else
        {
        Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, aActive.iStatus.Int(), AccInfo().iSuppProfiles);
        Parent().ChangeStateL(NULL);
        }
    }
    
void CBasrvAccStateConnect::CancelRequest(CBasrvActive& /*aActive*/)
    {
    TRACE_FUNC
    Parent().AccMan().PluginMan().Plugin(iConnectingProfile)->CancelConnectToAccessory(AccInfo().iAddr);
    }

CBasrvAccStateConnect::CBasrvAccStateConnect(CBasrvAcc& aParent)
    : CBasrvAccState(aParent, NULL)
    {
    }
    
void CBasrvAccStateConnect::ConstructL()
    {
    iActive = CBasrvActive::NewL(*this, CActive::EPriorityStandard, KConnectRequestId);
    }

