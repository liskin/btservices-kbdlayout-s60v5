/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Numeric Comparison notifier for Secure Simle Pairing 
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
#include "btnumcmpnotifier.h"  // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include <btotgpairpub.inl>
#include <btengconnman.h>
#include <secondarydisplay/BTnotifSecondaryDisplayAPI.h>
#include <bluetoothuiutil.h>
#include "btnotifnameutils.h"

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTNumCmpNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTNumCmpNotifier* CBTNumCmpNotifier::NewL()
    {
    CBTNumCmpNotifier* self=new (ELeave) CBTNumCmpNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------
// CBTNumCmpNotifier::CBTNumCmpNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTNumCmpNotifier::CBTNumCmpNotifier()
    {
    iPasskeyToShow.Zero();
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTNumCmpNotifier::~CBTNumCmpNotifier()
    {
    }

// ----------------------------------------------------------
// CBTNumCmpNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTNumCmpNotifier::TNotifierInfo CBTNumCmpNotifier::RegisterL()
    {
    iInfo.iUid = KBTNumericComparisonNotifierUid;
    iInfo.iChannel = KBTNumericComparisonNotifierUid;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTNumCmpNotifier::GetParamsL
// Initialize parameters and check if device is already
// in registry. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTNumCmpNotifier::GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::GetParamsL()"));

    CBTNPairNotifierBase::GetParamsL( aBuffer, aReplySlot, aMessage );
    
    TBTNumericComparisonParamsPckg pckg;
 	pckg.Copy(aBuffer);
 	iBTAddr = pckg().DeviceAddress();
    if ( OtherOutgoPairing( iBTAddr) )
        {
        // We won't allow incoming pairing during outgoing pairing:
        FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier: outgoing pair in progress, reject incoming pair"));
        CompleteMessage(KErrCancel);
        return;
        }
    
	TUint32 passkey = pckg().NumericalValue(); 
	iLocallyInitiated = pckg().LocallyInitiated(); 
	iPasskeyToShow.AppendNumFixedWidth(passkey, EDecimal, Klength);
	iPasskeyToShow.Insert(3,_L(" "));

	// base class
	ProcessParamsGetDeviceL( iBTAddr, pckg().DeviceName() );
	
	FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::GetParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTNumCmpNotifier::HandleGetDeviceCompletedL
// Ask user response and return it to caller.
// Store device into registry if user has accepted pairing.
// ----------------------------------------------------------
//
void CBTNumCmpNotifier::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {
    FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::HandleGetDeviceCompletedL()"));
    
    // Check if blocked device. 
    if( CheckBlockedDeviceL() )
        return;
    
    // Turn lights on and deactivate apps -key
    iNotifUiUtil->TurnLightsOn();

   	// For incoming pairing request
	if(!iLocallyInitiated)
	    {
	    TBool keypress = AuthoriseIncomingPairingL();
	    if( !keypress ) 
	        {
	        //User rejects the incoming pairing, complete message in QueryBlockDevice.
	        FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::HandleGetDeviceCompletedL() <<"));
	        return;
	        }
	    }
	
	// For outgoing pairing request or accepted incoming pairing request,
	// ask user to compare the pincodes in two devices.
	TBTDeviceName nameCoverUi( KNullDesC );
    RBuf prompt;
    prompt.CleanupClosePushL();
    GenerateQueryPromptL( prompt );
    TInt answer = iNotifUiUtil->ShowQueryL( prompt, R_BT_GENERIC_QUERY, ECmdShowBtOpenCoverNote, nameCoverUi );   
	CleanupStack::PopAndDestroy( &prompt ); 
	
    if( answer )
        {
        CompleteMessage(answer, KErrNone);
        }
    else
        {
        // If the outgoing pairing mode is set, then this is bonding initiated by the user.
        TBool incomingConn = ( OutgoPairingMode( iBTAddr ) == EBTOutgoingPairNone );

        if( !iMessage.IsNull() )
            {
            // No need to inform the user of the pairing result if the user cancelled it.
            // If user doesn't answer anything but stack cancels, pairing result is needed 
            // to show for outgoing pairing. In this case, don't stop pairng observer.
            CBTEngConnMan::StopPairingObserver( iBTAddr );
            }
        // We may be asking for pairing (locally initiated), because a remote device is requesting
        // accesss to a service for which we require authentication.
        if( iLocallyInitiated && incomingConn )
            {
            CheckAndHandleQueryIntervalL();
            }
        CompleteMessage(KErrCancel);
        }

    FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::HandleGetDeviceCompletedL() completed"));
    }

// ----------------------------------------------------------
// CBTNumCmpNotifier::GenerateQueryPromptL
// Generate prompt for Numeric Comparison query and return.
// ----------------------------------------------------------
//
void CBTNumCmpNotifier::GenerateQueryPromptL( RBuf& aRBuf )
    {
    FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::GenerateQueryPromptL()"));
    TBTDeviceName devName;
    BtNotifNameUtils::GetDeviceDisplayName( devName, iDevice );
    TBuf<KBTDeviceShortNameLength> shortName;
    
    TPtrC namePtr;
    // Cut the name and put ellipsis in the middle if necessary
    // By example "SampleSymbianDevice" after this operation will be shown in
    // the dialog as "Sam...ice"(for 7 chars device name limit)
    // This need to be done, because if name of the device is too long, the PIN
    // codes aren't visible and user can't compare it with PIN code taken from
    // other phone
    if ( devName.Length() > KBTDeviceShortNameLength ) 
        {
        shortName.Copy( devName.Left( KBTDeviceShortNameLength/2 ) );
        //adding ellipsis ("...") char in the middle of the string
        shortName.Append( TChar( KEllipsis) );
        //adding only end of the name to the final string
        shortName.Append( devName.Right( shortName.MaxLength() - 
                          shortName.Length() ) );
        namePtr.Set( shortName );
        }
    else
        {
        namePtr.Set( devName );
        }

    BluetoothUiUtil::LoadResourceAndSubstringL( 
            aRBuf, R_BT_SSP_PASSKEY_COMPARISON_PROMPT, namePtr, 0 );
    // Numeric comparison key shall not be localized either, use our own
    // string loading:
    BluetoothUiUtil::AddSubstringL( aRBuf, iPasskeyToShow, 1);
    FLOG(_L("[BTNOTIF]\t CBTNumCmpNotifier::GenerateQueryPromptL() <<"));

    }

// End of File
