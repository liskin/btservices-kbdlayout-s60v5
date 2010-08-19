/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  In this state, a stereo audio accessory is configuring us.
*
*/




// INCLUDE FILES
#include "btsacStateConfiguring.h"
#include "btsacStateConfigured.h"
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


// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacConfiguring::NewL
// -----------------------------------------------------------------------------
//
CBtsacConfiguring* CBtsacConfiguring::NewL(CBTSAController& aParent, TSEID aLocalSEID, TSEID aRemoteSEID)
    {
   	CBtsacConfiguring* self = new( ELeave ) CBtsacConfiguring(aParent, aLocalSEID, aRemoteSEID);
    return self;
   	}

// -----------------------------------------------------------------------------
// CBtsacConfiguring::CBtsacConfiguring
// -----------------------------------------------------------------------------
//
CBtsacConfiguring::CBtsacConfiguring(CBTSAController& aParent, TSEID aLocalSEID, TSEID aRemoteSEID)
:   CBtsacState(aParent, EStateConfiguring), iLocalSEID(aLocalSEID), iRemoteSEID(aRemoteSEID),
	iSEPFound(EFalse), iRemoteSEPIndex(0), iAudioOpenedBy(EAudioOpenedByNone)
    {
    }
  
// -----------------------------------------------------------------------------
// CBtsacConfiguring::~CBtsacConfiguring
// -----------------------------------------------------------------------------
//    
CBtsacConfiguring::~CBtsacConfiguring()
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacConfiguring::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacConfiguring::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Configuring"))
	_LIT(KName, "CBtsacStateConfiguring");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);
	
	if ( iLocalSEID.SEID() != 1)	// we have only one local SEP, SBC
		{
		TRACE_INFO((_L("CBtsacConfiguring::EnterL() Wrong Local SEP being configured !!!")))
		}
	
	// we have registered only one local SBC SEP, so aRemoteSEID should be a SBC Audio SEP, 
	// atleast we assume that. So stash it as SBC Audio SEP
	TAvdtpSEPInfo info = TAvdtpSEPInfo();
	info.SetIsSink(ETrue); 
	info.SetMediaType(EAvdtpMediaTypeAudio);
	info.SetSEID(iRemoteSEID);
	TRAPD(err, Parent().iRemoteSEPs->NewSEPL(info));
	if (!err)
		{
		Parent().iRemoteSEPs->SetState(info.SEID(), CBTSACStreamEndPoint::EDiscoveredRemote, &info);
		}
	else // internal problem
		{
  		CancelActionL(KErrCancel);
		}
    }

// -----------------------------------------------------------------------------
// CBtsacConfiguring::CancelActionL
// -----------------------------------------------------------------------------
//
void CBtsacConfiguring::CancelActionL(TInt aError)
    {
    TRACE_FUNC
	Parent().CompletePendingRequests((KConnectReq | KOpenAudioReq), aError);
    Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EResetGavdp, aError));
    }

// -----------------------------------------------------------------------------
// CBtsacConfiguring::CancelConnectL
// -----------------------------------------------------------------------------
//
void CBtsacConfiguring::CancelConnectL()
    {
    TRACE_FUNC
	CancelActionL(KErrCancel);
    }

// -----------------------------------------------------------------------------
// CBtsacConfiguring::OpenAudioLinkL
// -----------------------------------------------------------------------------
//    
void CBtsacConfiguring::OpenAudioLinkL(const TBTDevAddr& /*aAddr*/)
	{  
	TRACE_FUNC
	// Handle open audio completion later, after configuration is ready (completion will happen
	// in CBtsacConfigured state after we receive GAVDP_StartIndication).
	}
	
// -----------------------------------------------------------------------------
// CBtsacConfiguring::DisconnectL
// -----------------------------------------------------------------------------
//
void CBtsacConfiguring::DisconnectL()	
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KDisconnectReq, KErrNone);
	CancelActionL(KErrCancel);
	}	

// -----------------------------------------------------------------------------
// CBtsacConfiguring::GAVDP_BearerReady
// -----------------------------------------------------------------------------
//
void CBtsacConfiguring::GAVDP_BearerReady(RSocket aNewSocket, const TAvdtpSockAddr& aAddr)
	{
	TRACE_INFO((_L("CBtsacConfiguring::GAVDP_BearerReady() SEID: %d"), aAddr.SEID().SEID()))
	(void) aAddr.SEID(); 
	Parent().NewAccessory(Parent().GetRemoteAddr());
	
	// If there is pending connect request, complete it.
	Parent().CompletePendingRequests(KConnectReq, KErrNone);
	
	// only one socket in this implementation at the moment, 
	// as we dont have recovery and reporting yet
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacConfigured::NewL(Parent(), aNewSocket, iAudioOpenedBy, EStreamConfiguredBySink)));
	}

// -----------------------------------------------------------------------------
// CBtsacConfiguring::GAVDP_ConfigurationIndication
// -----------------------------------------------------------------------------
//
TInt CBtsacConfiguring::GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability)
	{
	TRACE_FUNC
	Parent().iRemoteSEPs->StoreCaps(iRemoteSEPIndex, aCapability);
	if ( aCapability->Category() == EServiceCategoryMediaCodec )
		{
		TAvdtpMediaCodecCapabilities& codecCaps = static_cast<TAvdtpMediaCodecCapabilities&>(*aCapability);
		if ( codecCaps.MediaCodecType() == EAudioCodecSBC ) // found SEP that we are interested in
			{
			TRACE_INFO((_L("CBtsacConfiguring::GAVDP_SEPCapability() Found SBC Audio Sink SEP !!!")))
			iSEPFound = ETrue;
			Parent().SetSEPIndex(iRemoteSEPIndex);
			}
		}
	return KErrNone;
	}
	
// -----------------------------------------------------------------------------
// CBtsacConfiguring::GAVDP_ConfigurationEndIndication
// -----------------------------------------------------------------------------
//	
TInt CBtsacConfiguring::GAVDP_ConfigurationEndIndication()
	{
	TRACE_FUNC
	if ( iSEPFound ) // proposed SEP has SBC (and media transport!) caps
		{
		RPointerArray<TAvdtpServiceCapability> SEPCapabilities;
	    if ((Parent().iRemoteSEPs->GetCaps(Parent().GetSEPIndex(), SEPCapabilities)) )
			{
			TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Couldn't retrieve Capabilities !")))
		   	SEPCapabilities.Close();
		   	TRAPD(err, CancelActionL(KErrCancel));
		   	if(err)
		   		{
		   		return KErrNoMemory;
		   		}
			return KErrGeneral; 
			}
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Retrieve %d capabilities"), SEPCapabilities.Count()))
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
			    TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() SBC Caps retrieved.")))
				}
			else if ( cat == EServiceCategoryMediaTransport )
				{
				MedTransCaps = static_cast<TAvdtpMediaTransportCapabilities&>(*cap);
				TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Media Transport Caps retrieved.")))
				}
	   		}
	   	SEPCapabilities.Close();	
		
		// Check if headset's capabilities suits us			
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory Sampling Frequencies: %d"), SBCCaps.SamplingFrequencies()))
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory Channel modes: %d"), SBCCaps.ChannelModes()))
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory Blocks: %d"), SBCCaps.BlockLengths()))
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory SubBands: %d"), SBCCaps.Subbands()))
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory Alloc method: %d"), SBCCaps.AllocationMethods()))
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory Max bitpool: %d"), SBCCaps.MaxBitpoolValue()))
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Accessory Min bitpool: %d"), SBCCaps.MinBitpoolValue()))
		if (Parent().iStreamer->ConfigureSEP(SBCCaps) )
			{
	   		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() Streamer couldn't configure SEP !")))
	        TRAPD(err, CancelActionL(KErrCancel));
	        if(err)
	        	{
	        	return KErrNoMemory;
	        	}
	        return KErrGeneral;   // capabilites doesn't suit us
			}
		// Everyting has been done on behalf of us, let's wait GAVDP_BearerReady indication.
		TRACE_INFO((_L("CBtsacConfiguring::GAVDP_ConfigurationEndIndication() completed")))
		return KErrNone;
		}
	else 
	 	{
	 	return KErrNotFound; // remote is missing SBC (or media transport!) caps in SEP it proposed
	 	}
	}

// -------------------------------------------------------------
// CBtsacConfiguring::GAVDP_StartIndication
// -------------------------------------------------------------
//
TInt CBtsacConfiguring::GAVDP_StartIndication(TSEID aLocalSEID)
	{
	TRACE_INFO((_L("CBtsacConfiguring::GAVDP_StartIndication(), LocalSEID:%d"), aLocalSEID.SEID()))
	(void)aLocalSEID;
	
	// accessory has send this indication right after GAVDP_ConfigurationEndIndication
	// but we must be waiting for Gavdp_BearerReady. In order to solve this, just flag 
	// our indication that an open audio request exists
	// CBtsacConfigured state can use this indication to start audio automatically then later
	
	// If we have already audio open request pending (from accessory FW), let keep it that way.
	// We have to complete open audio request later
	if(!Parent().IsOpenAudioReqFromAccFWPending())
		{		
		iAudioOpenedBy = EAudioOpenedByAcc;
		}
	return KErrNone;
	}
    
// -----------------------------------------------------------------------------
// CBtsacConfiguring::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacConfiguring::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
	switch(aError)
		{
		case KErrAvdtpRequestTimeout: // -18005
			{
			TRACE_INFO((_L("CBtsacConfiguring::HandleGavdpErrorL() Request TIMEOUT")))
			// Go to listening state, gavdp will be shutdown in listening state
			CancelActionL(KErrDisconnected);
			break;
			}
			
		case KErrHCILinkDisconnection: // -6305
		case KErrDisconnected: // -36
			{
			TRACE_INFO((_L("CBtsacConfiguring::HandleGavdpErrorL() Signalling disconnected.")))
			// for both outgoing or incoming connection, if we have an error, 
			// this means there is disconnection
			CancelActionL(aError);
			break;
			}
		
		default:			
		//case KErrNotReady: // -18
		//case KErrInUse: // -14
			{
			CancelActionL(KErrDisconnected);
			break;
			}
		}
	}
    
//  End of File  
