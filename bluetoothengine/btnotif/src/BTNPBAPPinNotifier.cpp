/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares PBAP PIN query notifier class.
*
*/


// INCLUDE FILES
#include <StringLoader.h>       // Localisation stringloader
#include <utf.h>                // Unicode character conversion utilities

#include <BTNotif.rsg>          // own resources
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifiers.h>
#else
#include <btextnotifiers.h>
#include <btextnotifierspartner.h>
#endif

#include "btnpbappinnotifier.h" // own class definition
#include "btNotifDebug.h"       // debugging macros

#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTObexPinNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTPBAPPinNotifier* CBTPBAPPinNotifier::NewL()
    {
    CBTPBAPPinNotifier* self=new (ELeave) CBTPBAPPinNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTObexPinNotifier::CBTObexPinNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTPBAPPinNotifier::CBTPBAPPinNotifier()
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTPBAPPinNotifier::~CBTPBAPPinNotifier()
    {
    }

// ----------------------------------------------------------
// CBTPBAPPinNotifier::RegisterL
// ----------------------------------------------------------
//
CBTPBAPPinNotifier::TNotifierInfo CBTPBAPPinNotifier::RegisterL()
    {
    iInfo.iUid=KPbapAuthNotifierUid;
    iInfo.iChannel=KBTObexPINChannel;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTPBAPPinNotifier::ProcessStartParamsL
// Initialize parameters. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTPBAPPinNotifier::ProcessStartParamsL()
    {
    FLOG(_L("[BTNOTIF]\t CBTPBAPPinNotifier::ProcessStartParamsL()"));

    if ( AutoLockOnL() )
        {
        // The phone is locked, access denied.
        //
        CompleteMessage(KErrCancel);
        return;
        }

    ShowNoteCompleteMessageL();

    FLOG(_L("[BTNOTIF]\t CBTPBAPPinNotifier::ProcessStartParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTPBAPPinNotifier::ShowNoteCompleteMessageL
// Ask user response and return it to caller.
// ----------------------------------------------------------
//
void CBTPBAPPinNotifier::ShowNoteCompleteMessageL()
    {
    FLOG(_L("[BTNOTIF]\t CBTPBAPPinNotifier::ShowNoteCompleteMessageL()"));

    TBuf<KBTObexPasskeyMaxLength> pbapPasskey;    // Unicode buffer
		
    TInt keypress = iNotifUiUtil->ShowTextInputQueryL( pbapPasskey, 
            R_BT_OBEX_PASSKEY_QUERY, ECmdShowBtOpenCoverNote );
	
    if( keypress ) // OK pressed
        {        
        TPbapAuthNotifierResponsePckg  pbapPasskeyResponse;
        pbapPasskeyResponse().SetPassword(pbapPasskey);        
        CompleteMessage(pbapPasskeyResponse, KErrNone);
        }
    else  // Cancel pressed
        {
        CompleteMessage(KErrCancel);
        }

    FLOG(_L("[BTNOTIF]\t CBTPBAPPinNotifier::ShowNoteCompleteMessageL() completed"));
    }

// End of File
