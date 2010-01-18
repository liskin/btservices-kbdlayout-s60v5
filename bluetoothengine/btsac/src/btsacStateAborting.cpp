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
* Description:  In this state, BTSAC is waiting for a confirmation for abort
*								 operation from GAVDP API. 
*
*/




// INCLUDE FILES
#include "btsacStateAborting.h"
#include "btsacStateListening.h"
#include "btsacactive.h"
#include "btsacGavdp.h"
#include "debug.h"


// ================= MEMBER FUNCTIONS =======================
// -----------------------------------------------------------------------------
// CBtsacAborting::NewL
// -----------------------------------------------------------------------------
//
CBtsacAborting* CBtsacAborting::NewL(CBTSAController& aParent)
    {
    CBtsacAborting* self = new( ELeave ) CBtsacAborting(aParent);
    return self;
    }

// -----------------------------------------------------------------------------
// CBtsacAborting::CBtsacAborting
// -----------------------------------------------------------------------------
//
CBtsacAborting::CBtsacAborting(CBTSAController& aParent)
:   CBtsacState(aParent, EStateAborting)
    {
    }
  
// -----------------------------------------------------------------------------
// CBtsacAborting::~CBtsacAborting
// -----------------------------------------------------------------------------
//    
CBtsacAborting::~CBtsacAborting()
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtsacAborting::EnterL
// -----------------------------------------------------------------------------
//
void CBtsacAborting::EnterL()
    {
	TRACE_STATE(_L("[BTSAC State] Aborting"))
	_LIT(KName, "CBtsacStateAborting");
	const TDesC& Name = KName;
	Parent().iGavdp->RegisterObserver(this, Name);
    }

// -----------------------------------------------------------------------------
// CBtsacAborting::HandleGavdpErrorL
// -----------------------------------------------------------------------------
//	
void CBtsacAborting::HandleGavdpErrorL(TInt aError)
	{
	TRACE_FUNC
	Parent().CompletePendingRequests(KCompleteAllReqs, aError);
	Parent().ChangeStateL(CBtsacListening::NewL(Parent(), EGavdpResetReasonGeneral, KErrDisconnected));
	}

//  End of File  
