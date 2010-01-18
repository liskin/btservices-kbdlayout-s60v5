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
* Description:  Implementation of BT Acc Server accessory base state.
*
*/


// INCLUDE FILES
#include "basrvaccstate.h"
#include "debug.h"

const TInt KBitAvrcpSupportedFeatureCategory2 = 0x2;


// ================= MEMBER FUNCTIONS =======================

CBasrvAccState::~CBasrvAccState()
    {
    CompleteStateRequest(KErrAbort);
    }
    
CBasrvAccState* CBasrvAccState::ErrorOnEntry(TInt /*aReason*/)
    {
    TRACE_FUNC
    return NULL;
    }

void CBasrvAccState::ConnectL(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
    LEAVE(KErrAlreadyExists);
    }

void CBasrvAccState::CancelConnect()
    {
    TRACE_FUNC
    }

void CBasrvAccState::DisconnectL()
    {
    TRACE_FUNC
    LEAVE(KErrDisconnected);
    }

void CBasrvAccState::AccessoryConnected(TProfiles aProfile)
    {
    TRACE_FUNC
    NewProfileConnection(aProfile);    
    StatePrint(_L("StateXXX"));
    }

void CBasrvAccState::AccOpenedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    AccInfo().iAudioOpenedProfiles |= aProfile;
    StatePrint(_L("StateXXX"));
    }

void CBasrvAccState::AccClosedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    if (aProfile & EAnyMonoAudioProfiles)
        AccInfo().iAudioOpenedProfiles &= ~EAnyMonoAudioProfiles;
    else
        AccInfo().iAudioOpenedProfiles &= ~aProfile;
    StatePrint(_L("StateXXX"));
    }
    
void CBasrvAccState::AccessoryDisconnectedL(TProfiles /*aProfile*/)
    {
    TRACE_FUNC
    }

void CBasrvAccState::OpenAudioL(TAccAudioType /*aType*/)
    {
    TRACE_FUNC
    }

void CBasrvAccState::CloseAudioL(TAccAudioType /*aType*/)
    {
    TRACE_FUNC
    }

void CBasrvAccState::RequestCompletedL(CBasrvActive& /*aActive*/)
    {
    TRACE_FUNC
    }
    
void CBasrvAccState::CancelRequest(CBasrvActive& /*aActive*/)
    {
    TRACE_FUNC
    }

CBasrvAccState::CBasrvAccState(CBasrvAcc& aParent, TRequestStatus* aRequest)
    : iParent(aParent), iRequest(aRequest)
    {
    }

CBasrvAcc& CBasrvAccState::Parent()
    {
    return iParent;
    }

void CBasrvAccState::CompleteStateRequest(TInt aErr)
    {
    if (iRequest)
        {
        User::RequestComplete(iRequest, aErr);
        TRACE_INFO((_L("Request 0x%08X completed with %d"), iRequest, aErr))
        iRequest = NULL;
        }
    }
    
void CBasrvAccState::CompleteRequest(TRequestStatus* aStatus, TInt aErr)
    {
    if (aStatus)
        {
        User::RequestComplete(aStatus, aErr);
        TRACE_INFO((_L("Request 0x%08X completed with %d"), aStatus, aErr))
        }
    }

void CBasrvAccState::StatePrint(const TDesC& aStateName)
    {
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        AccInfo().iAddr.GetReadable(buf);
        Trace(_L("[%S State %S] Profiles %04X, Connected %04X, withAudio %04X"),
            &aStateName, &buf, AccInfo().iSuppProfiles, AccInfo().iConnProfiles, AccInfo().iAudioOpenedProfiles);
        }); (void) aStateName;
    }

TAccInfo& CBasrvAccState::AccInfo()
    {
    return Parent().AccInfo();
    }

void CBasrvAccState::NewProfileConnection(TProfiles aProfile)
    {
    AccInfo().iConnProfiles |= aProfile;
    AccInfo().iSuppProfiles |= aProfile;
    if (aProfile == ERemConCT && 
        Parent().AccMan().IsAvrcpVolCTSupported()&& 
        IsAvrcpTGCat2SupportedByRemote())
        {
        AccInfo().iConnProfiles |= ERemConTG;
        }
    TRACE_INFO((_L("Profiles %04X, Connected %04X"), AccInfo().iSuppProfiles, AccInfo().iConnProfiles))
    }

void CBasrvAccState::ProfileDisconnected(TProfiles aProfile)
    {
    if (aProfile & EAnyRemConProfiles)
        {
        AccInfo().iConnProfiles &= ~EAnyRemConProfiles;
        }
    else if (aProfile & EAnyMonoAudioProfiles)
        {
        AccInfo().iConnProfiles &= ~EAnyMonoAudioProfiles;
        }
    else
        {
        AccInfo().iConnProfiles &= ~aProfile;
        }
    }

TBool CBasrvAccState::IsAvrcpTGCat2SupportedByRemote()
    {
    return (AccInfo().iSuppProfiles & ERemConTG) && 
        (AccInfo().iSupportedFeatures[TAccInfo::ERemConTGIndex] & KBitAvrcpSupportedFeatureCategory2);
    }

    
//  End of File  
