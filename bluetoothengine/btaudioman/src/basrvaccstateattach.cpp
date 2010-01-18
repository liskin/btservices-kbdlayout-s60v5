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
* Description:  Implementation of Attach state.
*
*/


// INCLUDE FILES
#include "basrvaccstateattach.h"
#include "basrvaccstatedisconnect.h"
#include "basrvaccstateattached.h"
#include "debug.h"

const TInt KAttachRequestId = 10;

// ================= MEMBER FUNCTIONS =======================

CBasrvAccStateAttach* CBasrvAccStateAttach::NewL(CBasrvAcc& aParent, TBool aConnecting)
    {
    CBasrvAccStateAttach* self=new(ELeave) CBasrvAccStateAttach(aParent, aConnecting);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBasrvAccStateAttach::~CBasrvAccStateAttach()
    {
    delete iActive;
    TRACE_FUNC
    }

void CBasrvAccStateAttach::EnterL()
    {
    StatePrint(_L("Attach"));    
    Parent().AccMan().AccfwConnectionL(&(AccInfo()))->AttachAccessory(AccInfo().iAddr, iActive->iStatus);
    iActive->GoActive();
    }

CBasrvAccState* CBasrvAccStateAttach::ErrorOnEntry(TInt aReason)
    {
    TRACE_FUNC
    TRAP_IGNORE(Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, aReason, AccInfo().iSuppProfiles));
    CBasrvAccState* next = NULL;
    TRAP_IGNORE(next = CBasrvAccStateDisconnect::NewL(Parent()));
    return next;
    }

TBTEngConnectionStatus CBasrvAccStateAttach::ConnectionStatus() const
    {
    return EBTEngConnecting;
    }

void CBasrvAccStateAttach::CancelConnect()
    {
    TRACE_FUNC
    iActive->Cancel();
    CBasrvAccState* next = NULL;
    TRAP_IGNORE(next = CBasrvAccStateDisconnect::NewL(Parent()));
    TRAP_IGNORE(Parent().ChangeStateL(next));
    }


void CBasrvAccStateAttach::DisconnectL()
    {
    TRACE_FUNC
    iActive->Cancel();
    Parent().ChangeStateL(CBasrvAccStateDisconnect::NewL(Parent()));
    }
     
void CBasrvAccStateAttach::AccessoryDisconnectedL(TProfiles aProfile)
    {
    TRACE_FUNC
    ProfileDisconnected(aProfile);
    AccClosedAudio(aProfile);
    StatePrint(_L("Attach"));
    if (!AccInfo().iConnProfiles)
        {
        if (iConnecting)
            {
            Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, KErrDisconnected, AccInfo().iSuppProfiles);
            }
        iActive->Cancel();
        Parent().ChangeStateL(NULL);
        }
    }

void CBasrvAccStateAttach::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_FUNC
    if (iConnecting && aActive.iStatus == KErrNone)
        {
        Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, aActive.iStatus.Int(), AccInfo().iConnProfiles);
        }
        
    if (aActive.iStatus == KErrNone)
        {
        Parent().AccMan().ListenAudioRequestL();        	
        Parent().AccMan().PluginMan().AccInUse();
        Parent().ChangeStateL(CBasrvAccStateAttached::NewL(Parent(), !iConnecting));
        }
    else
        {
        Parent().ChangeStateL(CBasrvAccStateDisconnect::NewL(Parent(), aActive.iStatus.Int()));
        }
    }
    
void CBasrvAccStateAttach::CancelRequest(CBasrvActive& /*aActive*/)
    {
    TRACE_FUNC
    CBasrvAccfwIf* accif = NULL;
    TRAP_IGNORE(accif = Parent().AccMan().AccfwConnectionL());
    if (accif)
        accif->CancelAttachAccessory(AccInfo().iAddr);
    }

CBasrvAccStateAttach::CBasrvAccStateAttach(CBasrvAcc& aParent, TBool aConnecting)
    : CBasrvAccState(aParent, NULL), iConnecting(aConnecting)
    {
    }
 
void CBasrvAccStateAttach::ConstructL()
    {
    iActive = CBasrvActive::NewL(*this, CActive::EPriorityStandard, KAttachRequestId);
    }

