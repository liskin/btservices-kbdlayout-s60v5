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
* Description:  In this state, avdtp link is ready to be used and btsac opens
*				 audio automatically if there is request pending otherwise
*				 it will wait open audio request.
*
*/




// INCLUDE FILES
#include "btsacStateConfigured.h"
#include "btsacStateListening.h"
#include "btsacStateStreaming.h"
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

const TInt KStartIndicationDelay = 1000000; // 1 sec

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacConfigured::NewL
// -----------------------------------------------------------------------------
//
CBtsacConfigured* CBtsacConfigured::NewL(CBTSAController& aParent, RSocket aNewSocket,
	TAudioOpenedBy aAudioOpenedBy, TStreamConfiguredBy aStreamConfiguredBy)
    {
   	CBtsacConfigured* self = new( ELeave ) CBtsacConfigured(aParent, aNewSocket, aAudioOpenedBy, aStreamConfiguredBy);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
   	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::CBtsacConfigured
// -----------------------------------------------------------------------------
//
CBtsacConfigured::CBtsacConfigured(CBTSAController& aParent, RSocket aNewSocket,
	TAudioOpenedBy aAudioOpenedBy, TStreamConfiguredBy aStreamConfiguredBy)
:   CBtsacState(aParent, EStateConfigured), iAudioOpenedBy(aAudioOpenedBy),
	iStreamConfiguredBy(aStreamConfiguredBy), iStartStreamStatus(EStartNone)
    {
    TRACE_ASSERT(Parent().iStreamingSockets.Count() == 0, EBTPanicSocketExists)
    TRACE_INFO((_L("[SOCKET] created.")))
    Parent().iStreamingSockets.Append(aNewSocket);
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::ConstructL
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::ConstructL()
	{
	iTimerActive = CBtsacActive::NewL(*this, CActive::EPriorityStandard, KRequestIdTimer);
	iTimer.CreateLocal();
	}
  
// -----------------------------------------------------------------------------
// CBtsacConfigured::~CBtsacConfigured
// -----------------------------------------------------------------------------
//    
CBtsacConfigured::~CBtsacConfigured()
    {
    TRACE_FUNC
    delete iTimerActive;
    iTimerActive = NULL;
    iTimer.Close();
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Configured"))
	_LIT(KName, "CBtsacStateConfigured");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);

  	// ** OPEN AUDIO automatically **
  	if(iStreamConfiguredBy == EStreamConfiguredBySrc)
	  	{
	  	if(Parent().IsOpenAudioReqFromAccFWPending())
		  	{
		  	TInt err;
	  		TRAP(err, OpenAudioLinkL(Parent().GetRemoteAddr()));
	  		if( err )
	  			{
	  			Parent().CompletePendingRequests(KOpenAudioReq, err);
	  			}
		  	}
	  	}
	 else // Stream configured by sink
	 	{
	 	if(iAudioOpenedBy == EAudioOpenedByAcc)
		 	{
		 	GAVDP_StartIndication(TSEID(1, ETrue));
		 	}
 		else if(Parent().IsOpenAudioReqFromAccFWPending())
	 		{
	 		// Start timer to protect such a situation that we will not receive StartIndication from remote end.
  			StartTimer(KStartIndicationDelay);
	 		}
	 	}	  
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::StartTimer
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::StartTimer(TTimeIntervalMicroSeconds32 aTimeout)
	{
	TRACE_FUNC
	if (iTimerActive)
    	{
	    if(!iTimerActive->IsActive())
		    {	    	
	        iTimer.After(iTimerActive->iStatus, aTimeout);
	        iTimerActive->GoActive();
		    }
        }
    else
    	{
    	TRACE_INFO((_L("CBtsacConfigured::StartTimer, Timer Active doesn't exist.")))
    	}
	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::StopTimer
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::StopTimer()
	{
	TRACE_FUNC
	if (iTimerActive)
    	{
		iTimerActive->Cancel();
        }
	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::CancelActionL
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::CancelActionL(TInt aError)
    {
    TRACE_FUNC
	Parent().CompletePendingRequests(KOpenAudioReq, aError);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EResetGavdp, aError));
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::CancelConnectL
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::CancelConnectL()
    {
	TRACE_FUNC	
	CancelActionL(KErrCancel);
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::OpenAudioLinkL
// -----------------------------------------------------------------------------
//    
void CBtsacConfigured::OpenAudioLinkL(const TBTDevAddr& aAddr)
	{  
	TRACE_FUNC
	if((iStreamConfiguredBy == EStreamConfiguredBySrc) || (iAudioOpenedBy == EAudioOpenedByAFW))
		{		
		TAvdtpSEPInfo SEPInfo;
		iStartStreamStatus = EStartSendBySrc;
		if (Parent().iRemoteSEPs->GetInfo(Parent().GetSEPIndex(), SEPInfo) || (aAddr != Parent().GetRemoteAddr() ))
			{
			TRACE_INFO((_L("TStateConfigured::OpenAudio Couldn't retrieve SEP Info !")))
	       	Parent().CompletePendingRequests(KOpenAudioReq, KErrGeneral);
	       	return;
			}
		Parent().SetRemoteAddr(aAddr); 	
		TSEID remoteSEPid = SEPInfo.SEID(); 	
		Parent().iGavdp->StartStreams(remoteSEPid);
		}
	else
		{
		// Remote end (sink) has configured us. We can't call StartStreams. Complete audio req later,
		// after we have received start indication from the remote (sink).

		// Start timer to protect such a situation that we will not receive StartIndication from remote end.
		StartTimer(KStartIndicationDelay);
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::CancelOpenAudioLinkL()
// -----------------------------------------------------------------------------
//    
void CBtsacConfigured::CancelOpenAudioLinkL()
    {
    TRACE_FUNC
	CancelActionL(KErrCancel);
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::DisconnectL
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::DisconnectL()
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KDisconnectReq, KErrNone);
	// Cancel all other requests
	Parent().CompletePendingRequests(KCompleteAllReqs, KErrCancel);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EResetGavdp, KErrNone));
	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::GAVDP_StartStreamsConfirm()
// -----------------------------------------------------------------------------
//    
void CBtsacConfigured::GAVDP_StartStreamsConfirm()
	{
	TRACE_FUNC
	if ( Parent().iStreamer->StartStream(Parent().iStreamingSockets[0], Parent().iStreamer->FrameLength()) != KErrNone )
	 	{
		TRACE_INFO((_L("CBtsacConfigured::GAVDP_StartStreamsConfirm() [ERR] Could not start streaming!")))
		TInt err = Parent().AbortStream();
		if(err)
			{
			TRACE_INFO((_L("CBtsacConfigured::GAVDP_StartStreamsConfirm() [ERR] Couldn't abort stream.")))
			}
		TRAP_IGNORE(CancelActionL(KErrNotReady));
	 	}
	 else
	 	{
	 	TBool Collision = iStartStreamStatus == EStartCollision ? ETrue : EFalse;
		Parent().CompletePendingRequests(KOpenAudioReq, KErrNone);
		TRAP_IGNORE(Parent().ChangeStateL(CBtsacStreaming::NewL(Parent(), EAudioOpenedByAFW, Collision)));
	 	}
	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::GAVDP_StartIndication
// -----------------------------------------------------------------------------
//
TInt CBtsacConfigured::GAVDP_StartIndication(TSEID aLocalSEID)
	{
	TRACE_INFO((_L("CBtsacConfigured::GAVDP_StartIndication() aLocalSEID:%d"), aLocalSEID.SEID()))
	(void) aLocalSEID;
	StopTimer();
	// Check if acc fw has already requested to open audio
	if(Parent().IsOpenAudioReqFromAccFWPending())
		{
		if(iStartStreamStatus == EStartSendBySrc)
			{
			// Src has also sent start stream cmd. 
			iStartStreamStatus = EStartCollision;
			}
			
		GAVDP_StartStreamsConfirm();
		}
	else
		{
    	// Accessory has send this indication.
		// CBtsacStreaming state can use this indication to start audio automatically then later.
 		TRAPD(err, Parent().ChangeStateL(CBtsacStreaming::NewL(Parent(), EAudioOpenedByAcc, EFalse)));
		if (err)
			{
			TRACE_INFO((_L("CBtsacConfigured::GAVDP_StartIndication() Couldn't change state.")))
			return KErrNoMemory;
			}
		}	
	return KErrNone;
	}

// -----------------------------------------------------------------------------
// CBtsacConfigured::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::RequestCompletedL(CBtsacActive& aActive)
    {
	TRACE_FUNC
	switch(aActive.RequestId())
		{
		case KRequestIdTimer:
			{
	  		if(Parent().IsOpenAudioReqFromAccFWPending())
		  		{
		  		// Well, seems that accessory didn't send us GAVDP_StartIndication. Let's try to open audio.
				// Over write iAudioOpenedBy previous value and call open audio
		  		iAudioOpenedBy = EAudioOpenedByAFW;
				OpenAudioLinkL(Parent().GetRemoteAddr());
		  		}
			break;
			}
		default:
			{
			TRACE_INFO((_L("CBtsacConfigured::RequestCompletedL() Unknown request")))
			break;
			}				
		}
    }

// -----------------------------------------------------------------------------
// CBtsacConfigured::CancelRequest
// -----------------------------------------------------------------------------
//
void CBtsacConfigured::CancelRequest(CBtsacActive& aActive)
	{
	TRACE_FUNC
	if(aActive.RequestId() == KRequestIdTimer )
		{		
		iTimer.Cancel();
		}
	else
		{
		TRACE_INFO((_L("CBtsacConfigured::CancelRequest() Unknown request")))
		}
	}    

// -----------------------------------------------------------------------------
// CBtsacConfigured::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacConfigured::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
	StopTimer();
	switch (aError)
		{
		case KErrAvdtpRequestTimeout: // -18005
		case (KErrAvdtpSignallingErrorBase - EAvdtpBadState): // 18094
			{
			TRACE_INFO((_L("CBtsacConfigured::HandleGavdpErrorL() Request TIMEOUT/Bad state")))
			TInt err = Parent().AbortStream();
			if(!err)
				{
				// Complete Connect/Audio requests in Aborting state
				Parent().ChangeStateL(CBtsacAborting::NewL(Parent()));
				}
			else
				{
				CancelActionL(KErrDisconnected);
				}
			break;
			}
		case KErrHCILinkDisconnection: // -6305
		case KErrDisconnected: // -36
			{
			TRACE_INFO((_L("CBtsacConfigured::HandleGavdpErrorL() Signalling disconnected.")))
			CancelActionL(aError);
			break;
			}
		default:
			{
			CancelActionL(KErrDisconnected);
			break;
			}
		}
	}
    
//  End of File  
