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
* Description:  CBtmMan definition
*  Version     : %version: 15.1.7 %
*
*/


#include <btdevice.h>
#include <AccessoryTypes.h>
#include <btengdiscovery.h>
#include <btengdomaincrkeys.h>
#include <centralrepository.h>
#include "btmchandlerapi.h"
#include "btmman.h"
#include "btmactive.h"
#include "btmstate.h"
#include "btmslisten.h"
#include "debug.h"


// CONSTANTS
#ifdef _DEBUG // only used in traces
const TInt KBTDevAddrReadable = KBTDevAddrSize * 2;
#endif

const TInt KDestroyObsoleteStateOneShot = 20;

void CompleteRequest(TRequestStatus* aStatus, TInt aErr)
    {
    User::RequestComplete(aStatus, aErr);
    }

// ======== MEMBER FUNCTIONS ========

CBtmMan* CBtmMan::NewL(TPluginParams& aParams)
    {
    CBtmMan* self = new (ELeave) CBtmMan(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBtmMan::~CBtmMan()
    {
    TRACE_FUNC_ENTRY
    delete iActive;
    delete iCmdHandler;
    iTrashBin.ResetAndDestroy();
    iTrashBin.Close();
    delete iState;
    iMonoStateProp.Close();
    iMonoAddrProp.Close();
    iSockServ.Close();
    DeleteAllRegisteredServices();
    delete iBteng;
    iServices.Close();
    TRACE_FUNC_EXIT
    }

void CBtmMan::ChangeStateL(CBtmState* aState)
    {
    TRACE_FUNC_ENTRY
    TRACE_ASSERT(!!aState, EBTPanicNullPointer);
    if (iState)
        {
        iTrashBin.AppendL(iState);
        }
        
    iState = aState;
    TInt err = KErrNone;
    TRAP(err, iState->EnterL());
    if (err)
        {
        ChangeStateL(iState->ErrorOnEntryL(err));
        }
  
    if (iTrashBin.Count())
        {
        if (!iActive)
            {
            iActive = CBtmActive::NewL(*this, CActive::EPriorityStandard, KDestroyObsoleteStateOneShot);
            }
        if (!iActive->IsActive())
            {
            iActive->iStatus = KRequestPending;
            iActive->GoActive();
            TRequestStatus* sta = &(iActive->iStatus);
            CompleteRequest(sta, KErrNone);
            }
        }
    TRACE_FUNC_EXIT
    }

RSocketServ& CBtmMan::SockServ()
    {
    return iSockServ;
    }

void CBtmMan::NewAccessory(const TBTDevAddr& aAddr, TProfiles aProfile)
    {
    Observer().NewAccessory(aAddr, aProfile);
    }
    
void CBtmMan::AccessoryDisconnected(const TBTDevAddr& aAddr, TProfiles aProfile)    
    {
    Observer().AccessoryDisconnected(aAddr, aProfile);
    }

void CBtmMan::RemoteAudioOpened(const TBTDevAddr& aAddr, TProfiles aProfile)   
    {
    Observer().RemoteAudioOpened(aAddr, aProfile);
    }

void CBtmMan::RemoteAudioClosed(const TBTDevAddr& aAddr, TProfiles aProfile)
    {
    Observer().RemoteAudioClosed(aAddr, aProfile);
    }

void CBtmMan::RegisterServiceL(TUint aService, TUint aChannel)
    {
 	TRACE_FUNC_ENTRY
    TInt count = iServices.Count();
    
    for (TInt i = count - 1; i >= 0; i--)
        {
        if (iServices[i].iService == aService)
            {
            if (iServices[i].iChannel == aChannel)
                {
                TRACE_INFO((_L("Profile 0x%04X already registered at Channel %d"), aService, aChannel))
                return;
                }
            else
                {
                iBteng->DeleteSdpRecord(iServices[i].iHandle); 
                iServices.Remove(i);
                break;
                }
            }
        }

 	TSdpServRecordHandle handle;
 	TInt err = iBteng->RegisterSdpRecord(aService, aChannel, handle); 
    LEAVE_IF_ERROR(err);
    TBtmService record;
    record.iChannel = aChannel;
    record.iService = aService;
    record.iHandle = handle;
    iServices.AppendL(record);
    
    
    
	TRACE_INFO((_L("Profile 0x%04X new registered at Channel %d"), aService, aChannel))
	TRACE_FUNC_EXIT
    }

TInt CBtmMan::GetLastUsedChannel(TUint aService)
    {
    TInt count = iServices.Count();
    for (TInt i = count - 1; i >= 0; i--)
           {
           if (iServices[i].iService == aService)
               {
               return iServices[i].iChannel;
               }
           }
    
    return KErrNotFound;
    }

TInt CBtmMan::AudioLinkLatency()
    {
    return iState->AudioLinkLatency();
    }
TBool CBtmMan::IsTrashBinEmpty()
    {
    return iTrashBin.Count() == 0 ? ETrue : EFalse;
    }

void CBtmMan::DeleteRegisteredService(TUint aService)
    {
    TRACE_FUNC_ENTRY
    TInt count = iServices.Count();
    for (TInt i = count - 1; i >= 0; i--)
        {
        if (iServices[i].iService == aService)
            {
            iBteng->DeleteSdpRecord(iServices[i].iHandle);
            iServices.Remove(i);
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

void CBtmMan::DeleteAllRegisteredServices()
    {
    TInt count = iServices.Count();
    if (count > 0)
        {
        TRACE_FUNC_ENTRY
        for (TInt i = 0; i < count; i++)
            {
            iBteng->DeleteSdpRecord(iServices[i].iHandle);
            }        
        iServices.Reset();
        TRACE_FUNC_EXIT
        }
    }

void CBtmMan::LoadCmdHandlerL(TBtmcProfileId aProfile, const TBTDevAddr& aAddr, TBool aAccessoryInitiated)
    {
    if (!iCmdHandler)
        {
        iCmdHandler = CBtmcHandlerApi::NewL(*this, aProfile, aAddr.Des(), aAccessoryInitiated);
        }
    }
    
void CBtmMan::DeleteCmdHandler()
    {
    delete iCmdHandler;
    iCmdHandler = NULL;
    }
    
void CBtmMan::NewProtocolDataL(const TDesC8& aData)
    {
    if(iCmdHandler)
        {
        iCmdHandler->HandleProtocolDataL(aData);
        }
    else
        {
        // Panic in udeb builds only, disconnect remote in urel
        __ASSERT_DEBUG(EFalse, PANIC(EBTPanicNullCmdHandler));
        LEAVE(KErrNotReady);
        }
    }

TBool CBtmMan::IsAccInuse() const
    {
    return iAccInuse;
    }

TBool CBtmMan::IsEdrSupported() const
    {
    return iEdr;
    }
    
void CBtmMan::ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})
    TRAPD(err, iState->ConnectL(aAddr, aStatus));
    if (err)
        {
        CompleteRequest(&aStatus, err);
        }
    TRACE_FUNC_EXIT
    }

void CBtmMan::CancelConnectToAccessory(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})
    TRAP_IGNORE(iState->CancelConnectL());
    (void) aAddr;
    TRACE_FUNC_EXIT
    }

void CBtmMan::DisconnectAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);});      
    TRAPD(err, iState->DisconnectL(aAddr, aStatus));
    if (err)
        {
        CompleteRequest(&aStatus, err);
        }
    TRACE_FUNC_EXIT
    }

void CBtmMan::OpenAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); 
         Trace(_L("Request to open audio to BT Addr %S"), &buf);})
    TRAPD(err, iState->OpenAudioLinkL(aAddr, aStatus));
    if (err)
        {
        CompleteRequest(&aStatus, err);
        }
    TRACE_FUNC_EXIT    
    }

void CBtmMan::CancelOpenAudioLink(const TBTDevAddr& aAddr)
    {
    TRAP_IGNORE(iState->CancelOpenAudioLinkL(aAddr));
    }

void CBtmMan::CloseAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})        
    TRAPD(err, iState->CloseAudioLinkL(aAddr, aStatus));
    if (err)
        {
        CompleteRequest(&aStatus, err);
        }
    TRACE_FUNC_EXIT
    }

void CBtmMan::CancelCloseAudioLink(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})        
    TRAP_IGNORE(iState->CancelCloseAudioLinkL(aAddr));
    TRACE_FUNC_EXIT
    }
    
void CBtmMan::AccInUse()
    {
    TRACE_FUNC_ENTRY
    iAccInuse = ETrue;
    iState->AccInUse();
    TRACE_FUNC_EXIT
    }

void CBtmMan::AccOutOfUse()
    {
    TRACE_FUNC
    iAccInuse = EFalse;
    }

TProfiles CBtmMan::PluginType()
    {
    return EAnyMonoAudioProfiles;
    }
    

void CBtmMan::ActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    if(iCmdHandler)
        iCmdHandler->ActivateRemoteVolumeControl();
    else
        {
        TRACE_INFO((_L("Null cmd handler")))
        }
    }

void CBtmMan::DeActivateRemoteVolumeControl()
    {
    if (iCmdHandler)
        iCmdHandler->DeActivateRemoteVolumeControl();
    }

TInt CBtmMan::GetRemoteSupportedFeature()
    {
    if (iCmdHandler)
        return iCmdHandler->GetRemoteSupportedFeature();
    return 0;
    }
    
CBtmMan::CBtmMan(TPluginParams& aParams)
    : CBTAccPlugin(aParams)
    {
    TRACE_FUNC
    }

void CBtmMan::RequestCompletedL(CBtmActive& /*aActive*/)
    {
    TRACE_FUNC
    TRACE_INFO((_L("TRASHBIN SIZE: %d"), iTrashBin.Count()))
    iTrashBin.ResetAndDestroy();
    if( iState )
        {
        iState->StartListenerL();
        }
        
    }

void CBtmMan::CancelRequest(CBtmActive& /*aActive*/)
    {
    }

void CBtmMan::SlcIndicateL(TBool aSlc)
    {
    if (iState)
        iState->SlcIndicateL(aSlc);
    }

void CBtmMan::SendProtocolDataL(const TDesC8& aData)
    {
    if (iState)
        iState->SendProtocolDataL(aData);
    }

void CBtmMan::ConstructL()
    {
    TRACE_FUNC_ENTRY
    LEAVE_IF_ERROR(iSockServ.Connect());
    TInt edr = EBTEScoNotSupported;
    CRepository* cenrep = NULL;
    TRAP_IGNORE(cenrep = CRepository::NewL(KCRUidBluetoothEngine));
    if (cenrep)
        {
        cenrep->Get(KBTEScoSupportedLV, edr);
        delete cenrep;
        }
    iEdr = (edr == EBTEScoSupported) ? ETrue : EFalse;
    TRACE_INFO((_L("EDR feature %d"), iEdr))
    iBteng = CBTEngDiscovery::NewL();
    CBtmState* state = CBtmsListen::NewL(*this);
    CleanupStack::PushL(state);
    ChangeStateL(state);
    CleanupStack::Pop(state);
    TRACE_FUNC_EXIT
    }
    
