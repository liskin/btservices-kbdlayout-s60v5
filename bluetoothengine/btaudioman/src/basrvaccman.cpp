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
* Description:  Implementation of an accessory management.
*  Version     : %version:  14.1.8 %
*
*/


// INCLUDE FILES
#include <centralrepository.h>
#include "basrvaccman.h"
#include "BTAccSession.h"
#include "BTAccClientSrv.h"
#include "basrvacc.h"
#include "BTAccInfo.h"
#include "basrvpluginman.h"
#include "BTAccFwIf.h"
#include "debug.h"

const TInt KOpenAudioListenerId = 20;
const TInt KCloseAudioListenerId = 21;
const TInt KShowNoteRequest = 22;
const TInt KAsyncHandlingRequest = 23;

// ================= MEMBER FUNCTIONS =======================

CBasrvAccMan* CBasrvAccMan::NewL()
    {
    CBasrvAccMan* self = new (ELeave) CBasrvAccMan();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBasrvAccMan::~CBasrvAccMan()
    {
    delete iOpenListener;
    delete iCloseListener;
    delete iAsyncHandlingActive;
    
    // Response to any buffered audio request from Audio Policy.
    if (iAccfw)
        {
        TInt count = iAudioRequests.Count();
        for (TInt i = 0;  i < count; i++)
            {
            if (iAudioRequests[i].iReqType == EOpenReqFromAudioPolicy)
                {
                TRAP_IGNORE(iAccfw->AudioConnectionOpenCompleteL(
                    iAudioRequests[i].iAddr, KErrDisconnected));
                }
            else if (iAudioRequests[i].iReqType == ECloseReqFromAudioPolicy)
                {
                TRAP_IGNORE(iAccfw->AudioConnectionCloseCompleteL(
                    iAudioRequests[i].iAddr, KErrDisconnected));
                }
            }
        }
    
    iAudioRequests.Close();
    iAccs.ResetAndDestroy();
    iAccs.Close();
    delete iAccfw;
    delete iPluginMan;
    iNotifierArray.ResetAndDestroy();
    iNotifierArray.Close();
    iClientRequests.Close();
    DestructVariant();
    TRACE_FUNC    
    }

void CBasrvAccMan::LoadServicesL()
    {
    TRACE_FUNC    
    if (!iPluginMan)
        {
        iPluginMan = CBasrvPluginMan::NewL();
        }
    iPluginMan->LoadPluginsL(*this);
    }
    
void CBasrvAccMan::ConnectL(CBTAccSession& aSession, const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TClientRequestCache request;
    request.iSession = &aSession;
    request.iRequest = EBTAccSrvConnectToAccessory;
    request.iAddr = aAddr;
    iClientRequests.AppendL(request);
    
    //remove the last item from the RArray if a leave occurs later
    TCleanupItem cleanupItem(CleanupLastItemFromClientRequestsArray, &iClientRequests);
    CleanupStack::PushL(cleanupItem);
    
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        iAccs[idx]->ConnectL(aAddr);
        }
    else
        {
        CBasrvAcc* acc = CBasrvAcc::NewLC(*this);
        iAccs.AppendL(acc);
        CleanupStack::Pop(acc);
        acc->ConnectL(aAddr);
        }
    
    CleanupStack::Pop(&iClientRequests);
    }

void CBasrvAccMan::CancelConnect(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
	    TInt count = iClientRequests.Count();
	    for (TInt i = 0; i < count; i++)
	        {
	        if (iClientRequests[i].iAddr == aAddr && 
	            iClientRequests[i].iRequest == EBTAccSrvConnectToAccessory)
	            {
	            iClientRequests.Remove(i);
	            break;
	            }
	        }        
        iAccs[idx]->CancelConnect();
        }
    }

void CBasrvAccMan::DisconnectL(CBTAccSession& aSession, const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        RemoveAudioRequest(aAddr);
        TClientRequestCache request;
        request.iSession = &aSession;
        request.iRequest = EBTAccSrvDisconnectAccessory;
        request.iAddr = aAddr;
        iClientRequests.AppendL(request);
        iAccs[idx]->DisconnectL();
        }
    else
        {
        aSession.DisconnectCompleted(EAnyMonoAudioProfiles, KErrNotFound);
        }
    }

void CBasrvAccMan::DisconnectAllL(CBTAccSession& aSession)
    {
    TRACE_FUNC
    if (!iDisconnectAllPending)
        {
        TInt count = iAccs.Count();
        if (!count)
            {
            aSession.DisconnectAllCompleted(KErrNone);
            }
        else
            {
            iSessionOfDisconnectAll = &aSession;
            for (TInt i = 0; i < count; i++)
                {
                RemoveAudioRequest(iAccs[i]->Remote());
                iAccs[i]->DisconnectL();
                }
            iDisconnectAllPending = ETrue;
            }
        }
    }

void CBasrvAccMan::SetProfileNotifySession(CBTAccSession& aSession)
    {
    iProfileNotifySession = &aSession;
    }

void CBasrvAccMan::ClearProfileNotifySession(CBTAccSession& aSession)
    {
    //if the session passed in is equal to what we have stored
    if (&aSession == iProfileNotifySession)
        {
        //invalidate the pointer to the session
        iProfileNotifySession = NULL;
        }
    }

const TAccInfo* CBasrvAccMan::AccInfo(const TBTDevAddr& aAddr)
    {
    TInt count = iAccs.Count();
    const TAccInfo* info = NULL;
    for (TInt i = 0; i < count; i++)
        {
        info = iAccs[i]->AccInfo(aAddr);
        if (info)
            {
            return info;
            }
        }
    return NULL;
    }

CBasrvPluginMan& CBasrvAccMan::PluginMan()
    {
    return *iPluginMan;
    }
    
void CBasrvAccMan::ConnectCompletedL(const TBTDevAddr& aAddr, TInt aErr, 
    TInt aProfile, const RArray<TBTDevAddr>* aConflicts)
    {
    TRACE_FUNC
    TInt count = iClientRequests.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iClientRequests[i].iAddr == aAddr && 
            iClientRequests[i].iRequest == EBTAccSrvConnectToAccessory)
            {
            iClientRequests[i].iSession->ConnectCompleted(aErr, aProfile, aConflicts);
            iClientRequests.Remove(i);
            break;
            }
        }
    }
    
void CBasrvAccMan::DisconnectCompletedL(const TBTDevAddr& aAddr, TInt aProfile, TInt aErr)
    {
    TRACE_FUNC
    TInt count = iClientRequests.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iClientRequests[i].iAddr == aAddr && 
            iClientRequests[i].iRequest == EBTAccSrvDisconnectAccessory)
            {
            iClientRequests[i].iSession->DisconnectCompleted(aProfile, aErr);
            iClientRequests.Remove(i);
            break;
            }
        }
    }

#define TRACE_AUDIO_REQUEST_COMPLETE \
    TRACE_INFO_SEG(\
        {\
        TBuf<12> buf;\
        aAddr.GetReadable(buf);\
        Trace(_L("[Audio Request Complete] %d %d at '%S', err %d"), \
            iAudioRequests[0].iReqType, aType, &buf, aErr);\
        });

TInt CBasrvAccMan::OpenAudioCompleted(const TBTDevAddr& aAddr, TAccAudioType aType, TInt aErr)
    {
    TInt err = KErrNone;
    if (iAudioRequests.Count())
        {
        TRACE_AUDIO_REQUEST_COMPLETE
        TRequestType reqType = iAudioRequests[0].iReqType;
        iAudioRequests.Remove(0);
        TInt latency = iPluginMan->AudioLinkLatency();
        if (reqType == EOpenReqFromAudioPolicy)
            {
            	
	        TRAP(err, AccfwConnectionL()->AudioConnectionOpenCompleteL(aAddr, aErr, latency))
	        TRACE_INFO((_L("AccfwConnectionL()->AudioConnectionOpenCompleteL trap %d"), err))
	        if (!err && !aErr)
	            {
	            err = NotifyAccFwAudioOpened(aAddr, aType, latency);
	            }
           else if (err == KErrAlreadyExists && !aErr)
                {
        	    RejectAudioLink(aAddr, aType);
                }
           }
        }
    if (!err) // if err, a rejecting audio link has been scheduled.
	    {
	    TRAP_IGNORE(DoAudioRequestL());
	    }
    return err;
    }
    
TInt CBasrvAccMan::CloseAudioCompleted(const TBTDevAddr& aAddr, TAccAudioType aType, TInt aErr)
    {
    TInt err = KErrNone;
    if (iAudioRequests.Count())
        {
        TRACE_AUDIO_REQUEST_COMPLETE
        if (iAudioRequests[0].iReqType == ECloseReqFromAudioPolicy)
            {
            TRAP(err, AccfwConnectionL()->NotifyAudioLinkCloseL(aAddr, aType));
            TRACE_INFO((_L("AccfwConnectionL()->NotifyAudioLinkCloseL trap %d"), err))
            TRAP(err, AccfwConnectionL()->AudioConnectionCloseCompleteL(aAddr, aErr));
            TRACE_INFO((_L("AccfwConnectionL()->AudioConnectionCloseCompleteL trap %d"), err))
            }
        iAudioRequests.Remove(0);
        }
    if (iAudioRequests.Count())
        {
        TRAP_IGNORE(DoAudioRequestL());
        }
    return err;
    }

void CBasrvAccMan::AccObsoleted(CBasrvAcc* aAcc, const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
    TInt idx = iAccs.Find(aAcc);
    if (idx >= 0)
        {
        RemoveAudioRequest(iAccs[idx]->Remote());
        delete aAcc;
        iAccs.Remove(idx);
        }
    if (!iAccs.Count())
        {
        delete iOpenListener;
        iOpenListener = NULL;
        delete iCloseListener;
        iCloseListener = NULL;
        delete iAccfw;
        iAccfw = NULL;
        iAudioRequests.Reset();
        if (iDisconnectAllPending)
            {
            iDisconnectAllPending = EFalse;
            if (iSessionOfDisconnectAll)
                iSessionOfDisconnectAll->DisconnectAllCompleted(KErrNone);
            }
        }
    }
    
CBasrvAccfwIf* CBasrvAccMan::AccfwConnectionL(const TAccInfo* aInfo)
    {
    if (!iAccfw)
        {
        iAccfw = CBasrvAccfwIf::NewL(aInfo);
        }
    return iAccfw;
    }

CBasrvAccfwIf* CBasrvAccMan::NewAccfwConnectionInstanceL()
    {
    TRACE_FUNC
    return CBasrvAccfwIf::NewL(NULL);
    }
    
void CBasrvAccMan::ListenAudioRequestL()
    {
    TRACE_FUNC
    if (!iOpenListener)
        {
        iCloseListener = CBasrvActive::NewL(*this, CActive::EPriorityStandard, KCloseAudioListenerId);
        iOpenListener = CBasrvActive::NewL(*this, CActive::EPriorityStandard, KOpenAudioListenerId);
        AccfwConnectionL()->NotifyBTAudioLinkOpenReq(iOpenAddr,iOpenListener->iStatus, iOpenType);
        iOpenListener->GoActive();
        AccfwConnectionL()->NotifyBTAudioLinkCloseReq(iCloseAddr,iCloseListener->iStatus, iCloseType);
        iCloseListener->GoActive();
        }
    }

void CBasrvAccMan::CancelAudioRequestListen()
    {
    delete iOpenListener;
    iOpenListener = NULL;
    delete iCloseListener;
    iCloseListener = NULL;
    TRACE_FUNC
    }

void CBasrvAccMan::RemoveAudioRequest(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TInt count = iAudioRequests.Count();
    TInt i = 0;
    while(count && i < count)
        {
        if (iAudioRequests[i].iAddr == aAddr)
            {
            TRACE_INFO_SEG(
                {
                TBuf<12> buf;
                aAddr.GetReadable(buf);
                Trace(_L("[Audio Request Aborted] %d %d at '%S'"), 
                    iAudioRequests[i].iReqType, iAudioRequests[i].iAudioType, &buf);
                });
            if (iAudioRequests[i].iReqType == EOpenReqFromAudioPolicy)
                {
                TRAP_IGNORE(AccfwConnectionL()->AudioConnectionOpenCompleteL(aAddr, KErrDisconnected));
                }
            else if (iAudioRequests[i].iReqType == ECloseReqFromAudioPolicy)
                {
                TRAP_IGNORE(AccfwConnectionL()->AudioConnectionCloseCompleteL(aAddr, KErrDisconnected));
                }
            iAudioRequests.Remove(i);
            count--;
            }
        else
            {
            i++;
            }
        }
    }

void CBasrvAccMan::ShowNote(TBTGenericInfoNoteType aNote, const TBTDevAddr& aAddr)
    {
    TRACE_INFO((_L("CBasrvAccMan::ShowNote %d"), aNote))
    TRAPD(err, iNotifierArray.Append(CBasrvActiveNotifier::NewL(*this, CActive::EPriorityStandard, KShowNoteRequest, aAddr, aNote)));
    if (err)
        {
        TRACE_ERROR((_L("could not construct active notifer object")))
        return;
        }
    }

void CBasrvAccMan::FilterProfileSupport(TAccInfo& aInfo)
    {
    TInt pluginbit = iPluginMan->AvailablePlugins();
    aInfo.iSuppProfiles &= pluginbit;
    }

TInt CBasrvAccMan::NotifyAccFwAudioOpened(const TBTDevAddr& aAddr, TAccAudioType aType, TInt /*aLatency*/)
    {
    TRAPD(err, AccfwConnectionL()->NotifyAudioLinkOpenL(aAddr, aType));
    TRACE_INFO((_L("AccfwConnectionL()->NotifyAudioLinkOpenL type %d trap %d"), aType, err))
    if (err)
        {
        RejectAudioLink(aAddr, aType);
        }
    return err;
    }

TInt CBasrvAccMan::AccInfos(RPointerArray<const TAccInfo>& aInfos)
    {
    TInt count = iAccs.Count();
    TInt err = KErrNone;
    
    for (TInt i = 0; i < count; i++)
        {
        err = aInfos.Append(&(iAccs[i]->AccInfo()));
        
        if (err != KErrNone)
            {
            //an error occured so cleanup
            aInfos.Close();
            break;
            }
        }
    
    return err;
    }

void CBasrvAccMan::NotifyClientNewProfile(TInt aProfile, const TBTDevAddr& aAddr)
    {
    if (iProfileNotifySession)
        {
        iProfileNotifySession->NotifyClientNewProfile(aProfile, aAddr);
        }
    }
    
void CBasrvAccMan::NotifyClientNoProfile(TInt aProfile, const TBTDevAddr& aAddr)
    {
    if (iProfileNotifySession)
        {
        iProfileNotifySession->NotifyClientNoProfile(aProfile, aAddr);
        }
    }

TBTEngConnectionStatus CBasrvAccMan::ConnectionStatus4Client(const TBTDevAddr& aAddr) const
    {
    TInt count = iAccs.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iAccs[i]->Remote() == aAddr)
            {
            return iAccs[i]->ConnectionStatus();
            }
        }
    return EBTEngNotConnected;
    }

TBool CBasrvAccMan::IsAvrcpVolCTSupported()
    {
    return iAvrcpVolCTSupported;
    }
    
TBool CBasrvAccMan::IsAbsoluteVolumeSupported(const TBTDevAddr& aAddr)
	{
    TInt count = iAccs.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iAccs[i]->Remote() == aAddr)
            {
            TBool ret = iAccs[i]->AccInfo().iAvrcpVersion == TAccInfo::EAvrcpVersion14 ? ETrue : EFalse; 
            return ret;
            }
        }
    return EFalse;
	}
    
TBool CBasrvAccMan::DisconnectIfAudioOpenFails()
    {
    return iDisconnectIfAudioOpenFails;
    }
    
void CBasrvAccMan::NewAccessory( const TBTDevAddr& aAddr, TProfiles aProfile )
    {
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        aAddr.GetReadable(buf);
        Trace(_L("[profile remote connect] profile %d, BTAddr '%S' "), aProfile, &buf );
        });
    
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        TRAP_IGNORE(iAccs[idx]->AccessoryConnectedL(aAddr, aProfile));
        }
    else
        {
        CBasrvAcc* acc = NULL;
        TRAP_IGNORE(acc = CBasrvAcc::NewL(*this));
        if (acc)
            {
            TInt err = iAccs.Append(acc);
            if (err)
                delete acc;
            else
                {
                TRAP_IGNORE(acc->AccessoryConnectedL(aAddr, aProfile));
                }
            }
        }
    }

void CBasrvAccMan::AccessoryDisconnected(const TBTDevAddr& aAddr, TProfiles aProfile)
    {
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        aAddr.GetReadable(buf);
        Trace(_L("[profile remote disconnect] profile %d, BTAddr '%S' "), aProfile, &buf );
        });
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        TRAP_IGNORE(iAccs[idx]->AccessoryDisconnectedL(aProfile));
        }
    }

void CBasrvAccMan::RemoteAudioOpened(const TBTDevAddr& aAddr, TProfiles aProfile)
    {
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        aAddr.GetReadable(buf);
        Trace(_L("[Audio remote open] profile %d, BTAddr '%S' "), aProfile, &buf );
        });

    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        // Check if another audio link opened already
        TInt audiolinks = AudioLinkStatus();
        
        if ( ( audiolinks && audiolinks != aProfile) || 
            ( iAudioRequests.Count() && 
              iAudioRequests[0].iOngoing && 
              iAudioRequests[0].iAddr == aAddr) )
            {
            // another audio type is opened while we have an audio link or pending audio request.
            if (iAudioRequests.Count())
                {
                TRACE_INFO((_L(" [audio link check] existing audio link %x, audio request pending ? %d. Audio should be rejected!"),
                        audiolinks, iAudioRequests[0].iOngoing))
                }
            else
                {
                TRACE_INFO((_L(" [audio link check] existing audio link %x. Audio should be rejected!"),
                        audiolinks))
                }
            RejectAudioLink(aAddr, (aProfile == EStereo) ? EAccStereoAudio : EAccMonoAudio);
            }
        else
            {
            iAccs[idx]->AccOpenedAudio(aProfile);
            }
        }
    }

void CBasrvAccMan::RemoteAudioClosed(const TBTDevAddr& aAddr, TProfiles aProfile)    
    {
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        aAddr.GetReadable(buf);
        Trace(_L("[Audio remote close] profile %d, BTAddr '%S' "), aProfile, &buf );
        });
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        iAccs[idx]->AccClosedAudio(aProfile);
        }
    }

TInt CBasrvAccMan::DisableNREC(const TBTDevAddr& aAddr)    
    {
    TRACE_FUNC
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
            TInt err = KErrNone;
            TRAPD(trap, err = AccfwConnectionL()->DisableNREC(aAddr));
            if (trap)
                return trap; 
            return err;
        }
    return KErrNotFound; 	    
    }

void CBasrvAccMan::AccessoryTemporarilyUnavailable(const TBTDevAddr& /*aAddr*/, TProfiles /*aType*/)
    {
    TRACE_INFO((_L("AccessoryTemporarilyUnavailable Deprecated !!!")))
    }

TInt CBasrvAccMan::ConnectionStatus(const TBTDevAddr& aAddr)
	{
	TRACE_FUNC
    TInt count = iAccs.Count();
    const TAccInfo* info = NULL;
    for (TInt i = 0; i < count; i++)
        {
        info = iAccs[i]->AccInfo(aAddr);
        if (info)
            {
            return info->iConnProfiles;
            }
        }
    return 0;
	}

TInt CBasrvAccMan::SupportedFeature( const TBTDevAddr& aAddr, TProfiles aProfile ) const
    {
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        aAddr.GetReadable(buf);
        Trace(_L("CBasrvAccMan::SupportedFeature '%S'  profile %d"), &buf, aProfile );
        });
    TInt idx = FindAcc(aAddr);
    if (idx >= 0)
        {
        return iAccs[idx]->SupportedFeature(aProfile );
        }
    return 0;
    }

void CBasrvAccMan::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_FUNC
    switch (aActive.RequestId())
        {
        case KOpenAudioListenerId:
            {
            if (aActive.iStatus == KErrNone)
                {
                TAudioRequest req;
                req.iAddr = iOpenAddr;
                req.iAudioType = iOpenType;
                req.iReqType = EOpenReqFromAudioPolicy;
                req.iOngoing = EFalse;
                
                TRACE_INFO_SEG(
                    {
                    TBuf<12> buf;
                    iOpenAddr.GetReadable(buf);
                    Trace(_L("[Audio Request Buf] 1(Open) %d at '%S'"), iOpenType, &buf);
                    });
                
                AccfwConnectionL()->NotifyBTAudioLinkOpenReq(iOpenAddr, aActive.iStatus, iOpenType);
                aActive.GoActive();
                iAudioRequests.AppendL(req);
                DoAudioRequestL();
                }
            break;
            }
        case KCloseAudioListenerId:
            if (aActive.iStatus == KErrNone)
                {
                TAudioRequest req;
                req.iAddr = iCloseAddr;
                req.iAudioType = iCloseType;
                req.iReqType = ECloseReqFromAudioPolicy;
                req.iOngoing = EFalse;
                TRACE_INFO_SEG(
                    {
                    TBuf<12> buf;
                    iCloseAddr.GetReadable(buf);
                    Trace(_L("[Audio Request Buf] 0(Close) %d at '%S'"), iCloseType, &buf);
                    });
                AccfwConnectionL()->NotifyBTAudioLinkCloseReq(iCloseAddr, aActive.iStatus, iCloseType);
                aActive.GoActive();
                iAudioRequests.AppendL(req);
                DoAudioRequestL();
                }
            break;
        case KShowNoteRequest:
            {
            TInt index = iNotifierArray.Find(&aActive);
            if(index>=0)
            	{
            	delete iNotifierArray[index];	
            	iNotifierArray.Remove(index);
            	}
            break;
            }
        case KAsyncHandlingRequest:
            {
            DoAudioRequestL();
            delete iAsyncHandlingActive;
            iAsyncHandlingActive = NULL;
            break;
            }
        default:
            {
            
            }
        }
    }
    
void CBasrvAccMan::CancelRequest(CBasrvActive& aActive)
    {
    TRACE_FUNC
    if (aActive.RequestId() == KOpenAudioListenerId)
        {
        TRAP_IGNORE(AccfwConnectionL()->CancelNotifyBTAudioLinkOpenReq());
        }
    else if (aActive.RequestId() == KCloseAudioListenerId)
        {
        TRAP_IGNORE(AccfwConnectionL()->CancelNotifyBTAudioLinkCloseReq());
        }
    }
    
CBasrvAccMan::CBasrvAccMan()
    {
    }
    
void CBasrvAccMan::DoAudioRequestL()
    {
    TRACE_FUNC
    if (iAudioRequests.Count() && !iAudioRequests[0].iOngoing && (!iAsyncHandlingActive || !iAsyncHandlingActive->IsActive()))
        {
        iAudioRequests[0].iOngoing = ETrue;
        TInt err = KErrNotFound;
        TInt idx = FindAcc(iAudioRequests[0].iAddr);
        if (idx >= 0)
            {
            if (iAudioRequests[0].iReqType == EOpenReqFromAudioPolicy)
                {
                TRAP(err, iAccs[idx]->OpenAudioL(iAudioRequests[0].iAudioType));
                }
            else
                {
                TRAP(err, iAccs[idx]->CloseAudioL(iAudioRequests[0].iAudioType));
                }
            TRACE_INFO_SEG({
                if ( iAudioRequests.Count() )
                    {
                    TBuf<12> buf;
                    iAudioRequests[0].iAddr.GetReadable(buf);
                    Trace(_L("[Audio Request Start] %d %d at '%S', trap %d"), 
                    iAudioRequests[0].iReqType, iAudioRequests[0].iAudioType, &buf, err);
                    }
                });
            }
        if (err)
            {
            if (iAudioRequests[0].iReqType == EOpenReqFromAudioPolicy)
                OpenAudioCompleted(iAudioRequests[0].iAddr, iAudioRequests[0].iAudioType, err);
            else if(iAudioRequests[0].iReqType == ECloseReqFromAudioPolicy)
                CloseAudioCompleted(iAudioRequests[0].iAddr, iAudioRequests[0].iAudioType, err);
            }
        }    
    }

TInt CBasrvAccMan::FindAcc(const TBTDevAddr& aRemote) const
    {
    TInt count = iAccs.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iAccs[i]->Remote() == aRemote)
            {
            return i;
            }
        }
    return KErrNotFound;
    }

TInt CBasrvAccMan::AudioLinkStatus()
    {
    TInt profiles = 0;
    TInt count = iAccs.Count();
    for (TInt i = 0; i < count; i++)
        {
        profiles |= iAccs[i]->AccInfo().iAudioOpenedProfiles;
        }  
    return profiles;
    }
    
void CBasrvAccMan::RejectAudioLink(const TBTDevAddr& aAddr, TAccAudioType aType)
    {
    TAudioRequest req;
    req.iAddr = aAddr;
    req.iAudioType = aType;
    req.iReqType = ERejectAudioOpenedByAcc;
    req.iOngoing = EFalse;
    TInt idx = 0;
    if (iAudioRequests.Count() && iAudioRequests[0].iOngoing)
        {
        idx = 1;
        }
    TInt err = iAudioRequests.Insert(req, idx);
    TRACE_INFO_SEG(
        {
        TBuf<12> buf;
        iCloseAddr.GetReadable(buf);
        Trace(_L("[Audio Request buf] 2(Reject) %d at '%S', to index %d"), iCloseType, &buf, idx);
        });

    if (!err && !iAsyncHandlingActive && !iAudioRequests[0].iOngoing)
        {
        // Start rejection asynchronously
        iAsyncHandlingActive = 
            CBasrvActive::New(*this, CActive::EPriorityStandard, KAsyncHandlingRequest);
        if (iAsyncHandlingActive)
            {
            iAsyncHandlingActive->iStatus = KRequestPending;
            TRequestStatus* sta = &(iAsyncHandlingActive->iStatus);
            User::RequestComplete(sta, KErrNone);
            iAsyncHandlingActive->GoActive();
            }
        }
    }

void CBasrvAccMan::CleanupLastItemFromClientRequestsArray(TAny* aPtr)
    {
    //cast the pointer passed in to the correct RArray type
    RArray<TClientRequestCache>& clientRequests = *static_cast<RArray<TClientRequestCache>*>(aPtr);
    
    //remove the last item from the array
    clientRequests.Remove(clientRequests.Count() - 1);
    }
