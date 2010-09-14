/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares authentication (PIN query) notifier class.
*
*/


// INCLUDE FILES
#include <StringLoader.h>    // Localisation stringloader
#include <utf.h>             // Unicode character conversion utilities
#include <avkon.mbg>
#include <btengconnman.h>
#include <btotgpairpub.inl>
#include <btextnotifiers.h>
#include <BTNotif.rsg>       // Own resources
#include <bluetoothuiutil.h>
#include "btnotif.hrh"       // Own resource header
#include "btnpinnotifier.h"  // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include "btnotifnameutils.h"

#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifiers.h>
#else
#include <btextnotifiers.h>
#include <btextnotifierspartner.h>
#endif

#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif
const TInt KDefaultPinCodeLength=4;
const TUint8 KZeroPinValue = '0';

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTPinNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTPinNotifier* CBTPinNotifier::NewL()
    {
    CBTPinNotifier* self=new (ELeave) CBTPinNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------
// CBTPinNotifier::CBTPinNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTPinNotifier::CBTPinNotifier()
    {    
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTPinNotifier::~CBTPinNotifier()
    {
    }

// ----------------------------------------------------------
// CBTPinNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTPinNotifier::TNotifierInfo CBTPinNotifier::RegisterL()
    {
    iInfo.iUid=KBTPinCodeEntryNotifierUid;
    iInfo.iChannel=KBTAuthenticationChannel;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTPinNotifier::GetParamsL
// Initialize parameters and check if device is already
// in registry. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTPinNotifier::ProcessStartParamsL()
    {
    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::ProcessStartParamsL() >>"));

    CBTNPairNotifierBase::ProcessStartParamsL();

    TBTPinCodeEntryNotifierParams param;
 	TPckgC<TBTPinCodeEntryNotifierParams> pckg(param);
 	pckg.Set(*iParamBuffer);

    iBTAddr = pckg().DeviceAddress();
    if ( OtherOutgoPairing(iBTAddr) )
        {
        // We won't allow incoming pairing during outgoing pairing:
        FLOG(_L("[BTNOTIF]\t CBTPinNotifier: outgoing pair in progress, reject incoming pair"));
        CompleteMessage(KErrCancel);
        return;
        }

 	iLocallyInitiated = pckg().LocallyInitiated();
    iPasskeyLength = pckg().PinCodeMinLength();
    iStrongPinRequired = pckg().StrongPinCodeRequired();
    ProcessParamsGetDeviceL( iBTAddr, pckg().DeviceName() );
    
    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::ProcessStartParamsL() <<"));
    }

// ----------------------------------------------------------
// CBTPinNotifier::HandleGetDeviceCompletedL
// Ask user response and return it to caller.
// Store device into registry if user has accepted pairing.
// ----------------------------------------------------------
//
void CBTPinNotifier::HandleGetDeviceCompletedL(const CBTDevice* aDev)
    {
    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::HandleGetDeviceCompletedL() >>"));
    (void) aDev;

    // Check if blocked device  
    if( CheckBlockedDeviceL() )
        return;

    // Turn lights on and deactivate apps -key
    iNotifUiUtil->TurnLightsOn();
    
    if( CheckAndSetAutomatedPairing() )
        {
        CompleteMessage(iPinCode, KErrNone);
        return;
        }

    // For incoming pairing request
    if(!iLocallyInitiated)
    	{
    	TBool userAnswer = AuthoriseIncomingPairingL();
        if( !userAnswer )
            {
            //User rejects the incoming pairing, complete message in QueryBlockDevice.
            FLOG(_L("[BTNOTIF]\t CBTPinNotifier::HandleGetDeviceCompletedL() <<"));
            return;
            }
    	}

	// Use a forever loop here to show query dialog again of ínput pin code does't fulfill the
	// minimum length requirement 
	//      
 	FOREVER
	 	{
	    TBuf<KBTPassKeyMaxLength> tempResultBuffer;
	    RBuf prompt;
	    prompt.CleanupClosePushL();
	    GenerateQueryPromptL( prompt );
        TInt keypress = iNotifUiUtil->ShowTextInputQueryL(tempResultBuffer, prompt, R_BT_ENTER_PASSKEY_QUERY, 
                ECmdShowBtOpenCoverNote, CAknQueryDialog::EConfirmationTone );
        CleanupStack::PopAndDestroy( &prompt );

        if( keypress ) // OK pressed
        	{
       		// Convert data from buf to pin
        	TBuf8<KBTPassKeyMaxLength> buf8;
        	buf8.Zero();
        	CnvUtfConverter::ConvertFromUnicodeToUtf8(buf8, tempResultBuffer);
        	
        	    //check if the length of pin less that minimum required pin code length
        	if ((iPasskeyLength == 0) || ( buf8.Length() >= iPasskeyLength))
        		{
			    iPinCode().iLength = STATIC_CAST(TUint8, buf8.Length());
			    for (TInt index=0; index<buf8.Length(); index++)
			        {
			        iPinCode().iPIN[index] = buf8[index];
			        }
			   	CompleteMessage(iPinCode, KErrNone);
        		break;
        		}
        	else 
        		{
        		//keep FOREVER loop runing, till the user input pin meet the requirement.
        		tempResultBuffer.Zero();
        		}
        	}
	    else    // Cancel pressed
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
	        return;
	        }
	 	}//END of FOREVER loop

    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::HandleGetDeviceCompletedL() completed"));
    }

// ----------------------------------------------------------
// CBTPinNotifier::CheckAndSetAutomatedPairing
// ---------------------------------------------------------- 
TBool CBTPinNotifier::CheckAndSetAutomatedPairing()
    {
    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::CheckAndSetAutomatedPairing()"));    
    TBool ret( EFalse );
    // check if device is headset and passkey is empty -> do automated pairing with 0000.
	if ( iDevice && 
	     OutgoPairingMode( iBTAddr ) == EBTOutgoingHeadsetAutoPairing && 
	     iPasskeyLength <= KDefaultPinCodeLength )
	    {
	    FLOG( _L( "[BTNOTIF]\t CBTPinNotifier::CheckAndSetAutomatedPairing - automated pairing on" ) );
	    FLOG(_L("[BTNOTIF]\t Setting default PIN code"));
	    iPinCode().iLength = KDefaultPinCodeLength;
	    for( TInt i=0; i < KDefaultPinCodeLength; ++i)
	        {
	        iPinCode().iPIN[i] = KZeroPinValue;
	        }
	    ret = ETrue;
	    }
	return ret;
    }

// ----------------------------------------------------------
// CBTPinNotifier::GenerateQueryPromptL
// ---------------------------------------------------------- 
void CBTPinNotifier::GenerateQueryPromptL( RBuf& aRBuf )
    {
    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::GenerateQueryPromptLC() >>"));
    TBTDeviceName devName;
    BtNotifNameUtils::GetDeviceDisplayName( devName, iDevice ); 
    if (iPasskeyLength > 0)  //Minimum length passkey is defined
        {
        BluetoothUiUtil::LoadResourceAndSubstringL( 
                aRBuf, R_BT_MIN_PASSKEY_PROMPT, devName, 1 );

        RBuf tmpBuf;
        tmpBuf.CreateL( aRBuf.MaxLength() );
        tmpBuf.CleanupClosePushL();
        tmpBuf.Swap( aRBuf );
        aRBuf.ReAllocL( aRBuf.MaxLength() + sizeof(TUint));
        aRBuf.Zero();
        // Passkey length should be localized, hope StringLoader can make it:
        StringLoader::Format( aRBuf, tmpBuf, 0, iPasskeyLength );
        CleanupStack::PopAndDestroy( &tmpBuf );
        }
    else
        {
        BluetoothUiUtil::LoadResourceAndSubstringL( 
                aRBuf, R_BT_PASSKEY_PROMPT, devName, 0 );
        }
    FLOG(_L("[BTNOTIF]\t CBTPinNotifier::GenerateQueryPromptLC() <<"));
    }
// End of File
