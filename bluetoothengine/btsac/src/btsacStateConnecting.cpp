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
#include "btsacStateConnecting.h"
#include "btsacStateConnected.h"
#include "btsacStateListening.h"
#include "btsacactive.h"
#include "btsacGavdp.h"
#include "btsacSEPManager.h"
#include "btsacStreamerController.h"
#include "debug.h"


// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBtsacConnecting::NewL
// -----------------------------------------------------------------------------
//
CBtsacConnecting* CBtsacConnecting::NewL(CBTSAController& aParent, TBTConnType aConnType)
   	{
   	CBtsacConnecting* self = new( ELeave ) CBtsacConnecting(aParent, aConnType);
    return self;
   	}

// -----------------------------------------------------------------------------
// CBtsacConnecting::CBtsacConnecting
// -----------------------------------------------------------------------------
//
CBtsacConnecting::CBtsacConnecting(CBTSAController& aParent, TBTConnType aConnType)
: 	CBtsacState(aParent, EStateConnecting), iConnType(aConnType)
    {
    }

// -----------------------------------------------------------------------------
// CBtsacConnecting::~CBtsacConnecting
// -----------------------------------------------------------------------------
//    
CBtsacConnecting::~CBtsacConnecting()
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacConnecting::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacConnecting::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Connecting"))
	_LIT(KName, "CBtsacStateConnecting");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name); // gavdp call-backs
	Parent().iGavdp->Connect(Parent().GetRemoteAddr());
    }

// -----------------------------------------------------------------------------
// CBtsacConnecting::CancelActionL
// -----------------------------------------------------------------------------
//
void CBtsacConnecting::CancelActionL(TInt aError, TBTSACGavdpResetReason aGavdpReset)
    {
	TRACE_FUNC
	Parent().CompletePendingRequests((KConnectReq | KOpenAudioReq), aError);
    Parent().ChangeStateL(CBtsacListening::NewL(Parent(), aGavdpReset, aError));
    }

// -----------------------------------------------------------------------------
// CBtsacConnecting::CancelOpenAudioL()
// -----------------------------------------------------------------------------
//    
void CBtsacConnecting::CancelOpenAudioL(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_FUNC
	CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
    }

// -----------------------------------------------------------------------------
// CBtsacConnecting::CancelConnectL
// -----------------------------------------------------------------------------
//
void CBtsacConnecting::CancelConnectL()
    {
    TRACE_FUNC
	CancelActionL(KErrCancel, EGavdpResetReasonGeneral);
    }
    
// -----------------------------------------------------------------------------
// CBtsacConnecting::GAVDP_ConnectConfirm
// -----------------------------------------------------------------------------
//	
void CBtsacConnecting::GAVDP_ConnectConfirm(const TBTDevAddr& aAddr)
	{
	TRACE_INFO((_L("CBtsacConnecting::GAVDP_ConnectConfirm() Connected with accessory.")))
	if (Parent().GetRemoteAddr() == aAddr)
		{
        // Complete connect request when configuration is complete
		// cause BT Acc Server will not send a seperate configure request
     	TRAP_IGNORE(Parent().ChangeStateL(CBtsacConnected::NewL(Parent(), iConnType)));
		}
	else // not the accessory that we are trying to connect to ! 
		 // only possibility is that another accessory has made incoming connection, after we have sent a connect request
		 // what a coincidence !
		{
		TRAP_IGNORE(CancelActionL(KErrDisconnected, EGavdpResetReasonGeneral));
		}
	}

// -----------------------------------------------------------------------------
// CBtsacConnecting::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacConnecting::HandleGavdpErrorL(TInt /*aError*/)
	{
	TRACE_FUNC
	// Seen error codes:
	// KErrDied  -13
	// KErrInUse  -14
	// KErrCouldNotConnect -34
	// KErrL2CAPRequestTimeout -6312
	CancelActionL(KErrDisconnected, EGavdpResetReasonGeneral);
	}

//  End of File  
