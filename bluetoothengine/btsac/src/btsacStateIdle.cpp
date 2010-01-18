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
*				 from a stereo audio accessory
*
*/




// INCLUDE FILES
#include "btsacStateIdle.h"
#include "btsacactive.h"
#include "btsacStateListening.h"
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
// CBtsacIdle::NewL
// -----------------------------------------------------------------------------
//
CBtsacIdle* CBtsacIdle::NewL(CBTSAController& aParent)
    {
    CBtsacIdle* self = new( ELeave ) CBtsacIdle(aParent);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtsacIdle::TStateIdle
// -----------------------------------------------------------------------------
//
CBtsacIdle::CBtsacIdle(CBTSAController& aParent)
: 	CBtsacState(aParent, EStateIdle)
    {
    }

// -----------------------------------------------------------------------------
// CBtsacIdle::TStateIdle
// -----------------------------------------------------------------------------
//
CBtsacIdle::~CBtsacIdle()
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacIdle::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacIdle::EnterL()
    {
    TRACE_STATE(_L("[BTSAC State] Idle"))
	_LIT(KName, "CBtsacStateIdle");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);
	
	TSBCCodecCapabilities sbc;
	TInt err =  Parent().iStreamer->FillCapabilities(sbc);
	if ( err )
		{
		TRACE_INFO((_L("CBtsacIdle::EnterL() Couldn't fill capabilities. Error: %d"), err))
		User::Leave(KErrAvdtpBaseError);
		}
	// start with registering sdp, open gavdp and registering local SEPs	
	err = Parent().iGavdp->StartSrc(*Parent().iLocalSEPs, sbc);
	if ( err )
		{
		TRACE_INFO((_L("CBtsacIdle::EnterL() Couldn't start Gavdp. Error: %d"), err))
		User::Leave(KErrAvdtpBaseError);
		}
	// Let's wait GAVDP_ConfigurationConfirm callback
    }
    
// -----------------------------------------------------------------------------
// CBtsacIdle::GAVDP_ConfigurationConfirm
// -----------------------------------------------------------------------------
//
void CBtsacIdle::GAVDP_ConfigurationConfirm()
	{
	TRACE_INFO((_L("CBtsacIdle::GAVDP_ConfigurationConfirm() Local SEPs registered successfully.")))
	TRAP_IGNORE(Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonNone, KErrNone)));
	}

// -----------------------------------------------------------------------------
// CBtsacIdle::GAVDP_Error
// -----------------------------------------------------------------------------
//	
void CBtsacIdle::GAVDP_Error(TInt aError, const TDesC8& /*aErrorData*/)
	{
	TRACE_INFO((_L("CBtsacIdle::GAVDP_Error(%d)"), aError))
	(void)aError;
	}
    
//  End of File  
