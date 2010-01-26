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
* Description:  In this state, streaming is active.
*
*/




// INCLUDE FILES
#include "btsacStateStreaming.h"
#include "btsacStateListening.h"
#include "btsacStateSuspended.h"
#include "btsacStateConnected.h"
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
// CBtsacStreaming::NewL
// -----------------------------------------------------------------------------
//
CBtsacStreaming* CBtsacStreaming::NewL(CBTSAController& aParent, TAudioOpenedBy aAudioOpenedBy, TBool aStartCollision)
    {
    CBtsacStreaming* self = new( ELeave ) CBtsacStreaming(aParent, aAudioOpenedBy, aStartCollision);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtsacStreaming::CBtsacStreaming
// -----------------------------------------------------------------------------
//
CBtsacStreaming::CBtsacStreaming(CBTSAController& aParent, TAudioOpenedBy aAudioOpenedBy, TBool aStartCollision)
:   CBtsacState(aParent, EStateStreaming), iSuspending(EFalse), iAudioOpenedBy(aAudioOpenedBy),
	iStartCollision(aStartCollision)
    {
    }
  
// -----------------------------------------------------------------------------
// CBtsacStreaming::~CBtsacStreaming
// -----------------------------------------------------------------------------
//    
CBtsacStreaming::~CBtsacStreaming()
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacStreaming::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Streaming"))
	_LIT(KName, "CBtsacStateStreaming");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);
	if( iAudioOpenedBy == EAudioOpenedByAcc )
	 	{
 		Parent().AccessoryOpenedAudio(Parent().GetRemoteAddr());
	 	}
    }

// -----------------------------------------------------------------------------
// CBtsacStreaming::CancelActionL
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::CancelActionL(TInt aError)
	{
	TRACE_FUNC
	TInt ret = Parent().iStreamer->StopStream();
	if ( ret )
		{
		TRACE_INFO((_L("CBtsacStreaming::Cancel() iStreamer.StopStream() returned error(%d) !!!"), ret))
		}
	Parent().CompletePendingRequests(KCompleteAllReqs, aError);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, aError));
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::OpenAudioLinkL
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::OpenAudioLinkL(const TBTDevAddr& /*aAddr*/)
	{  
	TRACE_FUNC
	// It is not allowed to open audio in streaming state, audio is allready opened
	// by an accessory. Inform accessory fw about this by returning KErrInUse.
	Parent().CompletePendingRequests(KOpenAudioReq, KErrInUse);
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::CloseAudioLinkL
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::CloseAudioLinkL(const TBTDevAddr& /*aAddr*/)
	{  
	TRACE_FUNC
	iStartCollision = EFalse;
	TInt ret = Parent().iStreamer->StopStream();
	if ( ret )
		{
		TRACE_INFO((_L("CBtsacStreaming::CloseAudio() iStreamer.StopStream() returned error(%d) !!!"), ret))
		}
	TAvdtpSEPInfo SEPInfo;
	ret = Parent().iRemoteSEPs->GetInfo(Parent().GetSEPIndex(), SEPInfo); 
	if ( ret )
		{
		TRACE_INFO((_L("CBtsacStreaming::CloseAudio() Couldn't retrieve SEP Info !")))
		CancelActionL(KErrCancel); 
 		return;   
		}
	// Suspend audio
	TSEID remoteSEPid = SEPInfo.SEID(); 	
	TRACE_INFO((_L("CBtsacStreaming::CloseAudio() Suspending remote SEID(%d)"), remoteSEPid.SEID()))	
	iSuspending = ETrue;
	Parent().iGavdp->SuspendStreams(remoteSEPid);
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::CancelCloseAudioLink
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::CancelCloseAudioLink(const TBTDevAddr& /*aAddr*/)
	{
	TRACE_FUNC
	// For safety sake complete all pending requests
	Parent().CompletePendingRequests(KCompleteAllReqs, KErrNone);
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected)));
	}


// -----------------------------------------------------------------------------
// CBtsacStreaming::StartRecording
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::StartRecording()
	{  
	TRACE_FUNC	
	iStartCollision = EFalse;
	if ( Parent().iStreamer->StartStream(Parent().iStreamingSockets[0], Parent().iStreamer->FrameLength()) != KErrNone ) 
	 	{
		TRACE_INFO((_L("CBtsacStreaming::StartRecording() [ERR] Could not start streaming!")))	
		TInt err = Parent().AbortStream();
		if(err)
			{
			TRACE_INFO((_L("CBtsacStreaming::StartRecording() Couldn't abort stream.")))	
			}
		TRAP_IGNORE(Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected)));
	 	}
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::DisconnectL
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::DisconnectL()
	{
	TRACE_FUNC
	TInt ret = Parent().iStreamer->StopStream();
	if ( ret )
		{
		TRACE_INFO((_L("CBtsacStreaming::DisconnectL() StopStream() returned error: %d"), ret))	
		}
	Parent().CompletePendingRequests(KDisconnectReq, ret);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrNone));
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::GAVDP_SuspendStreamsConfirm
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::GAVDP_SuspendStreamsConfirm()
	{
	TRACE_FUNC
    // so remote supports suspend 
    iSuspending = EFalse;
	Parent().CompletePendingRequests(KCloseAudioReq, KErrNone);
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacSuspended::NewL(Parent())));
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::GAVDP_AbortIndication
// -----------------------------------------------------------------------------
//	
void CBtsacStreaming::GAVDP_AbortIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBtsacStreaming::GAVDP_AbortIndication() Local SEP Id: %d"), aSEID.SEID()))	
	(void)aSEID;
	TInt ret = Parent().iStreamer->StopStream();
	if ( ret )
		{
		TRACE_INFO((_L("CBtsacStreaming::Cancel() iStreamer.StopStream() returned error(%d) !!!"), ret))
		}
	
	// It is possible the remote disconnected while we have active close audio request.
	Parent().CompletePendingRequests(KCompleteAllReqs, KErrNone);
    TRAP_IGNORE(Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected)));
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::GAVDP_ReleaseIndication
// -----------------------------------------------------------------------------
//
void CBtsacStreaming::GAVDP_ReleaseIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBtsacStreaming::GAVDP_ReleaseIndication() aSEID: %d"), aSEID.SEID()))	
	(void)aSEID;
	if(!iSuspending)
		{		
		TInt ret = Parent().iStreamer->StopStream();
		if ( ret ) 
		 	{
			TRACE_INFO((_L("CBtsacStreaming::GAVDP_ReleaseIndication() [ERR: %d] Could not stop streaming!"), ret))	
		 	}
	 	Parent().AccessoryClosedAudio(Parent().GetRemoteAddr());
		}
	else
		{
		// We are trying to suspend the stream and remote end has sent release indication.
		// Stream is already stopped but we have pending close audio request, complete it.
		Parent().CompletePendingRequests(KCloseAudioReq, KErrNone);
		}
	Parent().CleanSockets();
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), EConnExists)));
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::GAVDP_SuspendIndication
// -----------------------------------------------------------------------------
//	
TInt CBtsacStreaming::GAVDP_SuspendIndication(TSEID aSEID)
	{
	TRACE_INFO((_L("CBtsacStreaming::GAVDP_SuspendIndication() aSEID: %d"), aSEID.SEID()))	
	(void)aSEID;
	if(!iSuspending)
		{		
		TInt ret = Parent().iStreamer->StopStream();
		if ( ret )
			{
			TRACE_INFO((_L("CBtsacStreaming::GAVDP_SuspendIndication() iStreamer.StopStream() returned error(%d) !!!"), ret))	
			}
		Parent().AccessorySuspendedAudio(Parent().GetRemoteAddr());
		TRAPD(err, Parent().ChangeStateL(CBtsacSuspended::NewL(Parent())));
		if(err)
			{
			return KErrNoMemory;
			}
		}
	return KErrNone;
	}

// -----------------------------------------------------------------------------
// CBtsacStreaming::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacStreaming::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
	TInt err = KErrNone;

	if(aError == (KErrAvdtpSignallingErrorBase - EAvdtpBadState) && iStartCollision)
		{
		// Start stream collision has happened (src and snk have sent start stream cmd simultaneously).
		// iStreamStartIndCollision flag is set in configured state to indicate possible collision
		// situations.
		// Error can be ignored and we can continue streaming.
		iStartCollision = EFalse;
		TRACE_INFO((_L("CBtsacStreaming::HandleGavdpErrorL() Ignore error, continue streaming.")))
		return;
		}

	if(iSuspending)
		{
		iSuspending = EFalse;
		if(aError == (KErrAvdtpSignallingErrorBase - EAvdtpBadState)) // -18094
			{
			// For some reason remote rejected our suspend command. According to the specification
			// it is acceptor's responsibilty to cope with this situation, so we can move to suspended
			// state and wait for acceptor's reaction.
	        Parent().CompletePendingRequests(KCloseAudioReq, KErrNone);
			Parent().ChangeStateL(CBtsacSuspended::NewL(Parent()));
		    return;
			}
		else if(aError == (KErrAvdtpSignallingErrorBase - EAvdtpNotSupportedCommand)) // -18070 
			{
			// remote doesn't support suspend so close audio 
			Parent().CleanSockets();
	        Parent().CompletePendingRequests(KCloseAudioReq, KErrNone);
			Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), EConnExists));
		    return;
			}
		}
	else
		{
		err = Parent().iStreamer->StopStream();
		if (err)
	 		{
			TRACE_INFO((_L("CBtsacStreaming::HandleGavdpErrorL() [error: %d] Could not stop streaming!"), err))
	 		}
		}
	
	switch (aError)
		{
		case KErrAvdtpRequestTimeout: // -18005
		case (KErrAvdtpSignallingErrorBase - EAvdtpBadState): // 18094
			{
			err = Parent().AbortStream();
			if(!err)
				{
				// Complete pending requests in Aborting state
				Parent().ChangeStateL(CBtsacAborting::NewL(Parent()));
				}
			else
				{
				Parent().CompletePendingRequests(KCompleteAllReqs, aError);
				Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected));
				}
			break;
			}
		case KErrHCILinkDisconnection: // -6305
		case KErrDisconnected: // -36
			{
			Parent().CompletePendingRequests(KCompleteAllReqs, aError);
			Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, aError));
			break;
			}
		default:
			{
			// Unknown error. For safety's sake let's disconnect a2dp link and inform btaudioman
			TRACE_INFO((_L("CBtsacStreaming::HandleGavdpErrorL() Unknown error, goes to listening")))
			Parent().CompletePendingRequests(KCompleteAllReqs, aError);
			Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected));
			break;
			}
		}
	}
	
//  End of File  
