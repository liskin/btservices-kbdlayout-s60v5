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
* Description:  In this state, BTSAC is waiting for open audio request.
*
*/




// INCLUDE FILES
#include "btsacStateSuspended.h"
#include "btsacStateListening.h"
#include "btsacStateStreaming.h"
#include "btsacStateConnected.h"
#include "btsacStateAborting.h"
#include "btsacactive.h"
#include "btsacSEPManager.h"
#include "btsacStreamerController.h"
#include "btsacGavdp.h"
#include "debug.h"
#include <e32property.h>
#include "btaudioremconpskeys.h"

// A2DP codec-specific element definitions
// in bluetoothAV.h
using namespace SymbianBluetoothAV;


// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacSuspended::NewL
// -----------------------------------------------------------------------------
//
CBtsacSuspended* CBtsacSuspended::NewL(CBTSAController& aParent)
    {
    CBtsacSuspended* self = new( ELeave ) CBtsacSuspended(aParent);
    return self;
    }    

// -----------------------------------------------------------------------------
// CBtsacSuspended::CBtsacSuspended
// -----------------------------------------------------------------------------
//
CBtsacSuspended::CBtsacSuspended(CBTSAController& aParent)
:   CBtsacState(aParent, EStateSuspended), iOpenAudioReq(EFalse)
    {
    }
  
// -----------------------------------------------------------------------------
// CBtsacSuspended::~CBtsacSuspended
// -----------------------------------------------------------------------------
//    
CBtsacSuspended::~CBtsacSuspended()
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacSuspended::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacSuspended::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Suspended"))
	_LIT(KName, "CBtsacStateSuspended");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);
    }

// -----------------------------------------------------------------------------
// CBtsacSuspended::CancelActionL
// -----------------------------------------------------------------------------
//
void CBtsacSuspended::CancelActionL(TInt aError)
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KOpenAudioReq, aError);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, aError));
	}
    
// -----------------------------------------------------------------------------
// CBtsacSuspended::OpenAudioLinkL
// -----------------------------------------------------------------------------
//
void CBtsacSuspended::OpenAudioLinkL(const TBTDevAddr& aAddr)
	{  
	TRACE_FUNC
	TAvdtpSEPInfo SEPInfo;
	if (Parent().iRemoteSEPs->GetInfo(Parent().GetSEPIndex(), SEPInfo) || (aAddr != Parent().GetRemoteAddr()))
		{
		TRACE_INFO((_L("CBtsacSuspended::OpenAudioLinkL() Couldn't retrieve SEP Info!")))
		Parent().CompletePendingRequests(KOpenAudioReq, KErrGeneral);
       	return;
		}
	iOpenAudioReq = ETrue;
	Parent().SetRemoteAddr(aAddr);
	TSEID remoteSEPid = SEPInfo.SEID();
	Parent().iGavdp->StartStreams(remoteSEPid);
	}

// -----------------------------------------------------------------------------
// CBtsacSuspended::CancelOpenAudioL
// -----------------------------------------------------------------------------
//
void CBtsacSuspended::CancelOpenAudioL(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
	CancelActionL(KErrCancel);
    }
    
// -----------------------------------------------------------------------------
// CBtsacSuspended::DisconnectL
// -----------------------------------------------------------------------------
//
void CBtsacSuspended::DisconnectL()
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KDisconnectReq, KErrNone);
	CancelActionL(KErrCancel);
	}
	
// -----------------------------------------------------------------------------
// CBtsacSuspended::GAVDP_StartStreamsConfirm()
// -----------------------------------------------------------------------------
//    
void CBtsacSuspended::GAVDP_StartStreamsConfirm()
	{
	TRACE_FUNC
	if ( Parent().iStreamer->StartStream(Parent().iStreamingSockets[0], Parent().iStreamer->FrameLength()) != KErrNone ) 
	 	{
		TRACE_INFO((_L("CBtsacSuspended::GAVDP_StartStreamsConfirm() [ERR] Could not start streaming!")))
		TInt err = Parent().AbortStream();
		if(err)
			{
			TRACE_INFO((_L("CBtsacSuspended::GAVDP_StartStreamsConfirm() Couldn't abort stream.")))
			}
		TRAP_IGNORE(CancelActionL(KErrDisconnected));
	 	}
	 else
	 	{
	 	Parent().CompletePendingRequests(KOpenAudioReq, KErrNone);
		TRAP_IGNORE(Parent().ChangeStateL(CBtsacStreaming::NewL(Parent(), EAudioOpenedByAFW, EFalse)));
	 	}
	}
	
// -----------------------------------------------------------------------------
// CBtsacSuspended::GAVDP_StartIndication
// -----------------------------------------------------------------------------
//      
TInt CBtsacSuspended::GAVDP_StartIndication(TSEID aLocalSEID)
	{
	TRACE_INFO((_L("CBtsacSuspended::GAVDP_StartIndication() aLocalSEID: %d"), aLocalSEID.SEID()))	
	(void)aLocalSEID;
	// Accessory has send this indication.
	// TStateStreaming state can use this indication to start audio automatically then later.
	RProperty::Set(KBTAudioRemCon, KBTAudioPlayerControl, EBTAudioResumePlayer);
	TRAPD(err, Parent().ChangeStateL(CBtsacStreaming::NewL(Parent(), EAudioOpenedByAcc, EFalse)));
	if(err)
		{
		return KErrNoMemory;
		}
	return KErrNone;
	}

// -----------------------------------------------------------------------------
// CBtsacSuspended::GAVDP_ReleaseIndication
// -----------------------------------------------------------------------------
//
void CBtsacSuspended::GAVDP_ReleaseIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBtsacSuspended::GAVDP_ReleaseIndication() aSEID:%d"), aSEID.SEID()))	
	(void)aSEID; 
	Parent().AccessoryClosedAudio(Parent().GetRemoteAddr());
	Parent().CleanSockets();
	Parent().CompletePendingRequests(KOpenAudioReq, KErrCancel);
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), EConnExists)));
	}

// -----------------------------------------------------------------------------
// CBtsacSuspended::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacSuspended::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
	TInt err = KErrNone;
	
	switch (aError)
		{
		case KErrAvdtpRequestTimeout: // -18005
			{
			TRACE_INFO((_L("CBtsacSuspended::HandleGavdpErrorL() Request TIMEOUT")))
			err = Parent().AbortStream();
			if(!err)
				{
				// Complete pending request(s) in Aborting state
				Parent().ChangeStateL(CBtsacAborting::NewL(Parent()));
				}
			else
				{
				CancelActionL(KErrDisconnected);
				}
			break;
			}
		case (KErrAvdtpSignallingErrorBase - EAvdtpBadState): // -18094
			{
			if(iOpenAudioReq)
				{
				err = Parent().AbortStream();
				if(!err)
					{
					// Complete Audio request in Aborting state
					Parent().ChangeStateL(CBtsacAborting::NewL(Parent()));
					}
				else
					{
					CancelActionL(KErrDisconnected);
					}
				}
			else
				{					
				// For some reason remote rejected our suspend command (which was initiated in streaming state).
				// According to the specification it is acceptor's responsibilty to cope with this situation,
				// so we can stay in suspended state.
				// For safety's sake complete close audio request if any.
		        Parent().CompletePendingRequests(KCloseAudioReq, KErrNone);
				}
		    break;
			}
		case KErrHCILinkDisconnection: // -6305
		case KErrDisconnected: // -36
			{
			TRACE_INFO((_L("CBtsacSuspended::HandleGavdpErrorL() Signalling disconnected.")))
			CancelActionL(aError);
			break;
			}
		default:
			{
			// Unknown error. For safety's sake let's disconnect a2dp link and inform btaudioman
			TRACE_INFO((_L("CBtsacSuspended::HandleGavdpErrorL() Unknown error, goes to listening")))
			CancelActionL(KErrDisconnected);
			break;
			}
		}
	}

//  End of File  
