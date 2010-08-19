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
* Description:  In this state, BTSAC is listening for remote connection
*								 from a stereo audio accessory
*
*/




// INCLUDE FILES
#include "btsacStateListening.h"
#include "btsacStateConnected.h"
#include "btsacStateConnecting.h"
#include "btsacStateConfiguring.h"
#include "btsacactive.h"
#include "btsacGavdp.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacListening::NewL
// -----------------------------------------------------------------------------
//
CBtsacListening* CBtsacListening::NewL(CBTSAController& aParent,
									TBTSACResetGavdp aResetGavdp, 
									TInt aDisconnectReason)
    {
    CBtsacListening* self = new( ELeave ) CBtsacListening(aParent, aResetGavdp, aDisconnectReason);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtsacListening::CBtsacListening
// -----------------------------------------------------------------------------
//
CBtsacListening::CBtsacListening(CBTSAController& aParent,
	TBTSACResetGavdp aResetGavdp, TInt aDisconnectReason)
: 	CBtsacState(aParent, EStateListening), iResetGavdp(aResetGavdp),
	iDisconnectReason(aDisconnectReason), iPendingRequests(KRequestNone),
	iInitializationProcedure(EInitProcedureOngoing)
    {
    }

// -----------------------------------------------------------------------------
// CBtsacListening::ConstructL
// -----------------------------------------------------------------------------
//
void CBtsacListening::ConstructL()
	{
	iSelfCompleteActive = CBtsacActive::NewL(*this, CActive::EPriorityStandard, KRequestIdSelfComplete);
	}

// -----------------------------------------------------------------------------
// CBtsacListening::~CBtsacListening
// -----------------------------------------------------------------------------
//    
CBtsacListening::~CBtsacListening()
    {
    TRACE_FUNC
    delete iSelfCompleteActive;
    iSelfCompleteActive = NULL;
    }

// -----------------------------------------------------------------------------
// CBtsacListening::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacListening::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Listening"))
	_LIT(KName, "CBtsacStateListening");
	const TDesC& Name = KName;	
	Parent().iGavdp->RegisterObserver(this, Name);
	
	Parent().ResetRemoteCache();
	Parent().CleanSockets();
	
	// RGavdp.Close (which is called from ResetGavdp method) should not be called from 
	// within a client's implementation of any MGavdpUser (upcall) method. Do self
	// complete first then call ResetGavdp.
	
	// Need for self completion is gavdp user up calls but we will do self complete every time
	// when we enter into listening state.
	DoSelfComplete(KErrNone);
    }

// -----------------------------------------------------------------------------
// CBtsacListening::ConnectL
// -----------------------------------------------------------------------------
//
void CBtsacListening::ConnectL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    iPendingRequests = KConnectReq;
    if(iInitializationProcedure == EInitProcedureDone)
		{
		// we have a connection already with this accessory
		if ( Parent().GetRemoteAddr() == aAddr )
			{
			Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), EOutgoingConnNoDiscovery));
			}
		else
			{
		    Parent().SetRemoteAddr(aAddr);	// first update address cause InitializeSEPManager uses it
		    Parent().InitializeSEPManager(); // initializes iRemoteSEPs
		    Parent().ChangeStateL(CBtsacConnecting::NewL(Parent(), EOutgoingConn));
			}
		}
	else
		{
		// Save the address, reset procedure is not completed yet.
		Parent().SetRemoteAddr(aAddr);
		}
    }

// -----------------------------------------------------------------------------
// CBtsacListening::OpenAudioLinkL
// -----------------------------------------------------------------------------
//    
void CBtsacListening::OpenAudioLinkL(const TBTDevAddr& aAddr)
	{  
	TRACE_FUNC
	iPendingRequests = KOpenAudioReq;
	if(iInitializationProcedure == EInitProcedureDone)
		{
	    // we have a connection already with this accessory
	    if ( Parent().GetRemoteAddr() == aAddr )
	    	{
			Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), EOutgoingConnWithAudioNoDiscovery));
	    	}
	    else
	    	{
	        Parent().SetRemoteAddr(aAddr);	// first update address cause InitializeSEPManager uses it
		    Parent().InitializeSEPManager(); // initializes iRemoteSEPs
		    Parent().ChangeStateL(CBtsacConnecting::NewL(Parent(), EOutgoingConnWithAudio));
	    	}
		}
	else
		{
		// Save the address, reset procedure is not completed yet.
		Parent().SetRemoteAddr(aAddr);
		}	
	}

// -----------------------------------------------------------------------------
// CBtsacListening::CancelOpenAudioLinkL
// -----------------------------------------------------------------------------
//    
void CBtsacListening::CancelOpenAudioLinkL()
    {
    TRACE_FUNC
    // We can have only one request active at the time, so we can reset pending requests
    iPendingRequests = KRequestNone;
	GoListen();
	Parent().CompletePendingRequests(KOpenAudioReq, KErrCancel);
    }	

// -----------------------------------------------------------------------------
// CBtsacListening::DeleyedOpenAudioAndConnectL
// -----------------------------------------------------------------------------
//	
void CBtsacListening::DeleyedOpenAudioAndConnectL()
	{
	TRACE_FUNC
	if(iPendingRequests & (KConnectReq | KOpenAudioReq))
		{				
		Parent().InitializeSEPManager();
		if(iPendingRequests & KOpenAudioReq)
			{			
			Parent().ChangeStateL(CBtsacConnecting::NewL(Parent(), EOutgoingConnWithAudio));
			}
		else
			{			
			Parent().ChangeStateL(CBtsacConnecting::NewL(Parent(), EOutgoingConn));
			}				
		}
	}

// -----------------------------------------------------------------------------
// CBtsacListening::GAVDP_ConfigurationConfirm
// -----------------------------------------------------------------------------
//
void CBtsacListening::GAVDP_ConfigurationConfirm()
	{
	TRACE_INFO((_L("CBtsacListening::GAVDP_ConfigurationConfirm() Local SEPs registered successfully.")))
	// Local SEPs got registered, so start listening
	TInt err = Parent().iGavdp->Listen();
	if (err)
		{
		TRACE_INFO((_L("CBtsacListening::GAVDP_ConfigurationConfirm() Listen returned error:%d."), err))
		ResetGavdp();
		return;
		}
	if(iInitializationProcedure == EInitProcedureWaitingConfConfirmed)
		{
		iInitializationProcedure = EInitProcedureDone;
		TRAP_IGNORE(DeleyedOpenAudioAndConnectL());
		}
	}

// -----------------------------------------------------------------------------
// CBtsacListening::GAVDP_ConnectConfirm
// -----------------------------------------------------------------------------
//	
void CBtsacListening::GAVDP_ConnectConfirm(const TBTDevAddr& aAddr)
	{
	TRACE_INFO((_L("CBtsacListening::GAVDP_ConnectConfirm() Accessory made connection.")))
	Parent().SetRemoteAddr(aAddr);  // first update address cause InitializeSEPManager uses it
	Parent().InitializeSEPManager(); // initializes iRemoteSEPs
	TRAPD(err, Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), EIncomingConn)));
	if(err)
		{
		TRACE_INFO((_L("CBtsacListening::GAVDP_ConnectConfirm(), NewL failed.")))
		}
	}

// -----------------------------------------------------------------------------
// CBtsacListening::GAVDP_AbortStreamConfirm
// -----------------------------------------------------------------------------
//	
void CBtsacListening::GAVDP_AbortStreamConfirm()
	{
	TRACE_FUNC
	GoListen();
	}
	
// -----------------------------------------------------------------------------
// CBtsacListening::GAVDP_ConfigurationStartIndication
// -----------------------------------------------------------------------------
//	
void CBtsacListening::GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID)
	{
    TRACE_FUNC
    Parent().InitializeSEPManager();
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacConfiguring::NewL(Parent(), aLocalSEID, aRemoteSEID)));
	}

// -----------------------------------------------------------------------------
// CBtsacListening::RequestCompletedL
// -----------------------------------------------------------------------------
//
void CBtsacListening::RequestCompletedL(CBtsacActive& aActive)
    {
	TRACE_FUNC
	switch(aActive.RequestId())
		{
		case KRequestIdSelfComplete:
			{
			TBTDevAddr remoteAddr = Parent().GetRemoteAddr();
#ifdef PRJ_ENABLE_TRACE
			TBuf<12> buf;
			remoteAddr.GetReadable(buf);
			TRACE_INFO((_L("\tremoteAddr = %S"), &buf))
#endif
			TRACE_INFO((_L("\tiResetGavdp = %d"), iResetGavdp))
			if(iResetGavdp == EResetGavdp)
				{
				ResetGavdp();
				}
			else
				{
				GoListen();
				}				
			TRACE_INFO((_L("\tiPendingRequests = %d"), iPendingRequests))
			if(iPendingRequests == KRequestNone)
				{
				if(remoteAddr != TBTDevAddr())
					{						
					TRACE_INFO((_L("\tcalling DisconnectedFromRemote with iDisconnectReason = %d"), iDisconnectReason))
					Parent().DisconnectedFromRemote(remoteAddr, iDisconnectReason);
					}
				Parent().SetResetAudioInput(ETrue);
				}
			if(Parent().GetResetAudioInput())
				{
				Parent().SetResetAudioInput(EFalse);
				Parent().iStreamer->ResetAudioInput();
				}
			if(iInitializationProcedure != EInitProcedureWaitingConfConfirmed)
				{
				iInitializationProcedure = EInitProcedureDone;
				DeleyedOpenAudioAndConnectL();
				}
			break;
			}
		default:
			{
			TRACE_INFO((_L("CBtsacListening::RequestCompletedL() Unknown request")))
			break;
			}	
		}
    }

// -----------------------------------------------------------------------------
// CBtsacListening::GoListen
// -----------------------------------------------------------------------------
//	
void CBtsacListening::GoListen()
	{
	TRACE_INFO((_L("CBtsacListening::GoListen(), Disconnect signalling channel")))
	if(iPendingRequests == KRequestNone)
		{					
		Parent().SetRemoteAddr(TBTDevAddr());
		}
	TInt err = Parent().iGavdp->Shutdown();
	if(!err)
		{
		TRACE_INFO((_L("CBtsacListening::GoListen(), Signalling disconnected, Re-listen...")))
		// Starts to listen for inbound signalling channel connections.
		err = Parent().iGavdp->Listen();
		}
	if(err)
		{
		// Shutdown failed, reset gavdp
		TRACE_INFO((_L("CBtsacListening::GoListen(), error = %d."), err))
		ResetGavdp();
		}		
	}

// -----------------------------------------------------------------------------
// CBtsacListening::ResetGavdp
// -----------------------------------------------------------------------------
//
TInt CBtsacListening::ResetGavdp()
	{
	TRACE_FUNC
	Parent().iGavdp->Close();
	if( Parent().iGavdp->Open() == KErrNone )
		{
		iInitializationProcedure = EInitProcedureWaitingConfConfirmed;
        if(iPendingRequests == KRequestNone)
            {
            TRACE_INFO((_L("CBtsacListening::ResetGavdp() Remote Addr reseted.")))
            Parent().SetRemoteAddr(TBTDevAddr());
            }
		return Parent().iGavdp->RegisterSEPs(*Parent().iLocalSEPs, Parent().iStreamer->GetCaps());
		}
	else
		{
		TRACE_INFO((_L("CBtsacListening::ResetGavdp() Couldn't open gavdp.")))
		return KErrGeneral;
		}		
	}

// -----------------------------------------------------------------------------
// CBtsacListening::CancelRequest
// -----------------------------------------------------------------------------
//	
void CBtsacListening::CancelRequest(CBtsacActive& /*aActive*/)
	{
	TRACE_FUNC
	}

// -----------------------------------------------------------------------------
// CBtsacListening::DoSelfComplete
// -----------------------------------------------------------------------------
//
void CBtsacListening::DoSelfComplete(TInt aError) 
	{
	TRACE_FUNC
	if(iSelfCompleteActive)
		{		
		if (!iSelfCompleteActive->IsActive())
		    {		    
		    TRequestStatus* sta = &(iSelfCompleteActive->iStatus);
		    iSelfCompleteActive->iStatus = KRequestPending;
		    User::RequestComplete(sta, aError);
		    iSelfCompleteActive->GoActive();
		    }
		}
	else
		{
		TRACE_INFO((_L("CBtsacListening::DoSelfComplete() AO doesn't exist.")))
		}
	}
    
// -----------------------------------------------------------------------------
// CBtsacListening::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacListening::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
    if(iInitializationProcedure != EInitProcedureDone)
	    {
	    TRACE_INFO((_L("CBtsacListening::HandleGavdpErrorL() error ignored, initialization procedure ongoing.")))
	    iResetGavdp = EResetGavdp;
	    return;
	    }
	switch(aError)
		{
		case KErrHCILinkDisconnection: // -6305
		case KErrDisconnected: // -36
			{
			// If address is zero, initialization procedure is already executed, so there is no need
			// to tell gavdp to go listen and no need to inform btaudioman about the disconnection
			if(Parent().GetRemoteAddr() != TBTDevAddr())
				{
				GoListen();
				// for both outgoing or incoming connection, if we have an error, 
				// this means there is disconnection		 	
			 	Parent().DisconnectedFromRemote(Parent().GetRemoteAddr(), aError);	  	  	    	
				Parent().SetRemoteAddr(TBTDevAddr());
				Parent().iStreamer->ResetAudioInput();
			 	}
			break;
			}
		
		case KErrCouldNotConnect: // -34
			{
			GoListen();
			break;
			}
		
		default:
		// KErrAbort -39
		// KErrDied -13
		// KErrInUse -14
			{
			ResetGavdp();			
			break;
			}
		}
	Parent().CompletePendingRequests(KCompleteAllReqs, aError);
	}

//  End of File  
