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
* Description:  Declares OBEX PIN query notifier class.
*
*/


// INCLUDE FILES
#include <utf.h>                // Unicode character conversion utilities

#include <BTNotif.rsg>          // Own resources
#include "btnobexpinnotifier.h" // Own class definition
#include "btNotifDebug.h"       // Debugging macros

#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTObexPinNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTObexPinNotifier* CBTObexPinNotifier::NewL()
    {
    CBTObexPinNotifier* self=new (ELeave) CBTObexPinNotifier();
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
CBTObexPinNotifier::CBTObexPinNotifier()
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTObexPinNotifier::~CBTObexPinNotifier()
    {
    }

// ----------------------------------------------------------
// CBTObexPinNotifier::RegisterL
// ----------------------------------------------------------
//
CBTObexPinNotifier::TNotifierInfo CBTObexPinNotifier::RegisterL()
    {
    iInfo.iUid=KBTObexPasskeyQueryNotifierUid;
    iInfo.iChannel=KBTObexPINChannel;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTObexPinNotifier::GetParamsL
// Initialize parameters. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTObexPinNotifier::GetParamsL(const TDesC8& /* aBuffer */, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    FLOG(_L("[BTNOTIF]\t CBTObexPinNotifier::GetParamsL()"));

    if( !iMessage.IsNull())
        {
        User::Leave(KErrInUse);
        }
    else if ( AutoLockOnL() )
        {
        // The phone is locked, access denied.
        //
        CompleteMessage(KErrCancel);
        return;
        }
        
    iMessage = aMessage;
    iReplySlot = aReplySlot;
    
    ShowNoteCompleteMessageL();

    FLOG(_L("[BTNOTIF]\t CBTObexPinNotifier::GetParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTObexPinNotifier::ShowNoteCompleteMessageL
// Ask user response and return it to caller.
// ----------------------------------------------------------
//
void CBTObexPinNotifier::ShowNoteCompleteMessageL()
    {
    FLOG(_L("[BTNOTIF]\t CBTObexPinNotifier::ShowNoteCompleteMessageL()"));

    TBuf<KBTObexPasskeyMaxLength> tempPasskeyBuffer;    // Unicode buffer

    TInt keypress = iNotifUiUtil->ShowTextInputQueryL( tempPasskeyBuffer, 
            R_BT_OBEX_PASSKEY_QUERY, ECmdShowBtOpenCoverNote );
		
    if( keypress ) // OK pressed
        {
        // Convert data from unicode to 8 bit and write it back to caller
        //
        TBTObexPasskey obexPasskey;
        obexPasskey.Zero();
        CnvUtfConverter::ConvertFromUnicodeToUtf8(obexPasskey, tempPasskeyBuffer);
        CompleteMessage(TPckgBuf<TBTObexPasskey>(obexPasskey), KErrNone);
        }
    else  // Cancel pressed
        {
        CompleteMessage(KErrCancel);
        }	

    FLOG(_L("[BTNOTIF]\t CBTObexPinNotifier::ShowNoteCompleteMessageL() completed"));
    }


// End of File
