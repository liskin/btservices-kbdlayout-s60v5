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
* Description:  Base class for all notifiers for pairing purpose.
*
*/

#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifiers.h>
#else
#include <btextnotifiers.h>
#include <btextnotifierspartner.h> // new file introduced by xSymbian
#endif

#include <utf.h>             // Unicode character conversion utilities
#include <btengconnman.h>
#include <BTNotif.rsg>       // Own resources
#include <bluetoothuiutil.h>
#include "btnpairnotifier.h"
#include "btNotifDebug.h"
#include "btnotifnameutils.h"

// ----------------------------------------------------------
// CBTNPairNotifierBase::NewL
// ----------------------------------------------------------
//
CBTNPairNotifierBase* CBTNPairNotifierBase::NewL()
    {
    CBTNPairNotifierBase* self = new (ELeave) CBTNPairNotifierBase();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase Destructor
// ----------------------------------------------------------
//
CBTNPairNotifierBase::~CBTNPairNotifierBase()
    {
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase default construct
// ----------------------------------------------------------
//
CBTNPairNotifierBase::CBTNPairNotifierBase()
    :iLocallyInitiated(EFalse)
    {
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::RegisterL
// ----------------------------------------------------------
//
CBTNPairNotifierBase::TNotifierInfo CBTNPairNotifierBase::RegisterL()
    {
    return iInfo;
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::GetParamsL
// ----------------------------------------------------------
//
void CBTNPairNotifierBase::GetParamsL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    (void)aBuffer;
    if( !iMessage.IsNull() )
        {
        User::Leave(KErrInUse);
        }

    iMessage = (RMessage2)aMessage;
    iReplySlot = aReplySlot;

    if ( AutoLockOnL() )
        {
        // The phone is locked, access denied.
        //
        CompleteMessage(KErrCancel);
        }

    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::UpdateL
// ----------------------------------------------------------
//
void CBTNPairNotifierBase::UpdateL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    (void) aReplySlot;
    UpdateL(aBuffer);
    aMessage.Complete(KErrNone);
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::UpdateL
// ----------------------------------------------------------
//
TPtrC8 CBTNPairNotifierBase::UpdateL( const TDesC8& aBuffer )
    {
    FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase::UpdateL()"));

    TBTDeviceNameUpdateParamsPckg pckg;
    pckg.Copy( aBuffer );
    if (pckg().Type() != TBTNotifierUpdateParams2::EDeviceName)
        {
        TPtrC8 ret(KNullDesC8);
        return (ret);
        }

    if (pckg().Result() == KErrNone)
        {
        BtNotifNameUtils::SetDeviceNameL(pckg().DeviceName(), *iDevice);

    // Finally show new prompt for dialog if it is still on the screen
    // and user has not given a alias for device.   
        if( !iNotifUiUtil->IsQueryReleased() && !iDevice->IsValidFriendlyName() )
            {
            RBuf prompt;
            prompt.CleanupClosePushL();
            GenerateQueryPromptL( prompt );
            iNotifUiUtil->UpdateQueryDlgL( prompt );
            CleanupStack::PopAndDestroy( &prompt );
            }
        }

    FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase::UpdateL() completed"));
    TPtrC8 ret(KNullDesC8);
    return (ret);
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::ProcessParamsGetDeviceL
// Get device from registry based on notifier's param
// ----------------------------------------------------------
//
void CBTNPairNotifierBase::ProcessParamsGetDeviceL( const TBTDevAddr& aAddr, const TBTDeviceName& aName )
    {
    iDevice = CBTDevice::NewL(aAddr);
    BtNotifNameUtils::SetDeviceNameL(aName, *iDevice);

    // Get all properties of device from registry. 
    GetDeviceFromRegL( aAddr ); 
    
#ifdef _DEBUG
    FTRACE(FPrint(_L("[BTNOTIF]\t Executing authentication... Parameters:")));
    TBuf<12> deviceAddressString;
    aAddr.GetReadable(deviceAddressString);
    FTRACE(FPrint(_L("[BTNOTIF]\t BT Address: %S"), &deviceAddressString));
#endif
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::CheckBlockedDeviceL
// Forbid pairing from remote banned devices
// ----------------------------------------------------------
//
TBool CBTNPairNotifierBase::CheckBlockedDeviceL()
    {
    if ( !iDevice )
        {
        User::Leave( KErrGeneral );
        }

    // Do not allow pairing attempt from banned devices
    if( iDevice->GlobalSecurity().Banned() && !iLocallyInitiated )
       {
       FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase::CheckBlockedDeviceL() Pairing attempt from banned device is denied.")); 
       CompleteMessage(KErrCancel);       
       return ETrue;
       }
    return EFalse;
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::AuthoriseIncomingPairingL
// Query user to authorise imcoming pair request or not.
// If reject, query whether to block the remot device. 
// ----------------------------------------------------------
//
TBool CBTNPairNotifierBase::AuthoriseIncomingPairingL()
    {
    FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase::AuthoriseIncomingPairingL() >>"));
    
    // Start pairing observer so that user will be informed about the result:
    TInt err = CBTEngConnMan::StartPairingObserver( iBTAddr );
    if ( err )
        {
        // Failed to start observer because of other ongoing pairing,
        // reject this request:
        CompleteMessage( err );
        return EFalse;            
        }
    
    TBTDeviceName devName;
    BtNotifNameUtils::GetDeviceDisplayName( devName, iDevice );
    RBuf prompt;
    prompt.CleanupClosePushL();
    BluetoothUiUtil::LoadResourceAndSubstringL( 
            prompt, R_BT_ACCEPT_PAIRING_REQUEST, devName, 0 );

    devName.Zero();
    // Show query for use to accept/reject incoming pairing request
    TInt keypress = iNotifUiUtil->ShowQueryL( prompt, R_BT_GENERIC_QUERY, 
             ECmdBTnotifUnavailable, devName, CAknQueryDialog::EConfirmationTone );
    CleanupStack::PopAndDestroy( &prompt );

    if( iMessage.IsNull() ) // cancelled by the stack
        {
        return EFalse;
        }
    else if( !keypress ) // User rejected pairing
        {
        FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase: user rejected incoming pairing"));
        // stop observer to omit pairing failure note.
        CBTEngConnMan::StopPairingObserver( iDevice->BDAddr() );
        iDevice->SetPaired(EFalse);
        CheckAndHandleQueryIntervalL();
        if( !iMessage.IsNull() )
            {
            // Ask user does he/she wishes to block this device (if not canceled because the 
            // frequency is too high).
            QueryBlockDeviceL();
            }
        return EFalse;
        }

    FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase::AuthoriseIncomingPairingL() << "));
    return ETrue;
    }

// ----------------------------------------------------------
// CBTNPairNotifierBase::GenerateQueryPromptL
// To be implemented in derived classes.
// ----------------------------------------------------------
//
void CBTNPairNotifierBase::GenerateQueryPromptL(  RBuf& aRBuf )
    {
    FLOG(_L("[BTNOTIF]\t CBTNPairNotifierBase::GenerateQueryPromptL WARNING "));
    (void) aRBuf;
    }
