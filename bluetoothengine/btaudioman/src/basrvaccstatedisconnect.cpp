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
* Description:  Implementation of Disconnect state.
*
*/


// INCLUDE FILES
#include "basrvaccstatedisconnect.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

CBasrvAccStateDisconnect* CBasrvAccStateDisconnect::NewL(CBasrvAcc& aParent, TInt aConnErr)
    {
    CBasrvAccStateDisconnect* self=new(ELeave) CBasrvAccStateDisconnect(aParent, aConnErr);
    return self;
    }

CBasrvAccStateDisconnect::~CBasrvAccStateDisconnect()
    {
    delete iActive;
    TRACE_FUNC
    }

void CBasrvAccStateDisconnect::EnterL()
    {
    StatePrint(_L("Disconnect"));
    iProfiles = AccInfo().iConnProfiles;
    Parent().RequestActiveMode();
    DoDisconnectL();
    }

CBasrvAccState* CBasrvAccStateDisconnect::ErrorOnEntry(TInt /*aReason*/)
    {
    TRACE_FUNC
    return NULL;
    }

TBTEngConnectionStatus CBasrvAccStateDisconnect::ConnectionStatus() const
    {
    if (iConnErr)
        return EBTEngConnecting;
    return EBTEngDisconnecting;
    }

void CBasrvAccStateDisconnect::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_INFO((_L("Disconnect 0x%02x done, err %d"), aActive.RequestId(), iDiscErr))
    if (!iDiscErr)
        iDiscErr = aActive.iStatus.Int();
    AccInfo().iConnProfiles &= ~(aActive.RequestId());
    StatePrint(_L("Disconnect"));
    if (!AccInfo().iConnProfiles)
        {
        if (iConnErr)
            Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, iConnErr, AccInfo().iSuppProfiles);
        else
            Parent().AccMan().DisconnectCompletedL(AccInfo().iAddr, iProfiles, KErrNone);
        Parent().ChangeStateL(NULL);
        }
    else
        {
        DoDisconnectL();
        }
    }
    
void CBasrvAccStateDisconnect::CancelRequest(CBasrvActive& /*aActive*/)
    {
    }

CBasrvAccStateDisconnect::CBasrvAccStateDisconnect(CBasrvAcc& aParent, TInt aConnErr)
    : CBasrvAccState(aParent, NULL), iConnErr(aConnErr)
    {
    }
 
void CBasrvAccStateDisconnect::DoDisconnectL()
    {
    TRACE_FUNC
    TProfiles profile = EUnknownProfile;
    CBTAccPlugin* plugin = NULL;
    if (AccInfo().iConnProfiles & EStereo)
        {
        profile = EStereo;
        }
    else if (AccInfo().iConnProfiles & EAnyRemConProfiles)
        {
        profile = EAnyRemConProfiles;
        }
    else if (AccInfo().iConnProfiles & EAnyMonoAudioProfiles )
        {
        profile = EAnyMonoAudioProfiles;
        }
    plugin = Parent().AccMan().PluginMan().Plugin(profile);
    if (plugin)
        {
        if (!iActive)
            {
            iActive = CBasrvActive::NewL(*this, CActive::EPriorityStandard, profile);
            }
        iActive->SetRequestId(profile);
        plugin->DisconnectAccessory(AccInfo().iAddr, iActive->iStatus);
        iActive->GoActive();
        TRACE_INFO((_L("Disconnect 0x%02x ..."), profile))
        }
    }

