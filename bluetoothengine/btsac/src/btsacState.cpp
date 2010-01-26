/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Default implementation of BTSAC state machine handles
*				 methods which the active state is not supposed to handle
*
*/


// INCLUDE FILES
#include "btsacactive.h"
#include "btsacState.h"
#include "btsacStateIdle.h"
#include "btsacStateListening.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacState::CBtsacState
// -----------------------------------------------------------------------------
//
CBtsacState::CBtsacState(CBTSAController& aParent, TStateIndex aStateIndex)
: iParent(aParent), iStateIndex(aStateIndex)
    {
    }

// -----------------------------------------------------------------------------
// CBtsacState::~CBtsacState
// -----------------------------------------------------------------------------
//
CBtsacState::~CBtsacState()
    {
    }    

// -----------------------------------------------------------------------------
// CBtsacState::ErrorOnEntryL
// -----------------------------------------------------------------------------
//    
CBtsacState* CBtsacState::ErrorOnEntryL(TInt /*aReason*/)
    {
    TRACE_FUNC
    return CBtsacListening::NewL(iParent, EGavdpResetReasonNone, KErrNone);
    }

// -----------------------------------------------------------------------------
// CBtsacState::Parent
// -----------------------------------------------------------------------------
//    
 CBTSAController& CBtsacState::Parent()
    {
    return iParent;
    }
    
 // -----------------------------------------------------------------------------
// CBtsacState::GavdpErrorActive
// -----------------------------------------------------------------------------
//    
 CBtsacActive* CBtsacState::GavdpErrorActive()
    {
    return iParent.GetGavdpErrorActive();
    }

// -----------------------------------------------------------------------------
// CBtsacState::ConnectL
// -----------------------------------------------------------------------------
//
void CBtsacState::ConnectL(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
    Parent().CompletePendingRequests(KConnectReq, KErrInUse);
    }

// -----------------------------------------------------------------------------
// CBtsacState::CancelConnectL
// -----------------------------------------------------------------------------
//
void CBtsacState::CancelConnectL()
    {
    TRACE_FUNC
    Parent().CompletePendingRequests(KConnectReq, KErrNotSupported);
    }

// -----------------------------------------------------------------------------
// CBtsacState::DisconnectL
// -----------------------------------------------------------------------------
//
void CBtsacState::DisconnectL()
    {
    TRACE_FUNC
    Parent().CompletePendingRequests(KDisconnectReq, KErrCouldNotDisconnect);
    }

// -----------------------------------------------------------------------------
// CBtsacState::DisconnectedFromRemoteOrError
// -----------------------------------------------------------------------------
//
void CBtsacState::DisconnectedFromRemoteOrError(TInt /*aErr*/)
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacState::Configure()
// -----------------------------------------------------------------------------
//
void CBtsacState::Configure()
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacState::OpenAudioLinkL()
// -----------------------------------------------------------------------------
//
void CBtsacState::OpenAudioLinkL(const TBTDevAddr& /*aAddr*/)
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KOpenAudioReq, KErrNotSupported);
	}

// -----------------------------------------------------------------------------
// CBtsacState::CancelOpenAudioLinkL()
// -----------------------------------------------------------------------------
//
void CBtsacState::CancelOpenAudioLinkL()
    {
	TRACE_FUNC
	Parent().CompletePendingRequests(KOpenAudioReq, KErrNotSupported);
    }

// -----------------------------------------------------------------------------
// CBtsacState::CloseAudioLinkL
// -----------------------------------------------------------------------------
//
void CBtsacState::CloseAudioLinkL(const TBTDevAddr& /*aAddr*/)
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KCloseAudioReq, KErrNotSupported);
	}

// -----------------------------------------------------------------------------
// CBtsacState::CancelCloseAudioLink
// -----------------------------------------------------------------------------
//
void CBtsacState::CancelCloseAudioLink(const TBTDevAddr& /*aAddr*/)
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KCloseAudioReq, KErrNotSupported);
	}

// -----------------------------------------------------------------------------
// CBtsacState::StartRecording
// -----------------------------------------------------------------------------
//
void CBtsacState::StartRecording()
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_ConnectConfirm
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_ConnectConfirm(const TBTDevAddr& /*aDeviceAddr*/)
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SEPDiscovered
// -----------------------------------------------------------------------------
//		
void CBtsacState::GAVDP_SEPDiscovered(const TAvdtpSEPInfo& /*aSEP*/)
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SEPDiscoveryComplete
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_SEPDiscoveryComplete() 	
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SEPCapability
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_SEPCapability(TAvdtpServiceCapability* /*aCapability*/)
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SEPCapabilityComplete
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_SEPCapabilityComplete() 	
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_AbortStreamConfirm
// -----------------------------------------------------------------------------
//		
void CBtsacState::GAVDP_AbortStreamConfirm()
	{
	TRACE_INFO((_L("CBtsacState::GAVDP_AbortStreamConfirm(), state %d"), GetStateIndex()))
	Parent().CompletePendingRequests(KCompleteAllReqs, KErrAbort);
	TRAPD(err, Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected)));
	if (err)
		{
		TRACE_INFO((_L("CBtsacState::GAVDP_AbortStreamConfirm() Couldn't change state.")))
		}
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_StartStreamsConfirm
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_StartStreamsConfirm()
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SuspendStreamsConfirm
// -----------------------------------------------------------------------------
//		
void CBtsacState::GAVDP_SuspendStreamsConfirm()
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SecurityControlConfirm
// -----------------------------------------------------------------------------
//		
void CBtsacState::GAVDP_SecurityControlConfirm(const TDesC8& /*aResponseData*/)
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_ConfigurationConfirm
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_ConfigurationConfirm()
	{
	TRACE_FUNC
	// configuration complete and SEP selected *AND* reconfigure confirm
	}

// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_ConfigurationStartIndication
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_ConfigurationStartIndication(TSEID /*aLocalSEID*/, TSEID /*aRemoteSEID*/) 
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_ConfigurationIndication
// -----------------------------------------------------------------------------
//		
TInt CBtsacState::GAVDP_ConfigurationIndication(TAvdtpServiceCapability* /*aCapability*/) 
	{
	TRACE_FUNC
	return KErrGeneral; 	
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_ConfigurationEndIndication
// -----------------------------------------------------------------------------
//
TInt CBtsacState::GAVDP_ConfigurationEndIndication() 
	{
	TRACE_FUNC
	return KErrGeneral;	
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_StartIndication
// -----------------------------------------------------------------------------
//
TInt CBtsacState::GAVDP_StartIndication(TSEID /*aSEID*/) 
	{
	TRACE_FUNC
	return KErrGeneral;	
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SuspendIndication
// -----------------------------------------------------------------------------
//
TInt CBtsacState::GAVDP_SuspendIndication(TSEID /*aSEID*/) 
	{
	TRACE_FUNC
	return KErrGeneral;
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_SecurityControlIndication
// -----------------------------------------------------------------------------
//
TInt CBtsacState::GAVDP_SecurityControlIndication(TSEID /*aSEID*/, TDes8& /*aSecurityDataInOut*/) 
	{
	TRACE_FUNC
	return KErrGeneral;
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_AbortIndication
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_AbortIndication(TSEID aSEID) 
	{
	TRACE_INFO((_L("CBtsacState::GAVDP_AbortIndication(), state %d, SEID:%d"), GetStateIndex(), aSEID.SEID()))
	(void)aSEID;
	Parent().CompletePendingRequests(KCompleteAllReqs, KErrDisconnected);
	TRAPD(err, Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected)))
	if(err)
		{
		TRACE_INFO((_L("CBtsacState::GAVDP_AbortIndication(), error! Couldn't change state")))
		}
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_ReleaseIndication
// -----------------------------------------------------------------------------
//		
void CBtsacState::GAVDP_ReleaseIndication(TSEID /*aSEID*/) 
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_BearerReady
// -----------------------------------------------------------------------------
//			
void CBtsacState::GAVDP_BearerReady(RSocket /*aSocket*/, const TAvdtpSockAddr& /*aAddress*/)
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBtsacState::RequestCompletedL
// -----------------------------------------------------------------------------
//	
void CBtsacState::RequestCompletedL(CBtsacActive& /*aActive*/)
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacState::CancelRequest
// -----------------------------------------------------------------------------
//	
void CBtsacState::CancelRequest(CBtsacActive& /*aActive*/)
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacState::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//
void CBtsacState::HandleGavdpErrorL(TInt /*aError*/)
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacState::GetState
// -----------------------------------------------------------------------------
//
TStateIndex CBtsacState::GetStateIndex() const
	{
	return iStateIndex;
	}

// -----------------------------------------------------------------------------
// CBtsacState::GAVDP_Error
// -----------------------------------------------------------------------------
//
void CBtsacState::GAVDP_Error(TInt aError, const TDesC8& /*aErrorData*/) 
	{
	TRACE_INFO((_L("CBtsacState::GAVDP_Error(%d)"), aError))
	Parent().GAVDP_Error(aError);
	}
	
//  End of File  
