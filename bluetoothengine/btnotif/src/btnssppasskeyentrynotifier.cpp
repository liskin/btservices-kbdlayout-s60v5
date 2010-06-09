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
#include <btengdevman.h>
#include <btengconnman.h>
#include <btotgpairpub.inl>
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifiers.h>
#else
#include <btextnotifiers.h>
#include <btextnotifierspartner.h>
#endif
#include <BTNotif.rsg>       // Own resources
#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>
#include <bluetoothuiutil.h>
#include "btnotif.hrh"       // Own resource header
#include "btnssppasskeyentrynotifier.h"  // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include "btnotifnameutils.h"

#ifdef __SERIES60_HELP
#include <hlplch.h>
#include <csxhelp/bt.hlp.hrh> // The bt hrh info is needed, for help launching
#endif


// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTSSPPasskeyEntryNotifier* CBTSSPPasskeyEntryNotifier::NewL()
    {
    CBTSSPPasskeyEntryNotifier* self=new (ELeave) CBTSSPPasskeyEntryNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::CBTSSPPasskeyEntryNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTSSPPasskeyEntryNotifier::CBTSSPPasskeyEntryNotifier()
    {
    iPasskey.Zero();
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTSSPPasskeyEntryNotifier::~CBTSSPPasskeyEntryNotifier()
    {
    }

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTSSPPasskeyEntryNotifier::TNotifierInfo CBTSSPPasskeyEntryNotifier::RegisterL()
    {
    iInfo.iUid = KBTPasskeyDisplayNotifierUid;
    iInfo.iChannel = KBTPasskeyDisplayNotifierUid;
    iInfo.iPriority = ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::GetParamsL
// Initialize parameters and check if device is already
// in registry. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTSSPPasskeyEntryNotifier::GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::GetParamsL()"));

    CBTNPairNotifierBase::GetParamsL( aBuffer, aReplySlot, aMessage );
    
    TBTPasskeyDisplayParamsPckg pckg;
    pckg.Copy(aBuffer);
    iBTAddr = pckg().DeviceAddress();
    if ( OtherOutgoPairing(iBTAddr) )
        {
        // We won't allow incoming pairing during outgoing pairing:
        FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier: outgoing pair in progress, reject incoming pair"));
        CompleteMessage(KErrCancel);
        return;
        }

    iLocallyInitiated = pckg().LocallyInitiated();
    iPasskey.AppendNumFixedWidth(pckg().NumericalValue(), EDecimal, KPassKeylength);
    iPasskey.Insert(3,_L(" "));

    ProcessParamsGetDeviceL( iBTAddr, pckg().DeviceName() );
    
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::GetParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::UpdateL
// Notifier update. Stores the received bluetooth
// device name into registry and show it on screen.
// ----------------------------------------------------------
//
TPtrC8 CBTSSPPasskeyEntryNotifier::UpdateL(const TDesC8& aBuffer)
    {
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::UpdateL()"));
    
    TBTPasskeyDisplayUpdateParamsPckg pckg;
    pckg.Copy(aBuffer);
  
    if(pckg().Type() != TBTNotifierUpdateParams2::EPasskeyDisplay)
        {
        TPtrC8 ret(KNullDesC8);
        return (ret);
        }
    
    switch (pckg().KeypressNotification())
        {
        case EPasskeyEntryStarted:
            {
            iBuf.Zero();
            iBuf.Append(iPasskey);
            iBuf.Append(_L("    "));
            break;
            }
        case EPasskeyDigitEntered:
            {
            // show an '*'
            iBuf.Append(_L("*"));
            break;
            }
        case EPasskeyDigitDeleted:
            {
            // remove an '*' from the tail of iBuf
            iBuf.Delete(iBuf.Length()-1, 1);  
            break;
            }
        case EPasskeyCleared:
            {
            // remove all '*'
            iBuf.Zero();
            iBuf.Append(iPasskey);
            iBuf.Append(_L("    "));
            break;
            }
        case EPasskeyEntryCompleted:
            {                                  
            // complete passkey entry
            // Set iAnswer to 1 ,
            // Continue pairing in Run()
            iAnswer = 1;
            //iPasskeyDlg->DismissQueryL();
            iNotifUiUtil->DismissDialog();
            break;
            }
        }
    if ( !iNotifUiUtil->IsQueryReleased() )
        {
        RBuf prompt;
        prompt.CleanupClosePushL();
        GenerateQueryPromptL( prompt );
        iNotifUiUtil->UpdateQueryDlgL( prompt );
        CleanupStack::PopAndDestroy( &prompt ); 
        }        
    
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::UpdateL() completed"));
    TPtrC8 ret(KNullDesC8);
    return (ret);
    }

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::HandleGetDeviceCompletedL
// Ask user response and return it to caller.
// Store device into registry if user rejects the incoming pairing.
// ----------------------------------------------------------
//
void CBTSSPPasskeyEntryNotifier::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::HandleGetDeviceCompletedL()"));
    iAnswer = 0;
    
    // Check if blocked device. 
    if( CheckBlockedDeviceL() )
        return;

    // For pairing request from remote device
    if ( !iLocallyInitiated )
        {
        TBool keypress = AuthoriseIncomingPairingL();
        if( !keypress )
            {
            //User rejects the incoming pairing, complete message in QueryBlockDevice.
            FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::HandleGetDeviceCompletedL() <<"));
            return;
            }
        }
   
    TBTDeviceName nameCoverUi( KNullDesC );
    
    iBuf.Zero();
    iBuf.Append(iPasskey);
    iBuf.Append(_L("    "));
    
    RBuf prompt;
    prompt.CleanupClosePushL();
    GenerateQueryPromptL( prompt );
    
    TInt answer = iNotifUiUtil->ShowQueryL( prompt, R_BT_PASSKEY_QUERY, 
            ECmdBTnotifUnavailable, nameCoverUi );    
    CleanupStack::PopAndDestroy( &prompt );  

    if( iAnswer ) // user pressed entry key from remote keyboard.
        {
        CompleteMessage(KErrNone);
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
            (void) CBTEngConnMan::StopPairingObserver( iBTAddr );
            }
        // We may be asking for pairing (locally initiated), because a remote device is requesting
        // accesss to a service for which we require authentication.
        if( iLocallyInitiated && incomingConn )
            {
            CheckAndHandleQueryIntervalL();
            }
        CompleteMessage(KErrCancel);
        }
    
     FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::HandleGetDeviceCompletedL() completed"));
    }

// ----------------------------------------------------------
// CBTSSPPasskeyEntryNotifier::GenerateQueryPromptL
// ----------------------------------------------------------
//
void CBTSSPPasskeyEntryNotifier::GenerateQueryPromptL(  RBuf& aRBuf )
    {
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::GenerateQueryPromptL()"));
    TBTDeviceName devName; 
    BtNotifNameUtils::GetDeviceDisplayName(devName, iDevice);   
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            aRBuf, R_BT_SSP_PASSKEY_ENTRY, devName, 0 );
    BluetoothUiUtil::AddSubstringL( aRBuf, iBuf, 1 );
    FLOG(_L("[BTNOTIF]\t CBTSSPPasskeyEntryNotifier::GenerateQueryPromptL() << "));

    }
// End of File
