/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation for CBTSAController class. This class is advertised
*				 to E-com framework (@see proxy.cpp) so that it loads this class when 
*				 this plugin gets loaded. That is why this class implements the interface
*				 of the module (currently BT Accessory Server) which loads this plugin.Being
*				 main class, this class constructs other classes and also recieves callbacks,
*				 hence implementing their interfaces.  
*
*/


// INCLUDE FILES
#include "btsaController.h"
#include "btsacactive.h"
#include "btsacStateIdle.h"
#include "btsacStateListening.h"
#include "btsacStreamerController.h"
#include "btsacSEPManager.h"
#include "btsacGavdp.h"
#include "debug.h"
#include <e32property.h>
#include "btaudioremconpskeys.h"

_LIT_SECURITY_POLICY_C1( KPSKeyReadPolicy, 
                         ECapabilityLocalServices);
_LIT_SECURITY_POLICY_C1( KPSKeyWritePolicy, 
                         ECapabilityLocalServices);

// CONSTANTS

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTSAController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSAController* CBTSAController::NewL(TPluginParams& aParams)
    {
    CBTSAController* self = new (ELeave) CBTSAController(aParams);
    CleanupStack::PushL(self);
    self->ConstructL(  );
    CleanupStack::Pop(self);
    return self;
    }

    
// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTSAController::~CBTSAController()
    {
    TRACE_FUNC
    if(iStreamingSockets.Count())
		{
		TRACE_INFO((_L("[SOCKET] closed.")))
		iStreamingSockets[0].Close();
		}
	iStreamingSockets.Close();
	delete iStreamer;
    delete iGavdp;
    delete iState;
    delete iGavdpErrorActive;
    delete iLocalSEPs;
	delete iRemoteSEPs;
    iAccDb.ResetAndDestroy();
	iAccDb.Close();    
    RProperty::Delete( KBTAudioRemCon, KBTAudioPlayerControl );
    TRACE_INFO((_L("CBTSAController::~CBTSAController() completed")))
    }

// -----------------------------------------------------------------------------
// CBTSAController::CBTSAController
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CBTSAController::CBTSAController(TPluginParams& aParams)
    : CBTAccPlugin(aParams)
    {
  	TRACE_FUNC
    }
    
    
// -----------------------------------------------------------------------------
// CBTSAController::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTSAController::ConstructL()
    {
    TRACE_FUNC_ENTRY
    CBtsacState* state = CBtsacIdle::NewL(*this);
    CleanupStack::PushL(state);
	iStreamer = CBTSACStreamerController::NewL(*this);
    // Initially idle state is registered as observer of GAVDP
    iGavdp = CBTSACGavdp::NewL(state); 
 	iLocalSEPs = CBTSACSEPManager::NewL(TBTDevAddr());
 	iGavdpErrorActive = CBtsacActive::NewL(*this, CActive::EPriorityStandard, KRequestIdGavdpError); 	
 	CleanupStack::Pop(state);
 	ChangeStateL(state);
    ResetRemoteCache();
    
    TInt err = RProperty::Define( KBTAudioRemCon, 
                       KBTAudioPlayerControl, RProperty::EInt, 
                       KPSKeyReadPolicy, 
                       KPSKeyWritePolicy );
    TRACE_INFO((_L("PS define err %d"), err))
	TRACE_FUNC_EXIT
    }

// -----------------------------------------------------------------------------
// CBTSAController::ChangeStateL
// -----------------------------------------------------------------------------
//  

void CBTSAController::ChangeStateL(CBtsacState* aState)
  {
  TRACE_FUNC_ENTRY
  TRACE_ASSERT(aState != NULL, EBTPanicNullPointer);
  if(iState && aState->GetStateIndex() == iState->GetStateIndex())
      {
      // We already are in desired state, delete the state which came in and return
      TRACE_INFO((_L("CBTSAController::ChangeStateL(), already in desired state.")))
      delete aState;
      return;
      }
  delete iState;
  iState = aState;
  TInt err = KErrNone;
  TRAP(err, iState->EnterL());
  if (err)
      {
      ChangeStateL(iState->ErrorOnEntryL(err));
      }
  TRACE_FUNC_EXIT
  }


// -----------------------------------------------------------------------------
// CBTSAController::ResetRemoteCache
// -----------------------------------------------------------------------------
//
void CBTSAController::ResetRemoteCache()
    {
	TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBTSAController::InitializeSEPManager
// -----------------------------------------------------------------------------
//
void CBTSAController::InitializeSEPManager()
	{
    TRACE_FUNC
    iSBCSEPIndex = 0;   // SBC SEP Index in iRemoteSEPs array
    if( iRemoteSEPs )
    	{
    	delete iRemoteSEPs;
    	iRemoteSEPs = NULL;
    	}
	TRAPD(ret, iRemoteSEPs = CBTSACSEPManager::NewL(iRemoteAddr));
	if (ret)
		{
		TRACE_INFO((_L("CBTSAController::InitializeSEPManager(), ERROR")))
		}
	}

// -----------------------------------------------------------------------------
// CBTSAController::CleanSockets
// -----------------------------------------------------------------------------
//
void CBTSAController::CleanSockets()
	{
	TRACE_FUNC
	if(iStreamingSockets.Count())
		{
		// Close possible streaming socket(s)
		TRACE_INFO((_L("[SOCKET] closed.")))
		iStreamingSockets[0].Close();
		iStreamingSockets.Remove(0);
		iStreamingSockets.Reset();        
		}
	}

// -----------------------------------------------------------------------------
// CBTSAController::PluginType
// -----------------------------------------------------------------------------
//
TProfiles CBTSAController::PluginType()
	{
	TRACE_FUNC
	return EStereo;
	}

// -----------------------------------------------------------------------------
// CBTSAController::ConnectToAccessory
// -----------------------------------------------------------------------------
//
void CBTSAController::ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); 
         Trace(_L("[REQUEST] Connect request, BT Addr %S"), &buf);})
    iConnectStatus = &aStatus;
	aStatus = KRequestPending;
    TRAPD(err, iState->ConnectL(aAddr));
    if (err)
        {
        CompletePendingRequests(KConnectReq, err);
        }
    TRACE_FUNC_EXIT
	}

// -----------------------------------------------------------------------------
// CBTSAController::CancelConnectToAccessory
// -----------------------------------------------------------------------------
//
void CBTSAController::CancelConnectToAccessory(const TBTDevAddr& aAddr)
	{
	TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})
	if (aAddr != iRemoteAddr)
    	{
   	    TRACE_INFO((_L("CBTSAController::CancelConnectToAccessory() Invalid address, error.")))
   		CompletePendingRequests(KConnectReq, KErrArgument);
   		return;
    	}
    TRAP_IGNORE(iState->CancelConnectL());    
    TRACE_FUNC_EXIT
	}
	
// -----------------------------------------------------------------------------
// CBTSAController::DisconnectAccessory
// -----------------------------------------------------------------------------
//
void CBTSAController::DisconnectAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); 
         Trace(_L("[REQUEST] Disconnect request, BT Addr %S"), &buf);})
	
	// if we have an open/close audio request pending, complete it
	CompletePendingRequests((KOpenAudioReq | KCloseAudioReq), KErrDisconnected);
	iDisconnectStatus = &aStatus;
	aStatus = KRequestPending;
    if (aAddr != iRemoteAddr )
    	{
   	    TRACE_INFO((_L("CBTSAController::DisconnectAccessory() Invalid address, error.")))
   		CompletePendingRequests(KDisconnectReq, KErrArgument);
   		return;
    	}
    TRAPD(err, iState->DisconnectL());
    if (err)
        {
        CompletePendingRequests(KDisconnectReq, err);
        return;
        }
    SetResetAudioInput(ETrue);
    TRACE_FUNC_EXIT
	}
		
// -----------------------------------------------------------------------------
// CBTSAController::OpenAudioLink
// -----------------------------------------------------------------------------
//
void CBTSAController::OpenAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); 
         Trace(_L("[REQUEST] Open audio request, BT Addr %S"), &buf);})
    iOpenAudioStatus = &aStatus;
	aStatus = KRequestPending;
    TRAPD(err, iState->OpenAudioLinkL(aAddr));
    if (err)
        {
        CompletePendingRequests(KOpenAudioReq, err);
        }
    TRACE_FUNC_EXIT 		
	}

// -----------------------------------------------------------------------------
// CBTSAController::CancelOpenAudioLink
// -----------------------------------------------------------------------------
//
void CBTSAController::CancelOpenAudioLink(const TBTDevAddr& aAddr)
    {
	TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})
	if(aAddr != iRemoteAddr)
    	{
   	    TRACE_INFO((_L("CBTSAController::CancelOpenAudioLink() Invalid address, error.")))
   		CompletePendingRequests(KOpenAudioReq, KErrArgument);
   		return;
    	}
    if(IsOpenAudioReqFromAccFWPending())
	    {    	
	    TRAP_IGNORE(iState->CancelOpenAudioLinkL());
	    }
    TRACE_FUNC_EXIT
    }
	
// -----------------------------------------------------------------------------
// CBTSAController::CloseAudioLink
// -----------------------------------------------------------------------------
//
void CBTSAController::CloseAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); 
         Trace(_L("[REQUEST] Close audio request, BT Addr %S"), &buf);})
	iCloseAudioStatus = &aStatus;
	aStatus = KRequestPending;
    TRAPD(err, iState->CloseAudioLinkL(aAddr));
    if (err)
        {
        CompletePendingRequests(KCloseAudioReq, err);
        }
    TRACE_FUNC_EXIT
	}

// -----------------------------------------------------------------------------
// CBTSAController::CancelCloseAudioLink
// -----------------------------------------------------------------------------
//
void CBTSAController::CancelCloseAudioLink(const TBTDevAddr& aAddr)
	{
    TRACE_FUNC_ENTRY
    TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})
	if(aAddr != iRemoteAddr)
    	{
   	    TRACE_INFO((_L("CBTSAController::CancelCloseAudioLink() Invalid address, error.")))
   		CompletePendingRequests(KCloseAudioReq, KErrArgument);
   		return;
    	}
    if(iCloseAudioStatus)
	    {    	
	    iState->CancelCloseAudioLink(aAddr);
	    }
    TRACE_FUNC_EXIT
	}

// -----------------------------------------------------------------------------
// CBTSAController::AccInUse
// -----------------------------------------------------------------------------
//
void CBTSAController::AccInUse()
	{
 	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBTSAController::StartRecording
// -----------------------------------------------------------------------------
//
void CBTSAController::StartRecording()
	{
 	TRACE_FUNC
	iState->StartRecording(); 
	}
	
// -----------------------------------------------------------------------------
// CBTSAController::StoreAccInfo
// -----------------------------------------------------------------------------
//
void CBTSAController::StoreAccInfo()
	{
    TRACE_FUNC
	iAccDb.Append(iRemoteSEPs);
	}

// -----------------------------------------------------------------------------
// CBTSAController::DeleteAccInfo
// -----------------------------------------------------------------------------
//
void CBTSAController::DeleteAccInfo()
	{
    TRACE_FUNC
    for(TInt i = 0 ; i < iAccDb.Count() ; i++)
    	{
    	iAccDb.Remove(i);
    	}
	iAccDb.Reset();
	}
	
// -----------------------------------------------------------------------------
// CBTSAController::IsAccInfoAvailable
// -----------------------------------------------------------------------------
//
TBool CBTSAController::IsAccInfoAvailable()
	{
	TRACE_INFO((_L("CBTSAController::IsAccInfoAvailable() Count: %d"), iAccDb.Count()))
	for (TInt i=0; i<iAccDb.Count(); i++)	
		{
		CBTSACSEPManager* SM = iAccDb[i];
		if ( SM->GetDeviceAddr() == iRemoteAddr )	
			{
			iRemoteSEPs = iAccDb[i];	// retrieve SEP Manager
			return ETrue; 
			}
		}
	return EFalse;
	}

// -----------------------------------------------------------------------------
// CBTSAController::AbortStream
// -----------------------------------------------------------------------------
//
TInt CBTSAController::AbortStream()
	{
	TRACE_FUNC
	TAvdtpSEPInfo SEPInfo;
	if (iRemoteSEPs->GetInfo(iSBCSEPIndex, SEPInfo))
		{
		TRACE_INFO((_L("CBTSAController::AbortStream() Couldn't retrieve SEP Info !")))
		return KErrGeneral;
      	}
	TSEID remoteSEPid = SEPInfo.SEID(); 	
	iGavdp->AbortStreaming(remoteSEPid);
	return KErrNone;
	}

// -----------------------------------------------------------------------------
// CBTSAController::NewAccessory
// -----------------------------------------------------------------------------
//
void CBTSAController::NewAccessory(const TBTDevAddr& aAddr)
	{
 	TRACE_FUNC
 	TInt connectedProfiles = Observer().ConnectionStatus( aAddr );
 	if( !(connectedProfiles & EStereo) )
 	    {
 	    Observer().NewAccessory( aAddr, EStereo );
 	    } 
	}

// -----------------------------------------------------------------------------
// CBTSAController::DisconnectedFromRemote
// -----------------------------------------------------------------------------
//
void CBTSAController::DisconnectedFromRemote(const TBTDevAddr& aAddr, TInt /*aError*/)
	{
	TRACE_FUNC
	Observer().AccessoryDisconnected(aAddr, EStereo );
	}

// -----------------------------------------------------------------------------
// CBTSAController::AccessoryOpenedAudio
// -----------------------------------------------------------------------------
//
void CBTSAController::AccessoryOpenedAudio(const TBTDevAddr& aBDAddr)
	{
	TRACE_FUNC
	Observer().RemoteAudioOpened(aBDAddr, EStereo );
	}	


// -----------------------------------------------------------------------------
// CBTSAController::AccessoryClosedAudio
// -----------------------------------------------------------------------------
//		
void CBTSAController::AccessoryClosedAudio(const TBTDevAddr& aBDAddr)
	{
	TRACE_FUNC
	Observer().RemoteAudioClosed(aBDAddr, EStereo );
	}

// -----------------------------------------------------------------------------
// CBTSAController::AccessorySuspendedAudio
// -----------------------------------------------------------------------------
//		
void CBTSAController::AccessorySuspendedAudio(const TBTDevAddr& aBDAddr)
	{
	TRACE_FUNC
	RProperty::Set(KBTAudioRemCon, KBTAudioPlayerControl, EBTAudioPausePlayer);	
	Observer().RemoteAudioClosed(aBDAddr, EStereo );
	}

// -----------------------------------------------------------------------------
// CBTSAController::CompletePendingRequests
// -----------------------------------------------------------------------------
//
void CBTSAController::CompletePendingRequests(TUint aCompleteReq, TInt aError)
	{
	TRACE_INFO((_L("CBTSAController::CompletePendingRequests() Request %d"), aCompleteReq))
	
	if((aCompleteReq & KConnectReq) && iConnectStatus)
		{		
		TRACE_INFO((_L("[REQUEST] Connect request completed with %d"), aError))
		User::RequestComplete(iConnectStatus, aError);
		}
	if((aCompleteReq & KDisconnectReq) && iDisconnectStatus)
		{		
		TRACE_INFO((_L("[REQUEST] Disconnect request completed with %d"), aError))
		User::RequestComplete(iDisconnectStatus, aError);
		}
	if((aCompleteReq & KOpenAudioReq) && iOpenAudioStatus)
		{		
		TRACE_INFO((_L("[REQUEST] Open audio request completed with %d"), aError))
		User::RequestComplete(iOpenAudioStatus, aError);
		}
	if((aCompleteReq & KCloseAudioReq) && iCloseAudioStatus)
		{		
		TRACE_INFO((_L("[REQUEST] Close audio request completed with %d"), aError))
		User::RequestComplete(iCloseAudioStatus, KErrNone); // audio sw cant handle error codes.
		}
	}

// -----------------------------------------------------------------------------
// CBTSAController::SetRemoteAddr
// -----------------------------------------------------------------------------
//
void CBTSAController::SetRemoteAddr(const TBTDevAddr& aRemoteAddr)
	{
	iRemoteAddr = aRemoteAddr;
	}
	
// -----------------------------------------------------------------------------
// CBTSAController::GetRemoteAddr
// -----------------------------------------------------------------------------
//
TBTDevAddr CBTSAController::GetRemoteAddr() const
	{
	return iRemoteAddr;
	}

// -----------------------------------------------------------------------------
// CBTSAController::IsOpenAudioReqFromAccFWPending
// -----------------------------------------------------------------------------
//
TBool CBTSAController::IsOpenAudioReqFromAccFWPending() const
	{
	if(iOpenAudioStatus)
		return ETrue;
	else
		return EFalse;
	}

// -----------------------------------------------------------------------------
// CBTSAController::SetSEPIndex
// -----------------------------------------------------------------------------
//
void CBTSAController::SetSEPIndex(TInt aIndex)
	{
	iSBCSEPIndex = aIndex;
	}

// -----------------------------------------------------------------------------
// CBTSAController::GetSEPIndex
// -----------------------------------------------------------------------------
//
TInt CBTSAController::GetSEPIndex() const
	{
	return iSBCSEPIndex;
	}

// -----------------------------------------------------------------------------
// CBTSAController::SetResetAudioInput
// -----------------------------------------------------------------------------
//
void CBTSAController::SetResetAudioInput(TBool aReset)
	{
	iResetAudioInput = aReset;
	}

// -----------------------------------------------------------------------------
// CBTSAController::GetResetAudioInput
// -----------------------------------------------------------------------------
//
TBool CBTSAController::GetResetAudioInput() const
	{
	return iResetAudioInput;
	}

// -----------------------------------------------------------------------------
// CBTSAController::GetGavdpErrorActive
// -----------------------------------------------------------------------------
//
CBtsacActive* CBTSAController::GetGavdpErrorActive() const
	{
	return iGavdpErrorActive;
	}

// -----------------------------------------------------------------------------
// CBTSAController::NotifyError
// -----------------------------------------------------------------------------
//
void CBTSAController::NotifyError(TInt /*aError*/)
	{
	TRACE_FUNC
	if(iState->GetStateIndex() == EStateStreaming)
		{
		TRAP_IGNORE(iState->CloseAudioLinkL(GetRemoteAddr()));
		}
	else
		{
		TRAPD(err, ChangeStateL(CBtsacListening::NewL(*this, EGavdpResetReasonGeneral, EFalse)));
		if (err)
			{
			TRACE_INFO((_L("CBTSAController::NotifyError() Couldn't change state.")))
			}
		}
	CompletePendingRequests(KCompleteAllReqs, KErrCancel);
	}

// -----------------------------------------------------------------------------
// CBTSAController::GAVDP_Error
// -----------------------------------------------------------------------------
//
void CBTSAController::GAVDP_Error(TInt aError) 
	{
	TRACE_INFO((_L("CBTSAController::GAVDP_Error(%d)"), aError))
	if(iGavdpErrorActive)
		{
		if (!iGavdpErrorActive->IsActive())
		    {		    
		    TRequestStatus* sta = &(iGavdpErrorActive->iStatus);
		    iGavdpErrorActive->iStatus = KRequestPending;		    
		    User::RequestComplete(sta, aError);
		    iGavdpErrorActive->GoActive();
		    }
		}
	else
		{
		TRACE_INFO((_L("CBTSAController::CBTSAController() AO doesn't exist.")))
		}	
	}
	
// -----------------------------------------------------------------------------
// CBTSAController::RequestCompletedL
// -----------------------------------------------------------------------------
//	
void CBTSAController::RequestCompletedL(CBtsacActive& aActive)
	{
	TRACE_FUNC
	switch(aActive.RequestId())
		{
		case KRequestIdGavdpError:
			{
			iState->HandleGavdpErrorL(aActive.iStatus.Int());
			break;
			}
		default:
			{
			TRACE_INFO((_L("CBTSAController::RequestCompletedL() Unknown request")))
			break;
			}				
		}
	}

// -----------------------------------------------------------------------------
// CBTSAController::CancelRequest
// -----------------------------------------------------------------------------
//	
void CBTSAController::CancelRequest(CBtsacActive& /*aActive*/)
	{
	TRACE_FUNC
	}

//  End of File      
