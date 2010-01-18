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
#include <e32property.h>
#include <ctsydomainpskeys.h>
#include "basrvaccstatedetach.h"
#include "basrvaccstatedisconnect.h"
#include "BTAccFwIf.h"
#include "debug.h"

const TInt KDetachRequestId = 10;

// ================= MEMBER FUNCTIONS =======================

CBasrvAccStateDetach* CBasrvAccStateDetach::NewL(CBasrvAcc& aParent)
    {
    CBasrvAccStateDetach* self=new(ELeave) CBasrvAccStateDetach(aParent);
    return self;
    }

CBasrvAccStateDetach::~CBasrvAccStateDetach()
    {
    delete iActive;
    delete iAccfw;
    TRACE_FUNC
    }

void CBasrvAccStateDetach::EnterL()
    {
    StatePrint(_L("Detach"));
    // Stop any volume control
    AccInfo().iAudioOpenedProfiles = EUnknownProfile;
    Parent().NotifyLinkChange2Rvc();
    iAccfw = Parent().AccMan().NewAccfwConnectionInstanceL();
    iActive = CBasrvActive::NewL(*this, CActive::EPriorityStandard, KDetachRequestId);
    iAccfw->DetatchAccessory(AccInfo().iAddr, iActive->iStatus);
    iActive->GoActive();
    }

CBasrvAccState* CBasrvAccStateDetach::ErrorOnEntry(TInt aReason)
    {
    TRACE_FUNC
    TRAP_IGNORE(Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, aReason, AccInfo().iSuppProfiles));
    CBasrvAccState* next = NULL;
    TRAP_IGNORE(next = CBasrvAccStateDisconnect::NewL(Parent()));
    return next;
    }

TBTEngConnectionStatus CBasrvAccStateDetach::ConnectionStatus() const
    {
    return EBTEngDisconnecting;
    }
    
void CBasrvAccStateDetach::AccessoryDisconnectedL(TProfiles aProfile)
    {
    TRACE_FUNC
    ProfileDisconnected(aProfile);
    StatePrint(_L("Detach"));    
    }

void CBasrvAccStateDetach::OpenAudioL(TAccAudioType /*aType*/)
    {
    TRACE_FUNC
    LEAVE(KErrDisconnected);
    }

void CBasrvAccStateDetach::CloseAudioL(TAccAudioType /*aType*/)
    {
    TRACE_FUNC
    LEAVE(KErrDisconnected);    
    }

void CBasrvAccStateDetach::RequestCompletedL(CBasrvActive& /*aActive*/)
    {
    TRACE_FUNC
    Parent().AccMan().PluginMan().AccOutOfUse();
    TInt callState;
    TInt err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
    if(!err && (callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized))
        {
	    Parent().AccMan().ShowNote(EBTClosed, AccInfo().iAddr); 				  		
        }
    
    if (AccInfo().iConnProfiles)
        {
        Parent().ChangeStateL(CBasrvAccStateDisconnect::NewL(Parent()));
        }
    else
        {
        Parent().ChangeStateL(NULL);
        }
    }
    
void CBasrvAccStateDetach::CancelRequest(CBasrvActive& /*aActive*/)
    {
    }

CBasrvAccStateDetach::CBasrvAccStateDetach(CBasrvAcc& aParent)
    : CBasrvAccState(aParent, NULL)
    {
    }
 
