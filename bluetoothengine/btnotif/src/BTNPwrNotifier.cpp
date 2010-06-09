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
* Description:  Declares power mode setting notifier class.
*
*/


// INCLUDE FILES
#include <BTNotif.rsg>       // Own resources
#include "btnpwrnotifier.h"  // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include <bt_subscribe.h>
#include <SecondaryDisplay/BTnotifSecondaryDisplayAPI.h>


// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTPwrNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTPwrNotifier* CBTPwrNotifier::NewL()
    {
    CBTPwrNotifier* self=new (ELeave) CBTPwrNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTPwrNotifier::CBTPwrNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTPwrNotifier::CBTPwrNotifier()
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTPwrNotifier::~CBTPwrNotifier()
    {
    }

// ----------------------------------------------------------
// CBTPwrNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTPwrNotifier::TNotifierInfo CBTPwrNotifier::RegisterL()
    {
    iInfo.iUid=KPowerModeSettingNotifierUid;
    iInfo.iChannel=KBTPowerModeChannel;
    iInfo.iPriority=ENotifierPriorityVHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTPwrNotifier::GetParamsL
// Initialize parameters. Jump to RunL as soon as possible.
// ----------------------------------------------------------
//
void CBTPwrNotifier::GetParamsL(const TDesC8& /*aBuffer*/, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    FLOG(_L("[BTNOTIF]\t CBTPwrNotifier::GetParamsL()"));

    if( iReplySlot!=NULL || !iMessage.IsNull() )
        User::Leave(KErrInUse);

    iMessage = aMessage;
    iReplySlot = aReplySlot;

    ShowNoteAndCompleteMessageL();
    FLOG(_L("[BTNOTIF]\t CBTPwrNotifier::GetParamsL() completed"));
    }

// ----------------------------------------------------------
// CBTPwrNotifier::RunL
// Ask user response and return it to caller.
// ----------------------------------------------------------
//
void CBTPwrNotifier::ShowNoteAndCompleteMessageL()
    {
    FLOG(_L("[BTNOTIF]\t CBTPwrNotifier::ShowNoteAndCompleteMessageL()"));

    if ( !CheckAndSetPowerOnL() )
        {
        CompleteMessage(KErrGeneral);
        return;
        }
    // TODO: Why do we show BT BUSY note from BT power notifier?
    //get connection amount
    TInt linkCount = 0;
    RProperty::Get(KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetPHYCount, linkCount);	
        
    if (linkCount > 6)   //support only piconet
        {
        iNotifUiUtil->ShowInfoNoteL( R_BT_BUSY_NOTE, ECmdShowBtBusyNote );
        }
	CompleteMessage(ETrue, KErrNone );
    FLOG(_L("[BTNOTIF]\t CBTPwrNotifier::ShowNoteAndCompleteMessageL() completed"));
    }

// End of File
