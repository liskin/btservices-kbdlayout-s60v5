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
* Description:  In this state, BTSAC is listening for remote connection
*								 from a stereo audio accessory
*
*/




// INCLUDE FILES
#include "btsacStateConnected.h"
#include "btsacStateConfigured.h"
#include "btsacStateConfiguring.h"
#include "btsacStateListening.h"
#include "btsacStateAborting.h"
#include "btsacactive.h"
#include "btsacGavdp.h"
#include "btsacSEPManager.h"
#include "btsacStreamerController.h"
#include "debug.h"

// A2DP codec-specific element definitions
// in bluetoothAV.h
using namespace SymbianBluetoothAV;   

// Subband codec-specific values
// in bluetoothAV.h
using namespace SymbianSBC;   

const TInt KSEPDiscoverResponseDelay = 5000000; 	// 5 sec
const TInt KGetCapabilitiesResponseDelay = 4000000; // 4 sec
const TInt KSEPConfigureResponseDelay = 4000000; 	// 4 sec
const TInt KCreateBearersResponseDelay = 5000000; 	// 5 sec
const TInt KWaitConfStartIndicationDelay = 3000000; // 3 sec
const TInt KOneSecondDelay = 1000000; 				// 1 sec


// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacConnected::NewL
// -----------------------------------------------------------------------------
//
CBtsacConnected* CBtsacConnected::NewL(CBTSAController& aParent, TBTConnType aConnType)
    {
    CBtsacConnected* self = new( ELeave ) CBtsacConnected(aParent, aConnType);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::CBtsacConnected
// -----------------------------------------------------------------------------
//
CBtsacConnected::CBtsacConnected(CBTSAController& aParent, TBTConnType aConnType)
:	CBtsacState(aParent, EStateConnected), iSuitableSEPFoundAlready(EFalse), iConnType(aConnType),
	iSEPFound(ESEPNotInitialized), iRemoteSEPIndex(0), iBearersQuery(EFalse), iCancelConnectReq(EFalse),
	iAudioOpenedBy(EAudioOpenedByNone)
    {
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::ConstructL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::ConstructL()
	{
	iTimerActive = CBtsacActive::NewL(*this, CActive::EPriorityStandard, KRequestIdTimer);
	iTimer.CreateLocal();
	}
    
// -----------------------------------------------------------------------------
// CBtsacConnected::~CBtsacConnected
// -----------------------------------------------------------------------------
//    
CBtsacConnected::~CBtsacConnected()
    {
    TRACE_FUNC_ENTRY
    delete iTimerActive;
    iTimerActive = NULL;
    iTimer.Close();
    TRACE_FUNC_EXIT
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Connected"))	
	_LIT(KName, "CBtsacStateConnected");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);

	switch(iConnType)
		{
		case EOutgoingConnWithAudio:
		case EOutgoingConn:
			{
			if(iConnType == EOutgoingConnWithAudio)
				{
				iAudioOpenedBy = EAudioOpenedByAFW;
				}
			StartTimer(KSEPDiscoverResponseDelay, KRequestIdTimer);
			Parent().iGavdp->DiscoverSEPs(); // start with SEP discovery			
			break;
			}
		case EOutgoingConnWithAudioNoDiscovery:
		case EOutgoingConnNoDiscovery:
			{
			if(iConnType == EOutgoingConnWithAudioNoDiscovery)
				{
				iAudioOpenedBy = EAudioOpenedByAFW;
				}
			ConfigureL();
			break;
			}
		case EIncomingConn:
			{
			Parent().NewAccessory(Parent().GetRemoteAddr());
			break;
			}
		case EConnExists:
			break;
		}
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::StartTimer
// -----------------------------------------------------------------------------
//
void CBtsacConnected::StartTimer(TTimeIntervalMicroSeconds32 aTimeout, TInt aRequestId)
	{
	TRACE_FUNC
	if (iTimerActive)
    	{
	    if(!iTimerActive->IsActive())
		    {	    	
	        iTimerActive->SetRequestId(aRequestId);
	        iTimer.After(iTimerActive->iStatus, aTimeout);
	        iTimerActive->GoActive();
		    }
        }
    else
    	{
    	TRACE_INFO((_L("CBtsacConnected::StartTimer, Timer Active doesn't exist.")))
    	}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::StopTimer
// -----------------------------------------------------------------------------
//
void CBtsacConnected::StopTimer()
	{
	TRACE_FUNC
	if (iTimerActive)
    	{
		iTimerActive->Cancel();
        }
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::CancelActionL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::CancelActionL(TInt aError, TBTSACGavdpResetReason aGavdpReset)
    {
    TRACE_FUNC
    StopTimer();
	Parent().CompletePendingRequests((KConnectReq | KOpenAudioReq), aError);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), aGavdpReset, aError));
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::OpenAudioLinkL
// -----------------------------------------------------------------------------
//    
void CBtsacConnected::OpenAudioLinkL(const TBTDevAddr& aAddr)
	{  
	TRACE_FUNC	
	
	if (Parent().GetRemoteAddr() != aAddr)
		{
		TRACE_INFO((_L("CBtsacConnected::OpenAudio, Error!")))
		CancelActionL(KErrNotFound, EGavdpResetReasonGeneral);
		return;
		}
	iAudioOpenedBy = EAudioOpenedByAFW;
	Parent().SetRemoteAddr(aAddr);
	
	if(iConnType == EIncomingConn)
		{
		// Accessory has created connection to us. Don't open audio immediately, 
		// let's wait for a while if accessory starts configuration. This way
		// we can avoid configure collision.
		StartTimer(KOneSecondDelay, KRequestIdWaitRemoteConfStart);
		}
	else
		{
		StartConfigurationL();
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::CancelOpenAudioLinkL()
// -----------------------------------------------------------------------------
//    
void CBtsacConnected::CancelOpenAudioLinkL()
    {
    TRACE_FUNC
	CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
    }
	
// -----------------------------------------------------------------------------
// CBtsacConnected::DisconnectL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::DisconnectL()
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KDisconnectReq, KErrNone);
	CancelActionL(KErrCancel, EGavdpResetReasonDisconnect);
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::CancelConnectL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::CancelConnectL()
    {
    TRACE_FUNC
    if(!iBearersQuery)
    	{
		CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
    	}
    else
    	{
    	// We have to wait bearers to be completed
    	TRACE_INFO((_L("CBtsacConnected::CancelConnectL() Wait for BearerReady.")))
    	iCancelConnectReq = ETrue;
    	}
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::StartConfigurationL()
// -----------------------------------------------------------------------------
//    
void CBtsacConnected::StartConfigurationL()
	{
	TRACE_FUNC
	if (!Parent().iRemoteSEPs->NoOfSEPs())	// if accessory info not stored already
		{
		iRemoteSEPIndex = 0;
		Parent().InitializeSEPManager();
		StartTimer(KSEPDiscoverResponseDelay, KRequestIdTimer);
		Parent().iGavdp->DiscoverSEPs();		// start with SEP discovery
		}
	else	// otherwise we have explored this accessory before - go straight to configure 
		{
		ConfigureL();
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::ConfigureL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::ConfigureL()
    {
    TRACE_FUNC
   
 	RPointerArray<TAvdtpServiceCapability> SEPCapabilities;
    if ((Parent().iRemoteSEPs->GetCaps(Parent().GetSEPIndex(), SEPCapabilities)) )
		{
		TRACE_INFO((_L("CBtsacConnected::Configure() Couldn't retrieve Capabilities !")))
		CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
	   	SEPCapabilities.Close();
		return; 
		}   
    
	TRACE_INFO((_L("CBtsacConnected::Configure() Retrieve %d capabilities"), SEPCapabilities.Count()))
   	// loop through all capablities to find sbc codec capablities
   	TSBCCodecCapabilities SBCCaps; 
   	TAvdtpMediaTransportCapabilities MedTransCaps; 
   	for (TInt index=0; index < SEPCapabilities.Count(); index++)		
   		{
		TAvdtpServiceCapability* cap = SEPCapabilities[index];
		TAvdtpServiceCategory cat = cap->Category();
		if ( cat == EServiceCategoryMediaCodec )
			{
		    SBCCaps = static_cast<TSBCCodecCapabilities&>(*cap);	
		    TRACE_INFO((_L("CBtsacConnected::Configure() SBC Caps retrieved.")))
			}
		else if ( cat == EServiceCategoryMediaTransport )
			{
			MedTransCaps = static_cast<TAvdtpMediaTransportCapabilities&>(*cap);
			TRACE_INFO((_L("CBtsacConnected::Configure() Media Transport Caps retrieved.")))
			}
   		}
   	SEPCapabilities.Close();

   // Check if headset's capabilities suits us				
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory Sampling Frequencies: %d"), SBCCaps.SamplingFrequencies()))
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory Channel modes: %d"), SBCCaps.ChannelModes()))
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory Blocks: %d"), SBCCaps.BlockLengths()))
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory SubBands: %d"), SBCCaps.Subbands()))
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory Alloc method: %d"), SBCCaps.AllocationMethods()))
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory Max bitpool: %d"), SBCCaps.MaxBitpoolValue()))
	TRACE_INFO((_L("CBtsacConnected::Configure() Accessory Min bitpool: %d"), SBCCaps.MinBitpoolValue()))
	
	if (Parent().iStreamer->ConfigureSEP(SBCCaps) )
		{
   		TRACE_INFO((_L("CBtsacConnected::Configure() Streamer couldn't configure SEP !")))
        CancelActionL(KErrCancel, EGavdpResetReasonGeneral); 
        return;   // capabilites doesn't suit us
		}
		
	TAvdtpSEPInfo SEPInfo;
	if (Parent().iRemoteSEPs->GetInfo(Parent().GetSEPIndex(), SEPInfo))
		{
		TRACE_INFO((_L("CBtsacConnected::Configure() Couldn't retrieve SEP Info !")))
		CancelActionL(KErrCancel, EGavdpResetReasonGeneral); 
        return;   // capabilites doesn't suit us
		}
	TSEID remoteSEPid = SEPInfo.SEID(); 	
	// local sep index is hard coded cause current implementation is only sbc encoded
	TSEID localSEPid = TSEID(1, ETrue);
 	
 	StartTimer(KSEPConfigureResponseDelay, KRequestIdTimer);
	if ( Parent().iGavdp->ConfigureSEP(localSEPid, remoteSEPid , SBCCaps, MedTransCaps ) )
		{
		TRACE_INFO((_L("CBtsacConnected::Configure() ConfigureSEP returned Error !!!")))
		CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
		}
    }
    
// -----------------------------------------------------------------------------
// CBtsacConnected::GetCapabilitiesOfAllSEPs
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GetCapabilitiesOfAllSEPs()
	{
	TRACE_INFO((_L("CBtsacConnected::GetCapabilitiesOfAllSEPs() CurrentSEP:%d"), iRemoteSEPIndex))
	TRACE_INFO((_L("CBtsacConnected::GetCapabilitiesOfAllSEPs() Total Remote SEPs registered:%d"), Parent().iRemoteSEPs->NoOfSEPs()))

	if ( iRemoteSEPIndex < Parent().iRemoteSEPs->NoOfSEPs() )
		{
		TAvdtpSEPInfo SEPInfo;
		if (!Parent().iRemoteSEPs->GetInfo(iRemoteSEPIndex, SEPInfo))
			{
			TSEID id = SEPInfo.SEID(); 
			Parent().iGavdp->GetCapabilities(id);	
			}
		}
	else // we have covered all SEPs
		{
		StopTimer();
		// store all info in our db
		//Parent().StoreAccInfo();  // stores iRemoteSEPs (SEPManager) into database
		iSuitableSEPFoundAlready = EFalse;
		
		if ( iSEPFound == ESEPConfigure ) 
			{
			TRAP_IGNORE(ConfigureL());
			}
		else if ( iSEPFound == ESEPInUse )
			{
			// We have open audio or connect request pending but do not complete it yet. We expect that
			// accessory will start configure us. We should receive GAVDP_ConfigurationStartIndication
			// from the remote and remote will configure us. Eventually acc FW's open audio request
			// will be completed in CBTsacConfigured state and connect request will be completed in
			// CBTsacConfiguring state.

			// For safety's sake start timer to protect the situation where we do not receive
			// GAVDP_ConfigurationStartIndication.
			StartTimer(KWaitConfStartIndicationDelay, KRequestIdTimer);
			}
		else // no audio sbc sink sep found
			{
			TRAP_IGNORE(CancelActionL(KErrCancel, EGavdpResetReasonGeneral));
			}
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_SEPDiscovered
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEPInfo)
	{
	TRACE_INFO((_L("CBtsacConnected:::GAVDP_SEPDiscovered() SEID: %d  InUse: %d  MediaType: %d  IsSink: %d"),
				aSEPInfo.SEID().SEID(), aSEPInfo.InUse(), aSEPInfo.MediaType(), aSEPInfo.IsSink()))
	
	// BTSAC cares only about audio SEPs which are sink
     if (aSEPInfo.MediaType() == EAvdtpMediaTypeAudio && aSEPInfo.IsSink() )
     	{
  		TRAPD(err, Parent().iRemoteSEPs->NewSEPL(aSEPInfo))
		if (!err)
			{
			Parent().iRemoteSEPs->SetState(aSEPInfo.SEID(), CBTSACStreamEndPoint::EDiscoveredRemote, &aSEPInfo);
			}
		else // internal problem
			{
  			TRAP_IGNORE(CancelActionL(KErrCancel, EGavdpResetReasonGeneral));
			}
     	}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_SEPDiscoveryComplete
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GAVDP_SEPDiscoveryComplete()
	{
	TRACE_FUNC
	StopTimer();
	if ( Parent().iRemoteSEPs->NoOfSEPs() )
		{
		StartTimer(KGetCapabilitiesResponseDelay, KRequestIdTimer);
		GetCapabilitiesOfAllSEPs();
		}
	else // remote A2DP has no 'audio' 'sink' SEPs ! naughty remote
		{
		TRACE_INFO((_L("CBtsacConnected::GAVDP_SEPDiscoveryComplete() Remote A2dP has no 'audio' 'sink' SEPs !")))
		TRAP_IGNORE(CancelActionL(KErrCancel, EGavdpResetReasonGeneral));
		}
	}
	
// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_SEPCapability
// -----------------------------------------------------------------------------
//	
void CBtsacConnected::GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability)
	{
	TRACE_INFO((_L("CBtsacConnected::GAVDP_SEPCapability(%d)"), aCapability->Category()))

	Parent().iRemoteSEPs->StoreCaps(iRemoteSEPIndex, aCapability);
	
	if( iSuitableSEPFoundAlready )
		{
		return; 
		}

	if ( aCapability->Category() == EServiceCategoryMediaCodec )
		{
		TAvdtpMediaCodecCapabilities& codecCaps = static_cast<TAvdtpMediaCodecCapabilities&>(*aCapability);
		TAvdtpSEPInfo SEPInfo;
		TBool InUse = EFalse;

		if ( !Parent().iRemoteSEPs->GetInfo(iRemoteSEPIndex, SEPInfo))
			{
			InUse = SEPInfo.InUse();
			}
		else
			{
			TRACE_INFO((_L("CBtsacConnected::GAVDP_SEPCapability() Couldn't retrieve SEP Info !")))
			return;
			}

		if ( (codecCaps.MediaCodecType() == EAudioCodecSBC) && !InUse) // found SEP that we are interested in
			{
			TRACE_INFO((_L("CBtsacConnected::GAVDP_SEPCapability() Found SBC Audio Sink SEP !!!")))
			iSuitableSEPFoundAlready = ETrue;
			iSEPFound = ESEPConfigure;
			Parent().SetSEPIndex(iRemoteSEPIndex);
			}
		else
			{
			TRACE_INFO((_L("CBtsacConnected::GAVDP_SEPCapability() Remote SEP In Use or Codec Type not suitable !!!")))
			iSEPFound = InUse ? ESEPInUse : ESEPCodecTypeNotAllowed;
			}
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_SEPCapabilityComplete
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GAVDP_SEPCapabilityComplete()
	{
	TRACE_FUNC
	iRemoteSEPIndex++;	
	GetCapabilitiesOfAllSEPs();
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_ConfigurationConfirm
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GAVDP_ConfigurationConfirm()
	{
	TRACE_FUNC
	StopTimer();
	TAvdtpSEPInfo SEPInfo;
	if (Parent().iRemoteSEPs->GetInfo(Parent().GetSEPIndex(), SEPInfo))
		{
		TRACE_INFO((_L("CBtsacConnected::GAVDP_ConfigurationConfirm Couldn't retrieve SEP Info !")))
		TRAP_IGNORE(CancelActionL(KErrCancel, EGavdpResetReasonGeneral));
       	return;   // cannot get remote SEP capabilites 
		}
	TSEID remoteSEPid = SEPInfo.SEID();
	TRACE_INFO((_L("CBtsacConnected::GAVDP_ConfigurationConfirm() Asking for bearer for remote SEID(%d)"), remoteSEPid.SEID()))	
	iBearersQuery = ETrue;
	StartTimer(KCreateBearersResponseDelay, KRequestIdTimer);
	Parent().iGavdp->CreateBearers(remoteSEPid);
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_BearerReady
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GAVDP_BearerReady(RSocket aNewSocket, const TAvdtpSockAddr& aAddr)
	{
	TRACE_FUNC
	TRACE_INFO((_L("CBtsacConnected::GAVDP_BearerReady() SEID: %d"), aAddr.SEID().SEID()))
	(void)aAddr.SEID(); 	
	StopTimer();
	if(!iCancelConnectReq)
		{		
		if(iConnType != EIncomingConn)
			{
			// New accessory has been already informed if connection type is incoming,
			// otherwise tell about new accessory
		    Parent().NewAccessory(Parent().GetRemoteAddr());
			}
		
		if(iConnType == EOutgoingConn || iConnType == EOutgoingConnNoDiscovery)
			{
			// If connection is without audio, just complete pending connect request if any.
			// Otherwise pending request(s) is/are completed in configured state.
			Parent().CompletePendingRequests(KConnectReq, KErrNone);
			}			
		TRAP_IGNORE(Parent().ChangeStateL(CBtsacConfigured::NewL(Parent(), aNewSocket, iAudioOpenedBy, EStreamConfiguredBySrc)));
		}
	else
		{
		TRACE_ASSERT(Parent().iStreamingSockets.Count() == 0, EBTPanicSocketExists)
		TRACE_INFO((_L("[SOCKET] created.")))
		Parent().iStreamingSockets.Append(aNewSocket);
		Parent().CompletePendingRequests(KCompleteAllReqs, KErrCancel);
		TRAP_IGNORE(Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrNone)));
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_AbortIndication
// -----------------------------------------------------------------------------
//	
void CBtsacConnected::GAVDP_AbortIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBtsacConnected::GAVDP_AbortIndication() SEID:%d"), aSEID.SEID()))
	(void)aSEID;
	TRAP_IGNORE(CancelActionL(KErrDisconnected, EGavdpResetReasonNone));
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::GAVDP_ConfigurationStartIndication
// -----------------------------------------------------------------------------
//
void CBtsacConnected::GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID)
	{
    TRACE_INFO((_L("CBtsacConnected::GAVDP_ConfigurationStartIndication() LocalSEID: %d, RemoteSEID: %d"), aLocalSEID.SEID(), aRemoteSEID.SEID()))
    StopTimer();
	Parent().InitializeSEPManager();
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacConfiguring::NewL(Parent(), aLocalSEID, aRemoteSEID)));
	}

// -----------------------------------------------------------------------------
// CBtsacConnected::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::RequestCompletedL(CBtsacActive& aActive)
    {
	TRACE_FUNC
	switch(aActive.RequestId())
		{
		case KRequestIdTimer:
			{
			if(!iBearersQuery)
				{				
			// Go to listening state, gavdp will be shutdown in listening state
			CancelActionL(KErrCancel, EGavdpResetReasonNone);
				}
			else
				{
				// If bearers query timer has expired, lets handle it separately.		
				Parent().iGavdp->Cancel();
				TInt err = Parent().AbortStream();
				if(!err)
					{
					Parent().ChangeStateL(CBtsacAborting::NewL(Parent()));
					}
				else
					{
					CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
					}
				}
			break;
			}
		case KRequestIdWaitRemoteConfStart:
			{
			// Remote didn't configure us and we have open audio request from acc fw.
			// Let's configure the link and open audio by our selves.
			StartConfigurationL();
			break;
			}
		default:
			{
			TRACE_INFO((_L("CBtsacConnected::RequestCompletedL() Unknown request")))
			break;
			}				
		}
    }

// -----------------------------------------------------------------------------
// CBtsacConnected::CancelRequest
// -----------------------------------------------------------------------------
//
void CBtsacConnected::CancelRequest(CBtsacActive& aActive)
	{
	TRACE_FUNC
	(void)aActive;
	iTimer.Cancel();
	}    

// -----------------------------------------------------------------------------
// CBtsacConnected::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//
void CBtsacConnected::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
	StopTimer();
	switch (aError)
		{
		case KErrAvdtpRequestTimeout: // -18005
			{
			TRACE_INFO((_L("CBtsacConnected::HandleGavdpErrorL() Request TIMEOUT")))
			// Go to listening state, gavdp will be shutdown in listening state
			CancelActionL(KErrDisconnected, EGavdpResetReasonNone);
			break;
			}
			
		case KErrHCILinkDisconnection: // -6305
		case KErrDisconnected: // -36
			{
			TRACE_INFO((_L("CBtsacConnected::HandleGavdpErrorL() Signalling disconnected.")))
			// for both outgoing or incoming connection, if we have an error, 
			// this means there is disconnection
			CancelActionL(aError, EGavdpResetReasonGeneral);
			break;
			}	

		case (KErrAvdtpSignallingErrorBase - EAvdtpSEPInUse): // -18064 
			{
			// Remote SEP is in use.
			// Wait for a while, accessory might configure us. If we don't receive 
			// start ind during KOneSecondDelay, go to listening state.
    		StartTimer(KOneSecondDelay, KRequestIdTimer);
			break;
			}
		
		default:
		// KErrNotReady -18
		// KErrInUse -14
		// KErrCorrupt -20
		// (KErrAvdtpSignallingErrorBase - EAvdtpBadState) -18094
			{
			CancelActionL(KErrDisconnected, EGavdpResetReasonGeneral);
			}
		}
	}

//  End of File  
