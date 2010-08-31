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
* Description: Session class for handling commands from clients.
*
*/

#include "btnotifsession.h"
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif
#include "btnotifclientserver.h"
#include "btnotifsettingstracker.h"
#include "btnotifconnectiontracker.h"
#include "btnotifdeviceselector.h"


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Start the server.
// ---------------------------------------------------------------------------
//
void LeaveIfNullL( const TAny* aPtr, TInt aLeaveCode )
    {
    if( aPtr == NULL )
        {
        User::Leave( aLeaveCode );
        }
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifSession::CBTNotifSession()
:   CSession2()
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifSession::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifSession* CBTNotifSession::NewL()
    {
    CBTNotifSession* self = new( ELeave ) CBTNotifSession();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifSession::~CBTNotifSession()
    {
    // clients must complete the message they are responsible for
    // we do not complete any message here
    Server()->RemoveSession();
    }

// ---------------------------------------------------------------------------
// From class CSession2.
// Receives a message from a client.
// ---------------------------------------------------------------------------
//
void CBTNotifSession::ServiceL( const RMessage2& aMessage )
    {
    CBTNotifConnectionTracker* connTracker = Server()->ConnectionTracker();
    TInt opCode = aMessage.Function();
    TInt uid = aMessage.Int0();
    TInt err( KErrNotReady );
    switch(opCode){
        case EBTNotifCancelNotifier:
        case EBTNotifStartSyncNotifier:
        case EBTNotifStartAsyncNotifier:
        case EBTNotifUpdateNotifier:
            if( uid == KDeviceSelectionNotifierUid.iUid )
                {
                TRAP( err, {
                        CBTNotifDeviceSelector& selector = Server()->DeviceSelectorL();
                        selector.DispatchNotifierMessageL( aMessage ); }
                        );
                if ( err )
                    {
                    aMessage.Complete( err );
                    }
                    // deviceselector takes the ownership of aMessage.
                }
            else
                {
                // PIN/ SSP pairing notifiers from BT stack:
                // ***** Note for implementers:
                // message queue is not used for this notifier handling.
                if ( uid == KBTManAuthNotifierUid.iUid ||
                     uid == KBTManPinNotifierUid.iUid ||
                     uid == KBTPinCodeEntryNotifierUid.iUid ||
                     uid == KBTNumericComparisonNotifierUid.iUid ||
                     uid == KBTPasskeyDisplayNotifierUid.iUid ||
                     uid == KBTUserConfirmationNotifierUid.iUid )
                    {
                    if( connTracker )
                        {
                         // Pass it to the connection tracker.
                         TRAP( err, 
                                 connTracker->HandleNotifierRequestL( aMessage ) );
                        }
                    if ( err )
                        {
                        // tracker not available, can't do this now.
                        aMessage.Complete( err );
                        }
                    }
                }
            break;
        case EBTNotifPairDevice:
        case EBTNotifCancelPairDevice:
            // Pairing requests from clients:
            if ( connTracker )
                {
                TRAP( err, connTracker->HandleBondingRequestL( aMessage ) );
                }
            if ( err )
                {
                // tracker not available, can't do this now.
                aMessage.Complete( err );            
                }
            break;
        default:
            aMessage.Complete( KErrNotSupported );
            break;
    }
}


// ---------------------------------------------------------------------------
// From class CSession2.
// Completes construction of the session.
// ---------------------------------------------------------------------------
//
void CBTNotifSession::CreateL()
    {
    Server()->AddSession();
    }

