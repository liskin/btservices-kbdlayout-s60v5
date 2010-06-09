/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares User confirmation notifier for Secure Simle Pairing 
*
*/


// INCLUDE FILES
#include <StringLoader.h>    // Localisation stringloader
#include <utf.h>             // Unicode character conversion utilities
#include <avkon.mbg>
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifiers.h>
#else
#include <btextnotifiers.h>
#include <btextnotifierspartner.h>
#endif
#include <BTNotif.rsg>       // Own resources
#include "btnotif.hrh"       // Own resource header
#include "btuserconfnotifier.h"  // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include <btotgpairpub.inl>
#include <btengconnman.h>
#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>
#include <bluetoothuiutil.h>
#include "btnotifnameutils.h"

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTUserConfNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTUserConfNotifier* CBTUserConfNotifier::NewL()
    {
    CBTUserConfNotifier* self=new (ELeave) CBTUserConfNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------
// CBTUserConfNotifier::CBTUserConfNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTUserConfNotifier::CBTUserConfNotifier()
    {
    
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTUserConfNotifier::~CBTUserConfNotifier()
    {
    }

// ----------------------------------------------------------
// CBTUserConfNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTUserConfNotifier::TNotifierInfo CBTUserConfNotifier::RegisterL()
    {
    iInfo.iUid = KBTUserConfirmationNotifierUidCopy;
    iInfo.iChannel = KBTUserConfirmationNotifierUidCopy;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTUserConfNotifier::GetParamsL
// Initialize parameters and check if device is already
// in registry. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTUserConfNotifier::GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    FLOG(_L("[BTNOTIF]\t CBTUserConfNotifier::GetParamsL()"));

    CBTNPairNotifierBase::GetParamsL( aBuffer, aReplySlot, aMessage );
    
    TBTUserConfirmationParamsPckgCopy pckg;
 	pckg.Copy(aBuffer);
 	iBTAddr = pckg().DeviceAddress();
    if ( OtherOutgoPairing( iBTAddr) )
        {
        // We won't allow incoming pairing during outgoing pairing:
        FLOG(_L("[BTNOTIF]\t CBTUserConfNotifier: outgoing pair in progress, reject incoming pair"));
        CompleteMessage(KErrCancel);
        return;
        }
    
	iLocallyInitiated = pckg().LocallyInitiated(); 

	// base class
	ProcessParamsGetDeviceL( iBTAddr, pckg().DeviceName() );
	
	FLOG(_L("[BTNOTIF]\t CBTUserConfNotifier::GetParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTUserConfNotifier::HandleGetDeviceCompletedL
// Ask user response and return it to caller.
// Store device into registry if user has accepted pairing.
// ----------------------------------------------------------
//
void CBTUserConfNotifier::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {
    FLOG(_L("[BTNOTIF]\t CBTUserConfNotifier::HandleGetDeviceCompletedL()"));
    
    // Check if blocked device. 
    if( CheckBlockedDeviceL() )
        return;
    
    // Turn lights on and deactivate apps -key
    iNotifUiUtil->TurnLightsOn();

   	// For incoming pairing request
    TBool answer = AuthoriseIncomingPairingL();
    if( !answer ) 
        {
        //User rejects the incoming pairing, complete message in QueryBlockDevice.
        FLOG(_L("[BTNOTIF]\t CBTUserConfNotifier::HandleGetDeviceCompletedL() <<"));
        return;
        }
    CompleteMessage(answer, KErrNone);

    FLOG(_L("[BTNOTIF]\t CBTUserConfNotifier::HandleGetDeviceCompletedL() completed"));
    }

// End of File
