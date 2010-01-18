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
* Description:  Contains implementation of CBTSACGavdp class which uses GAVDP API. 
*				 Two other classes CAVStreamEndPoint and CAVSEPManager are helper 
*				 classes to use GAVDP. This class also constructs CBTSACStreamerController
*				 class.
*
*/


// INCLUDE FILES
#include "btsacGavdp.h"   
#include "btsacGavdpObserver.h"
#include "debug.h"
#include "btsacStreamerController.h"
#include "btsacStreamEndPoint.h"
#include "btsacSEPManager.h"
#include "btsacdefs.h"

// CONSTANTS

// DATA TYPES

// A2DP codec-specific element definitions
// in bluetoothAV.h
using namespace SymbianBluetoothAV;   

// Subband codec-specific values
// in bluetoothAV.h
using namespace SymbianSBC;      

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTSACGavdp::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSACGavdp* CBTSACGavdp::NewL(MInternalGavdpUser* aObserver )
    {
    CBTSACGavdp* self = new (ELeave) CBTSACGavdp(aObserver);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

    
// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTSACGavdp::~CBTSACGavdp()
    {
	TRACE_FUNC
	StopSrc();
	iSockServ.Close(); 
	
	if(iA2dpSourceSDPrecordHandle != 0) // unregister sdp
		{
		if(iBTConnectionPtr)
			{			
	       	TRACE_INFO((_L("CBTSACGavdp::~CBTSACGavdp():Unregister SDP Settings")))       	
			TInt err = iBTConnectionPtr->DeleteSdpRecord(iA2dpSourceSDPrecordHandle);
			if(err)
				{
				TRACE_INFO((_L("CBTSACGavdp::~CBTSACGavdp():Unregister SDP Settings failed %d."), err))
				}
			}
		iA2dpSourceSDPrecordHandle = 0;   		
		}
	delete iBTConnectionPtr;
    }

// -----------------------------------------------------------------------------
// CBTSACGavdp::CBTSACGavdp
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CBTSACGavdp::CBTSACGavdp( MInternalGavdpUser* aObserver )
    : iObserver ( aObserver ), 
      iA2dpSourceSDPrecordHandle( 0 )
    {
    }
        
// -----------------------------------------------------------------------------
// CBTSACGavdp::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::ConstructL()
    {
    TRACE_FUNC
    TSessionPref pref;
    pref.iAddrFamily = KBTAddrFamily;
    pref.iProtocol = KAVDTP;
	User::LeaveIfError(iSockServ.Connect(pref, KESockDefaultMessageSlots));
	iBTConnectionPtr = CBTEngDiscovery::NewL();
    }

// -----------------------------------------------------------------------------
// CBTSACGavdp::StartSrc
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::StartSrc(CBTSACSEPManager& aLocalSEPs, TSBCCodecCapabilities& aSbc)
	{	
	TRACE_FUNC
	TInt err = KErrGeneral; 
	// register sdp
	// For hardware only, in ConstructL SDP is not ready and hence this causes a crash in emulator
	TRACE_INFO((_L("CBTSACGavdp::StartSrc():Register A2DP SDP settings.")))
	if(iBTConnectionPtr)
		{		
	    err = iBTConnectionPtr->RegisterSdpRecord( 
	           			        KAudioSourceUUID, // btsdp.h
	                   			0, 				// There is no channel number used for A2DP
	                   			iA2dpSourceSDPrecordHandle ); 
		}

	if( !err )
		{
		err = iGavdp.Open(*this, iSockServ);
		if ( !err )
	 		{
	 		err = RegisterSEPs(aLocalSEPs, aSbc);
			}
		}
		
    TRACE_INFO((_L("CBTSACGavdp::StartSrc() completed. Return: %d"), err))    
    return err;
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::RegisterSEPs
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::RegisterSEPs(CBTSACSEPManager& aLocalSEPs, TSBCCodecCapabilities& aSbc )
	{
	TRACE_FUNC

	// This hard-coded value would be recieved from Audio Subsystem later
	// Currently our implementation supports only SBC Encoding
	const TInt KNumSEPs = 1;
	
	TInt err;
	TFixedArray<TAvdtpSEPInfo, KNumSEPs> sepInfos;
	
	for (TInt i=0; i<KNumSEPs; i++)
		{
		TAvdtpSEPInfo& info = sepInfos[i];
		info.SetMediaType(i==2 ?  EAvdtpMediaTypeVideo : EAvdtpMediaTypeAudio);
		info.SetIsSink(i==2); // make nth one video sink
		info.SetSEID(TSEID(i+1,ETrue));

		TRACE_INFO((_L("CBTSACGavdp::RegisterSEPs() calling Gavdp...")))
		err = iGavdp.RegisterSEP(info);
		
		if ( err==KErrNone )
			{
			TRAP(err,aLocalSEPs.NewSEPL(info));
			if (err==KErrNone)
				{
				aLocalSEPs.SetState(info.SEID(), CBTSACStreamEndPoint::ERegistered, &info);
				}
			}
		else
			{
			TRACE_INFO((_L("CBTSACGavdp::RegisterSEPs() Registering SEP [SEID 1] - completed with error %d"), err))
			return err; 
			}
		} // for
		
	
	// add some caps, not to all though 
	err = iGavdp.BeginConfiguringLocalSEP( TSEID(1, ETrue) );
	if ( err )
		{
		TRACE_INFO((_L("CBTSACGavdp::RegisterSEPs() Begin config Local SEP [SEID 1] - completed with error %d"), err))
		return err;
		}
	
 	TAvdtpMediaTransportCapabilities media;
	err = iGavdp.AddSEPCapability(media);
	if ( err )
		{
		TRACE_INFO((_L("CBTSACGavdp::RegisterSEPs() Add configuration [Category %d] - completed with error %d"), media.Category(), err))
		return err;
		}
			
	err = iGavdp.AddSEPCapability(aSbc);
  	if ( err )	
  		{
  		TRACE_INFO((_L("CBTSACGavdp::RegisterSEPs() error %d"), err))
		return err;
  		}

	// that's ASYNC! Wait for callback before configuring more seps
	iGavdp.CommitSEPConfiguration();
	
	TRACE_INFO((_L("CBTSACGavdp::RegisterSEPs() Commit configuration [SEID 1]")))
	return err;
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::StopSrc
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::StopSrc()
	{
	TRACE_FUNC
	iGavdp.Close();
	TRACE_INFO((_L("CBTSACGavdp::StopSrc() completed.")))
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::AbortStreaming
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::AbortStreaming(TSEID& aRemoteSEPId)
	{
	TRACE_INFO((_L("CBTSACGavdp::AbortStreaming() RemoteSEPId: %d"), aRemoteSEPId.SEID()))
	iGavdp.AbortStream(aRemoteSEPId);	
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::Shutdown
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::Shutdown()
	{
	TRACE_FUNC
	return(iGavdp.Shutdown());
	}
	

// -----------------------------------------------------------------------------
// CBTSACGavdp::SendSecurityControl
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::SendSecurityControl()
	{
	
	TRACE_FUNC
	TInt seid = 1;
	TBuf8<KBTDevAddrReadable> secBuf(_L8("Danger!"));
	iGavdp.SendSecurityControl(TSEID(seid, EFalse), secBuf);	
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GetCapabilities
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::GetCapabilities(TSEID& aId)
	{
	TRACE_FUNC
	// we're getting all capabilities
	// we could register interest in only those capabilities that we can select
	// we keep it so in order to print out all capabilities in GAVDP_SEPCapability
	TAvdtpServiceCategories caps;
	caps.SetCapability(EAllServiceCategories);
	iGavdp.GetRemoteSEPCapabilities(aId, caps);
	}	

// -----------------------------------------------------------------------------
// CBTSACGavdp::Connect
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::Connect( const TBTDevAddr& aAddr )
	{
	TRACE_FUNC
	RProperty property;
	TBuf8<KBTDevAddrSize> addr;
	TInt err = property.Get(KPropertyUidBluetoothCategory,
	KPropertyKeyBluetoothLocalDeviceAddress, addr);

	if ((err) || (addr.Length()!=KBTDevAddrSize))
		{
		TRACE_INFO((_L("CBTSACGavdp::Connect() P&S: ERROR retrieving local address")))
		}
	else
		{
		TBTDevAddr localAddr(addr);
		TRACE_INFO_SEG(
        {TBuf<KBTDevAddrReadable> bufLocal; TBuf<KBTDevAddrReadable> bufRemote; localAddr.GetReadable(bufLocal); aAddr.GetReadable(bufRemote);
         Trace(_L("CBTSACGavdp::Connect() Local address = 0x%S, Using Remote Addr = 0x%S"), &bufLocal, &bufRemote);})		
		}
	iGavdp.Connect(aAddr);
	TRACE_INFO((_L("CBTSACGavdp::Connect() completed")))
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::Listen
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::Listen()
	{
	TRACE_FUNC
	return iGavdp.Listen();
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::Cancel
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::Cancel()
	{
	TRACE_FUNC
	iGavdp.Cancel();
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::CreateBearers
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::CreateBearers(TSEID& aRemoteSEPId)
	{
	TRACE_FUNC
	TInt ret; 
	ret = iGavdp.CreateBearerSockets(aRemoteSEPId, EFalse, EFalse);
	TRACE_INFO((_L("CBTSACGavdp::CreateBearers() Asking for Bearers..(sync_result) %d."), ret))
	return ret; 
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::DiscoverSEPs
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::DiscoverSEPs()
	{
	TRACE_FUNC
	iGavdp.DiscoverRemoteSEPs();		
	}	

// -----------------------------------------------------------------------------
// CBTSACGavdp::Close
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::Close()
	{
	TRACE_FUNC
	iGavdp.Close();
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::Open
// -----------------------------------------------------------------------------
//
TInt CBTSACGavdp::Open()
	{
	TRACE_FUNC
	return( iGavdp.Open(*this, iSockServ) );
	}
	

///////////////////// Call Backs from GAVDP ///////////////////

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SEPCapability
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Capability = %d"), aCapability->Category()))

	switch (aCapability->Category())
		{
		case EServiceCategoryMediaCodec:
			{
			// print name of codec
			TAvdtpMediaCodecCapabilities& codecCaps = static_cast<TAvdtpMediaCodecCapabilities&>(*aCapability);
			switch ( codecCaps.MediaType() )
				{
				case EAvdtpMediaTypeAudio:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Audio:")))
					break;
				case EAvdtpMediaTypeVideo:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Video:")))
					break;
				case EAvdtpMediaTypeMultimedia:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Multimedia:")))
					break;
				}
			switch (codecCaps.MediaCodecType())
				{
				case EAudioCodecSBC:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() SBC")))
					break;
				case EAudioCodecMPEG12Audio:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() MPEG1,2 Audio")))
					break;
				case EAudioCodecMPEG24AAC:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() MPEG 2,4 AAC")))
					break;
				case EAudioCodecATRAC:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() ATRAC")))
					break;

				default:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Unknown codec")))
					break;
				}
			break;
			} // case
			
		 case EServiceCategoryRecovery:
			{
			TAvdtpRecoveryCapabilities* RecCaps = static_cast<TAvdtpRecoveryCapabilities*>(aCapability);
			switch ( RecCaps->RecoveryType() )
				{
				case EForbiddenRecovery:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Forbidden Recovery")))
					break;
				case ERFC2733Recovery:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() RFC2733 Recovery")))
					break;
				default:
					TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Unknown Recovery")))
					break;
				}
			break;
			} // case
				
		 case EServiceCategoryReporting:
			{
			TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Capable of Reporting")))
			break;
			}//case
		
		 case EServiceCategoryMediaTransport:
			{
			TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapability() Capable of a Media Transport")))
			break;
			}
		
		} // switch Category
		
	// state handles the call-back
	iObserver->GAVDP_SEPCapability(aCapability);

	}
				
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SEPDiscoveryComplete
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_SEPDiscoveryComplete()
	{
	TRACE_FUNC
	// state handles the call-back
	iObserver->GAVDP_SEPDiscoveryComplete();
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SEPCapabilityComplete
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_SEPCapabilityComplete()
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_SEPCapabilityComplete() GAVDP:SEP has no more capabilities.")))
	iObserver->GAVDP_SEPCapabilityComplete();
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_ConnectConfirm
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::GAVDP_ConnectConfirm(const TBTDevAddr& aAddr)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConnectConfirm() Signalling channel connected = GAVDP ready")))
	// inform observer
	iObserver->GAVDP_ConnectConfirm(aAddr);
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SEPDiscovered
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEPInfo)
	{
	TRACE_FUNC
	// state handles the call-back
	iObserver->GAVDP_SEPDiscovered(aSEPInfo);
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_AbortStreamConfirm
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::GAVDP_AbortStreamConfirm()
	{
	TRACE_FUNC
	 // state handles the call-back
	iObserver->GAVDP_AbortStreamConfirm();	
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SecurityControlConfirm
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_SecurityControlConfirm(const TDesC8& aResponseData)
	{
	TRACE_FUNC
	// state handles the call-back
	iObserver->GAVDP_SecurityControlConfirm(aResponseData);
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_StartStreamsConfirm
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_StartStreamsConfirm()
	{
	TRACE_FUNC
	iObserver->GAVDP_StartStreamsConfirm();
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SuspendStreamsConfirm
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_SuspendStreamsConfirm()
	{
	TRACE_FUNC
	// state handles the call-back
	iObserver->GAVDP_SuspendStreamsConfirm();	
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_ConfigurationConfirm
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_ConfigurationConfirm()
	{
	TRACE_FUNC
	// state handles the call-back
	iObserver->GAVDP_ConfigurationConfirm();
	}
	

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_Error
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_Error(TInt aError, const TDesC8& aErrorData)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_Error() **Error**: %d:"), aError))
	iObserver->GAVDP_Error(aError, aErrorData);
	}
	

// passive gubbins
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_ConfigurationStartIndication
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID)
	{
	// ah - remote is attempting to confuigure us
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigurationStartIndication() Remote SEP [SEID %d] is configuring Local SEP [SEID %d]"), aRemoteSEID.SEID(), aLocalSEID.SEID()))
	// state handles the call-back
	iObserver->GAVDP_ConfigurationStartIndication(aLocalSEID, aRemoteSEID);
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_ConfigurationIndication
// -----------------------------------------------------------------------------
//
TInt CBTSACGavdp::GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability)
	{
	// the new capability proposed by remote
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigurationIndication() Configuration proposed: category %d"), aCapability->Category()))
	// state handles the call-back
	return (iObserver->GAVDP_ConfigurationIndication(aCapability));	
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_ConfigurationEndIndication
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::GAVDP_ConfigurationEndIndication()
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigurationEndIndication() Remote configuration proposals now finished")))
	// state handles the call-back
	TInt ret = iObserver->GAVDP_ConfigurationEndIndication();
	return (ret);	
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_StartIndication
// -----------------------------------------------------------------------------
//
TInt CBTSACGavdp::GAVDP_StartIndication(TSEID aLocalSEID)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_StartIndication() Start indication for Local SEID %d"), aLocalSEID.SEID()))
	// state handles the call-back
	return (iObserver->GAVDP_StartIndication(aLocalSEID));	
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_AbortIndication
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_AbortIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_AbortIndication() Stream %d ABORTED by peer"), aSEID.SEID()))
	// state handles the call-back
	iObserver->GAVDP_AbortIndication(aSEID);	
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_ReleaseIndication
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_ReleaseIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ReleaseIndication() Stream %d RELEASED by peer"), aSEID.SEID()))
	// state handles the call-back
	iObserver->GAVDP_ReleaseIndication(aSEID);	
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SuspendIndication
// -----------------------------------------------------------------------------
//	
TInt CBTSACGavdp::GAVDP_SuspendIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_SuspendIndication() Suspend indication for seid %d!"), aSEID.SEID()))
	// state handles the call-back
	return iObserver->GAVDP_SuspendIndication(aSEID);	
	}
	
// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_SecurityControlIndication
// -----------------------------------------------------------------------------
//
TInt CBTSACGavdp::GAVDP_SecurityControlIndication(TSEID aSEID, TDes8& aSecurityData)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_SecurityControlIndication() Got security control data length %d for SEID %d"), aSEID.SEID(), aSecurityData.Length()))
	// state handles the call-back
	return (iObserver->GAVDP_SecurityControlIndication(aSEID, aSecurityData));	
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::GAVDP_BearerReady
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::GAVDP_BearerReady(RSocket aNewSocket, const TAvdtpSockAddr& aAddr)
	{
	// wrap socket with active wrapper...
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_BearerReady() Got a bearer, for session %d"), aAddr.Session()))

	// AV sockets don't foward opts yet so use addr version
	RBTPhysicalLinkAdapter phy;
	TBTDevAddr RemoteBDAddr = aAddr.BTAddr(); 
	TInt err = phy.Open(iSockServ, RemoteBDAddr);
	TUint16 packets = EAnyACLPacket;
	err = phy.RequestChangeSupportedPacketTypes(packets);
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_BearerReady() Modified PHY, result %d"), err))

	// state handles the call-back
	iObserver->GAVDP_BearerReady(aNewSocket, aAddr);
	}
///////////////////// Call Backs from GAVDP End.///////////////////	


// -----------------------------------------------------------------------------
// CBTSACGavdp::StartStreams
// -----------------------------------------------------------------------------
//
void CBTSACGavdp::StartStreams(TSEID& aRemoteSEPId)
	{
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_StartStreams() Starting remoteSEP %d streaming..."), aRemoteSEPId.SEID()))
	iGavdp.StartStream(aRemoteSEPId);
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::SuspendStreams
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::SuspendStreams(TSEID& aRemoteSEPId)
	{
	TRACE_INFO((_L("CBTSACGavdp::SuspendStreams() Suspending remoteSEP %d streaming..."), aRemoteSEPId.SEID()))
	iGavdp.SuspendStream(aRemoteSEPId);
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::ConfigureSEP
// -----------------------------------------------------------------------------
//
TInt CBTSACGavdp::ConfigureSEP(TSEID aLocalSEPId, TSEID aRemoteSEPId, 
			 					TSBCCodecCapabilities& aSBCCaps, TAvdtpMediaTransportCapabilities& aMedTranCaps  )
	{
	TRACE_INFO((_L("CBTSACGavdp::ConfigureSEP() Local SEP Id: %d, Remote SEP Id: %d"), aLocalSEPId.SEID(), aRemoteSEPId.SEID()))
	
	TInt res;
	res = iGavdp.BeginConfiguringRemoteSEP(aRemoteSEPId, aLocalSEPId);
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigureSEP() Begin configuring remote SEP returned %d"), res))
	
	if (res) 
		{
		return res; 
		}
		
	res = iGavdp.AddSEPCapability(aMedTranCaps);
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigureSEP() AddSEPCapability returned %d"), res))
	
	if ( res )		
		{
		return res;
		}

	res = iGavdp.AddSEPCapability(aSBCCaps);
	if ( !res )
		{
		TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigureSEP() Commiting configuration...")))
		iGavdp.CommitSEPConfiguration();	
		}
	TRACE_INFO((_L("CBTSACGavdp::GAVDP_ConfigureSEP() completed: %d"), res))
	return res; 
	}

// -----------------------------------------------------------------------------
// CBTSACGavdp::RegisterObserver
// -----------------------------------------------------------------------------
//	
void CBTSACGavdp::RegisterObserver(MInternalGavdpUser* aObserver, const TDesC& aName)
	{
	TRACE_INFO((_L("CBTSACGavdp::RegisterObserver(%S)"), &aName))
	(void) aName;
	iObserver = aObserver;
	}
		
//  End of File      
