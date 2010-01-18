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
* Description:  Implementation of Query state.
*
*/


// INCLUDE FILES
#include <AccPolGenericIdDefinitions.h>     //device type definitions
#include "basrvaccstatequery.h"
#include "basrvaccstateconnect.h"
#include "basrvaccstatedisconnect.h"
#include "basrvaccstateattach.h"
#include "basrvaccstateattached.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

CBasrvAccStateQuery* CBasrvAccStateQuery::NewL(CBasrvAcc& aParent, TBool aConnectingRequest)
    {
    CBasrvAccStateQuery* self=new(ELeave) CBasrvAccStateQuery(aParent, aConnectingRequest);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBasrvAccStateQuery::~CBasrvAccStateQuery()
    {
    delete iQuery;
    TRACE_FUNC
    }

void CBasrvAccStateQuery::EnterL()
    {
    StatePrint(_L("Query"));
    // (NOT iConnecting) -> Headset connects to phone, SDP failure is tolerated. 
    iQuery->QueryAccInfoL(AccInfo().iAddr, !iConnecting);    
    Parent().RequestActiveMode();
    Parent().PreventLowPowerMode();
    }

CBasrvAccState* CBasrvAccStateQuery::ErrorOnEntry(TInt aReason)
    {
    TRACE_FUNC
    if (iConnecting)
        {
        TRAP_IGNORE(Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, aReason, 0));
        }
    return NULL;
    }

TBTEngConnectionStatus CBasrvAccStateQuery::ConnectionStatus() const
    {
    return EBTEngConnecting;
    }

void CBasrvAccStateQuery::CancelConnect()
    {
    TRACE_FUNC
    if (iConnecting)
        {
        delete iQuery;
        iQuery = NULL;
        TRAP_IGNORE(Parent().ChangeStateL(NULL));
        }
    }

void CBasrvAccStateQuery::DisconnectL()
    {
    TRACE_FUNC
    delete iQuery;
    iQuery = NULL;
    if (AccInfo().iConnProfiles)
        {
        Parent().ChangeStateL(CBasrvAccStateDisconnect::NewL(Parent()));
        }
    else
        {
        Parent().ChangeStateL(NULL);
        }
    }
    
void CBasrvAccStateQuery::AccessoryDisconnectedL(TProfiles aProfile)
    {
    TRACE_FUNC
    ProfileDisconnected(aProfile);
    AccClosedAudio(aProfile);
    StatePrint(_L("Query"));
    if (!AccInfo().iConnProfiles && !iConnecting)
        {
        delete iQuery;
        iQuery = NULL;
        Parent().ChangeStateL(NULL);
        }
    }

void CBasrvAccStateQuery::GetAccInfoCompletedL(TInt aErr, const TAccInfo* aAcc, const TBTDeviceClass* aCod)
    {
    TRACE_FUNC
    CBasrvAccState* next = NULL;
    if ( ( aErr || !aAcc || !(aAcc->iSuppProfiles & EAnyAudioProfiles) ) &&
        ( iConnecting || !(AccInfo().iConnProfiles ) ))
        {
        TInt err = (aErr) ? aErr : KErrNotSupported;
        Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, err, 0);
        Parent().ChangeStateL(next);
        return;
        }

    if (aAcc)
        {
        TInt exist_conns = AccInfo().iConnProfiles;
        TInt exist_audios = AccInfo().iAudioOpenedProfiles;
        AccInfo() = *aAcc;
        AccInfo().iAudioOpenedProfiles = exist_audios;
        AccInfo().iConnProfiles = exist_conns;
        AccInfo().iSuppProfiles |= exist_conns;
        }
    
    // Take HFP supported feature from BTMAC if it is not available
    if ( ( AccInfo().iConnProfiles & EHFP ) && 
         AccInfo().iSupportedFeatures[TAccInfo::EHFPIndex] == 0 )
        {
        CBTAccPlugin* plugin = Parent().AccMan().PluginMan().Plugin(EHFP);;
        AccInfo().SetSupportedFeature((TUint16)plugin->GetRemoteSupportedFeature(), TAccInfo::EHFPIndex);
        }
    
    // if e.g. A2DP is supported by headset but BTSAC is not in ROM, filter it.
    Parent().AccMan().FilterProfileSupport(AccInfo());

    RArray<TBTDevAddr> conflicts;
    CleanupClosePushL(conflicts);
    ResolveConflicts(conflicts);
    if (conflicts.Count())
        {
        if (iConnecting)
            {
            Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, KErrAlreadyExists, 0, &conflicts);
            }
        if (AccInfo().iConnProfiles)
            {
            next = CBasrvAccStateDisconnect::NewL(Parent());
            }
        }
    else
        {
        // Guess if this is a carkit or headset using the CoD info
    	if (aCod && (aCod->MajorServiceClass() & EMajorServiceAudioService) &&
                    (aCod->MajorDeviceClass() & EMajorDeviceAudioDevice ))
        	{
            if (aCod->MinorDeviceClass() == EMinorDeviceAVHandsfree ||
                aCod->MinorDeviceClass() == EMinorDeviceAVCarAudio)
                {
                AccInfo().SetDeviceType(KDTCarKit);
                TRACE_INFO((_L("Device is a carkit.")));
                }
            else
                {
                AccInfo().SetDeviceType(KDTHeadset);
                TRACE_INFO((_L("Device is a headset.")));
                }
        	}
    	else
    		{
    		AccInfo().SetDeviceType(KDTHeadset);
            TRACE_INFO((_L("Device is a headset.")));
    		}
        
        if (iConnecting)
            {
            next = CBasrvAccStateConnect::NewL(Parent());
            }
        else if (AccInfo().iConnProfiles)
            {
            next = CBasrvAccStateAttach::NewL(Parent(), iConnecting);
            }
        }
    CleanupStack::PopAndDestroy(&conflicts);
    Parent().ChangeStateL(next);    
    }

CBasrvAccStateQuery::CBasrvAccStateQuery(CBasrvAcc& aParent, TBool aConnectingRequest)
    : CBasrvAccState(aParent, NULL), iConnecting(aConnectingRequest)
    {
    }

void CBasrvAccStateQuery::ConstructL()
    {
    iQuery = CBasrvSdpQuery::NewL(*this);    
    }
    
void CBasrvAccStateQuery::ResolveConflicts(RArray<TBTDevAddr>& aConflicts)
    {
    RPointerArray<const TAccInfo> infos;
    TInt err = Parent().AccMan().AccInfos(infos);
    
    if (err == KErrNone)
        {
        TInt supportedByThis = AccInfo().iSuppProfiles;
        
        // HFP and HSP are mutual exclusive, i.e. can only have one connected at a time.
        if (supportedByThis & EAnyMonoAudioProfiles)
            {
            supportedByThis |= EAnyMonoAudioProfiles;
            }
        
        for (TInt i = 0; i < infos.Count(); i++)
            {
            if ((infos[i]->iAddr != AccInfo().iAddr) &&
                (infos[i]->iSuppProfiles & supportedByThis))
                {
                aConflicts.Append(infos[i]->iAddr);
                }
            }
        
        infos.Close();
        }
    }
