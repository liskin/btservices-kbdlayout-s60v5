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
* Description:  Implementation of Connected state.
*  Version     : %version: 20 %
*
*/


// INCLUDE FILES
#include <e32property.h>
#include <ctsydomainpskeys.h>
#include "basrvaccstateattached.h"
#include "basrvaccstateattach.h"
#include "basrvaccstatedetach.h"
#include "basrvaccstatedisconnect.h"
#include "BTAccInfo.h"
#include "debug.h"

const TInt KRequestIdOpenMonoAudio = 40;
const TInt KRequestIdOpenStereoAudio = 41;
const TInt KRequestIdCloseMonoAudio = 42;
const TInt KRequestIdCloseStereoAudio = 43;
const TInt KRequestIdTimer = 44;
const TInt KRequestIdMonoActiveModeRequest = 45;
const TInt KRequestIdStereoActiveModeRequest = 46;
const TInt KRequestIdConnectRemConTG = 47;

const TInt KAudioCloseResponseDelay = 1200000; // 1.2 sec

// ================= MEMBER FUNCTIONS =======================

CBasrvAccStateAttached* CBasrvAccStateAttached::NewL(CBasrvAcc& aParent, TBool aShowNote)
    {
    CBasrvAccStateAttached* self=new(ELeave) CBasrvAccStateAttached(aParent, aShowNote);
    return self;
    }

CBasrvAccStateAttached::~CBasrvAccStateAttached()
    {
    delete iRemConTGConnector;
    delete iAudioOpener;
    delete iAudioCloser;
    delete iTimerActive;
    iTimer.Close();
    TRACE_FUNC
    }

void CBasrvAccStateAttached::EnterL()
    {
    StatePrint(_L("Attached"));
    iTimer.CreateLocal();
    if (AccInfo().iAudioOpenedProfiles)
        {
        if (AccInfo().iAudioOpenedProfiles & EStereo)
            HandleAccOpenedAudio(EStereo);
        if (AccInfo().iAudioOpenedProfiles & EHFP)
            HandleAccOpenedAudio(EHFP);
        if (AccInfo().iAudioOpenedProfiles & EHSP)
            HandleAccOpenedAudio(EHSP);
        }
    else
        {
        Parent().RequestSniffMode();
        }
    if (iShowNote)
        {
        Parent().AccMan().NotifyClientNewProfile(AccInfo().iConnProfiles, AccInfo().iAddr);
        TInt callState;
        TInt err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);
        if(!err && (callState == EPSCTsyCallStateNone || callState == EPSCTsyCallStateUninitialized))
            {
		    Parent().AccMan().ShowNote(EBTConnected, AccInfo().iAddr); 				  		
            }
        }
    }

CBasrvAccState* CBasrvAccStateAttached::ErrorOnEntry(TInt /*aReason*/)
    {
    TRACE_FUNC
    CBasrvAccState* next = NULL;
    TRAP_IGNORE(next = CBasrvAccStateDetach::NewL(Parent()));
    return next;
    }

TBTEngConnectionStatus CBasrvAccStateAttached::ConnectionStatus() const
    {
    return EBTEngConnected;
    }

void CBasrvAccStateAttached::ConnectL(const TBTDevAddr& aAddr)
    {
    if (aAddr == AccInfo().iAddr)
        {
        Parent().AccMan().ConnectCompletedL(AccInfo().iAddr, KErrNone, AccInfo().iConnProfiles);
        }
    else
        {
        CBasrvAccState::ConnectL(aAddr);
        }
    }

void CBasrvAccStateAttached::DisconnectL()
    {
    TRACE_FUNC
    Parent().AccMan().RemoveAudioRequest(AccInfo().iAddr);
        
    if (AccInfo().iAudioOpenedProfiles)
        {
        TAccAudioType type = (AccInfo().iAudioOpenedProfiles & EAnyMonoAudioProfiles) 
                ? EAccMonoAudio : EAccStereoAudio;    
        Parent().AccMan().AccfwConnectionL()->NotifyAudioLinkCloseL(AccInfo().iAddr, type);
        AccInfo().iAudioOpenedProfiles = EUnknownProfile;
        }
    
    Parent().ChangeStateL(CBasrvAccStateDetach::NewL(Parent()));
    }

void CBasrvAccStateAttached::AccessoryConnected(TProfiles aProfile)
    {
    TRACE_FUNC
    CBasrvAccState::AccessoryConnected(aProfile);
    Parent().NotifyLinkChange2Rvc();
    Parent().AccMan().NotifyClientNewProfile(aProfile, AccInfo().iAddr);
    }

void CBasrvAccStateAttached::AccOpenedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    delete iTimerActive;
    iTimerActive = NULL;    
    HandleAccOpenedAudio(aProfile);
    }

void CBasrvAccStateAttached::AccClosedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    UpdateAudioState(EAudioLinkClosed, (aProfile == EStereo) ? EAccStereoAudio : EAccMonoAudio);
    CBasrvAccState::AccClosedAudio(aProfile);
    Parent().NotifyLinkChange2Rvc();
    iCloseAudioProfile = aProfile;
    if (!iTimerActive)
        {
        iTimerActive = CBasrvActive::New(*this, CActive::EPriorityStandard, KRequestIdTimer);
        }
    if (iTimerActive)
        {
        iTimerActive->Cancel();
        iTimer.After(iTimerActive->iStatus, KAudioCloseResponseDelay);
        iTimerActive->GoActive();
        }
    Parent().RequestSniffMode();
    StatePrint(_L("Attached"));    
    }

void CBasrvAccStateAttached::AccessoryDisconnectedL(TProfiles aProfile)
    {
    TRACE_FUNC
    delete iTimerActive;
    iTimerActive = NULL;
    TInt audiolinkcache = AccInfo().iAudioOpenedProfiles;
    if ((AccInfo().iAudioOpenedProfiles & EStereo) && (aProfile & EStereo))
        {
        Parent().RequestSniffMode();
        }
    ProfileDisconnected(aProfile);
    CBasrvAccState::AccClosedAudio(aProfile);
    Parent().NotifyLinkChange2Rvc();
    Parent().AccMan().NotifyClientNoProfile(aProfile, AccInfo().iAddr);
    StatePrint(_L("Attached"));

    // AudioPolicy wants to get a notification about audio link closes before
    // detaches as well so here we go
    if (audiolinkcache & aProfile ||
        iCloseAudioProfile & aProfile )
        {
        TAccAudioType type = (aProfile & EAnyMonoAudioProfiles) 
            ? EAccMonoAudio : EAccStereoAudio;
        Parent().AccMan().AccfwConnectionL()->NotifyAudioLinkCloseL(AccInfo().iAddr, type);    
        
        // let's clear this variable here to be sure it's not used twice accidentally
        iCloseAudioProfile = EUnknownProfile;
        }

    if (!(AccInfo().iConnProfiles & EAnyAudioProfiles))
        {
        Parent().AccMan().RemoveAudioRequest(AccInfo().iAddr);
        Parent().ChangeStateL(CBasrvAccStateDetach::NewL(Parent()));
        }

    }

void CBasrvAccStateAttached::OpenAudioL(TAccAudioType aType)
    {
    TRACE_FUNC
    TInt requestId = (aType == EAccMonoAudio) 
        ? KRequestIdMonoActiveModeRequest : KRequestIdStereoActiveModeRequest;
    if (!iAudioOpener)
        {
        iAudioOpener = CBasrvActive::NewL(*this, CActive::EPriorityLow, requestId);
        }
    else
        {
        if (iAudioOpener->IsActive())
            {
            LEAVE(KErrAlreadyExists);
            }
        iAudioOpener->SetRequestId(requestId);
        }
    Parent().RequestActiveMode();

	// here we'll give time to the scheduler so that the active mode request has 
	// a chance to get through before the audio link open request - running
	// with EPriorityLow for that
	TRequestStatus* myStatus( &iAudioOpener->iStatus );
    *myStatus = KRequestPending;
    iAudioOpener->GoActive();
	User::RequestComplete( myStatus, KErrNone );
    }

void CBasrvAccStateAttached::CloseAudioL(TAccAudioType aType)
    {
    TRACE_FUNC

    DoCloseAudioL(aType);
    }

void CBasrvAccStateAttached::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_FUNC
    TInt status = aActive.iStatus.Int();
    TInt requestid = aActive.RequestId(); 
    switch (requestid)
        {
        case KRequestIdOpenMonoAudio:
            {
            if (!status)
                {
                if (AccInfo().iSuppProfiles & EHFP)
                    NewProfileConnection(EHFP);
                else
                    NewProfileConnection(EHSP);
                }
            TInt err = Parent().AccMan().OpenAudioCompleted(AccInfo().iAddr, EAccMonoAudio, status);
            if ( (err && err != KErrAlreadyExists) || (status && status != KErrInUse && status != KErrAlreadyExists))
                {
                if (Parent().AccMan().DisconnectIfAudioOpenFails())
                    {
                    Parent().AccMan().RemoveAudioRequest(AccInfo().iAddr);
                    Parent().ChangeStateL(CBasrvAccStateDetach::NewL(Parent()));
                    }
                }
            else
                {
                UpdateAudioState(EAudioLinkOpen, EAccMonoAudio);
                Parent().CancelPowerModeControl();
                AccInfo().iAudioOpenedProfiles |= (AccInfo().iConnProfiles & EAnyMonoAudioProfiles);
                Parent().NotifyLinkChange2Rvc();
                }
            break;
            }
        case KRequestIdCloseMonoAudio:
            {
            UpdateAudioState(EAudioLinkClosed, EAccMonoAudio);
            Parent().RequestSniffMode();
            (void) Parent().AccMan().CloseAudioCompleted(AccInfo().iAddr, EAccMonoAudio, status);
            break;
            }
        case KRequestIdOpenStereoAudio:
            {
            if (!status)
                {
                NewProfileConnection(EStereo);              
                }
            TInt err = Parent().AccMan().OpenAudioCompleted(AccInfo().iAddr, EAccStereoAudio, status);
            if ((status && status != KErrInUse && status != KErrAlreadyExists))
                {
                if (status == KErrDisconnected)
                    {
                    ProfileDisconnected(EStereo);
                    }
                if (Parent().AccMan().DisconnectIfAudioOpenFails())
                    {
                    Parent().AccMan().RemoveAudioRequest(AccInfo().iAddr);
                    Parent().ChangeStateL(CBasrvAccStateDetach::NewL(Parent()));
                    }
                }
            else if (!err)
                {
                UpdateAudioState(EAudioLinkOpen, EAccStereoAudio);
                Parent().PreventLowPowerMode();
                AccInfo().iAudioOpenedProfiles |= EStereo;
                Parent().NotifyLinkChange2Rvc();
                DoConnectRemConCtIfNeededL();
                }
            break;
            }
        case KRequestIdCloseStereoAudio:
            {
            UpdateAudioState(EAudioLinkClosed, EAccStereoAudio);
            Parent().RequestSniffMode();
            (void) Parent().AccMan().CloseAudioCompleted(AccInfo().iAddr, EAccStereoAudio, status);
            break;
            }
        case KRequestIdTimer:
            {
            TAccAudioType type = (iCloseAudioProfile & EAnyMonoAudioProfiles) 
                ? EAccMonoAudio : EAccStereoAudio;
            Parent().AccMan().AccfwConnectionL()->NotifyAudioLinkCloseL(AccInfo().iAddr, type);    

            // this variable needs to be cleared here so that the AccFW isn't notified a second time
            // about the same audio link close during disconnection process
            iCloseAudioProfile = EUnknownProfile;
            break;
            }
        case KRequestIdMonoActiveModeRequest:
        case KRequestIdStereoActiveModeRequest:
            {
            // the active mode request should have been scheduled at this point and we'll continue
            // with audio link open request - going back to standard priority
    		TProfiles profile = ( requestid == KRequestIdMonoActiveModeRequest ) ? EHFP : EStereo;            
		    Parent().AccMan().PluginMan().Plugin(profile)->OpenAudioLink(AccInfo().iAddr, 
		        iAudioOpener->iStatus);

    		TInt requestId = ( requestid == KRequestIdMonoActiveModeRequest ) 
    							? KRequestIdOpenMonoAudio : KRequestIdOpenStereoAudio;
		    iAudioOpener->SetPriority( CActive::EPriorityStandard );
		    iAudioOpener->SetRequestId( requestId );
		    iAudioOpener->GoActive();
            break;
            }
        case KRequestIdConnectRemConTG:
            {
            if (!status || status == KErrAlreadyExists)
                {
                NewProfileConnection(ERemConTG);
                Parent().NotifyLinkChange2Rvc();
                }
            delete iRemConTGConnector;
            iRemConTGConnector = NULL;
            break;
            }
        default:
            {
            }
        }
    }
    
void CBasrvAccStateAttached::CancelRequest(CBasrvActive& aActive)
    {
    TRACE_FUNC
    TInt request = aActive.RequestId();
    TAccAudioType type = (request == KRequestIdOpenMonoAudio || request == KRequestIdCloseMonoAudio ) ?
        EAccMonoAudio : EAccStereoAudio;
    TProfiles profile = (type == EAccMonoAudio) ? EAnyMonoAudioProfiles : EStereo;
    CBTAccPlugin* plugin = Parent().AccMan().PluginMan().Plugin(profile);
    if (request == KRequestIdOpenMonoAudio || request == KRequestIdOpenStereoAudio)
        {
        plugin->CancelOpenAudioLink(AccInfo().iAddr);
        Parent().AccMan().OpenAudioCompleted(AccInfo().iAddr, type, KErrCancel);
        }
    else if (request == KRequestIdCloseMonoAudio || request == KRequestIdCloseStereoAudio)
        {
        plugin->CancelCloseAudioLink(AccInfo().iAddr);
        Parent().AccMan().CloseAudioCompleted(AccInfo().iAddr, type, KErrCancel);
        }
    else if (request == KRequestIdTimer)
        {
        iTimer.Cancel();
        }
    else if (request == KRequestIdConnectRemConTG)
        {
        Parent().AccMan().PluginMan().Plugin(ERemConTG)->CancelConnectToAccessory(AccInfo().iAddr);
        }
    }

CBasrvAccStateAttached::CBasrvAccStateAttached(CBasrvAcc& aParent, TBool aShowNote)
    : CBasrvAccState(aParent, NULL), iShowNote(aShowNote)
    {
    }

void CBasrvAccStateAttached::HandleAccOpenedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    TAccAudioType type = (aProfile & EAnyMonoAudioProfiles) ? EAccMonoAudio : EAccStereoAudio;
    TInt latency = Parent().AccMan().PluginMan().AudioLinkLatency(); 

    TInt ret = Parent().AccMan().NotifyAccFwAudioOpened(AccInfo().iAddr, type, latency);

	if (ret == KErrNone)
        {
        UpdateAudioState(EAudioLinkOpen, (aProfile == EStereo) ? EAccStereoAudio : EAccMonoAudio);
        
        CBasrvAccState::AccOpenedAudio(aProfile);
        Parent().NotifyLinkChange2Rvc();
        if (type == EAccStereoAudio)
            {
            Parent().RequestActiveMode();
            Parent().PreventLowPowerMode();
            Parent().AccMan().PluginMan().Plugin(EStereo)->StartRecording();
            TRAP_IGNORE(DoConnectRemConCtIfNeededL());
            }
        else
            {
            Parent().CancelPowerModeControl();
            }
        }
	
    StatePrint(_L("Attached"));    
    }
    
void CBasrvAccStateAttached::DoCloseAudioL(TAccAudioType aType)
    {
    TRACE_FUNC
    TAccAudioType type = (iCloseAudioProfile & EAnyMonoAudioProfiles) ? EAccMonoAudio : EAccStereoAudio;
    if ( type == aType && iTimerActive  && iTimerActive->IsActive())
        {
        TRACE_STATE(_L("[BTAccStateAttached] Timer is cancelled"))   
        iTimerActive->Cancel();
        Parent().AccMan().CloseAudioCompleted(AccInfo().iAddr, type, KErrNone);
        return;
        }    

    TInt requestId = (aType == EAccMonoAudio) ? KRequestIdCloseMonoAudio : KRequestIdCloseStereoAudio;
    if (!iAudioCloser)
        {
        iAudioCloser = CBasrvActive::NewL(*this, CActive::EPriorityStandard, requestId);
        }
    else
        {
        if (iAudioCloser->IsActive())
            {
            LEAVE(KErrAlreadyExists);
            }
        iAudioCloser->SetRequestId(requestId);
        }
    TProfiles profile = (aType == EAccMonoAudio) ? EAnyMonoAudioProfiles : EStereo;
    AccInfo().iAudioOpenedProfiles &= ~profile;
    Parent().NotifyLinkChange2Rvc();
    Parent().AccMan().PluginMan().Plugin(profile)->CloseAudioLink(AccInfo().iAddr, iAudioCloser->iStatus);
    iAudioCloser->GoActive();
    }

void CBasrvAccStateAttached::DoConnectRemConCtIfNeededL()
    {
    TRACE_FUNC
    if (Parent().AccMan().IsAvrcpVolCTSupported() &&
        IsAvrcpTGCat2SupportedByRemote() &&
        !(AccInfo().iConnProfiles & ERemConTG) &&
        !iRemConTGConnector)
        {
        iRemConTGConnector = CBasrvActive::NewL(*this, CActive::EPriorityStandard, KRequestIdConnectRemConTG);
        CBTAccPlugin* plugin = Parent().AccMan().PluginMan().Plugin(ERemConTG);
        TRACE_ASSERT(plugin, KErrNotFound)
        plugin->ConnectToAccessory(AccInfo().iAddr, iRemConTGConnector->iStatus);
        iRemConTGConnector->GoActive();
        }
    }

void CBasrvAccStateAttached::UpdateAudioState(TBTAudioLinkState aState, TAccAudioType aType)
	{
	TBTAudioLinkInfo info;
	info.iAddr = AccInfo().iAddr;
	info.iProfile = (aType == EAccMonoAudio) ? 0 : 1;
	info.iState = aState;
	TPckgBuf<TBTAudioLinkInfo> pkg(info);
	RProperty::Set(KPSUidBluetoothEnginePrivateCategory, KBTAudioLinkStatus, pkg);
	}	
