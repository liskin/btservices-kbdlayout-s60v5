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
* Description: Class for tracking Bluetooth settings, and also for 
* handling notes unrelated to specific connection.
*
*/

#include "btnotifsettingstracker.h"
#include <btserversdkcrkeys.h>

#include "btnotifserver.h"
#include "btnotificationmanager.h"
#include "btnotifclientserver.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifSettingsTracker::CBTNotifSettingsTracker( CBTNotifServer* aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::ConstructL()
    {
    iSettings = CBTEngSettings::NewL( this );
    User::LeaveIfError( iSettings->GetPowerState( iPowerState ) );
    }


// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifSettingsTracker* CBTNotifSettingsTracker::NewL( CBTNotifServer* aServer )
    {
    CBTNotifSettingsTracker* self = new( ELeave ) CBTNotifSettingsTracker( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifSettingsTracker::~CBTNotifSettingsTracker()
    {
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    delete iSettings;
    }


// ---------------------------------------------------------------------------
// Process a client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::DispatchNotifierMessageL( const RMessage2& aMessage )
    {
    TUid uid = TUid::Uid( aMessage.Int0() );
    (void) uid;
    }


// ---------------------------------------------------------------------------
// Cancels an outstanding client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::CancelNotifierMessageL( const RMessage2& aMessage )
    {
    (void) aMessage;
    }


// ---------------------------------------------------------------------------
// From class MBTEngSettingsObserver.
// Handle a power status change.
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::PowerStateChanged( TBTPowerStateValue aState )
    {
    iPowerState = aState;
    TRAP_IGNORE( iServer->HandlePowerStateChangeL( aState ) );
    }


// ---------------------------------------------------------------------------
// From class MBTEngSettingsObserver.
// Handle a visibility mode change.
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::VisibilityModeChanged( TBTVisibilityMode aState )
    {
    if( iVisibilityMode == EBTVisibilityModeTemporary && 
            aState == EBTVisibilityModeHidden && iPowerState == EBTPowerOn )
        {
        // Timeout expired, launch a note.
        // Note that we get the power state change before this one, when powering
		// off and setting visibility mode to hidden automatically.
        NOTIF_NOTHANDLED( !iNotification )
        iNotification = iServer->NotificationManager()->GetNotification();
        if ( iNotification )
            {
            iNotification->SetObserver( this );
            iNotification->SetNotificationType( TBluetoothDeviceDialog::ENote, EVisibilityTimeout );
            TRAP_IGNORE(
                    iServer->NotificationManager()->QueueNotificationL( iNotification ) );
            }
        }
    iVisibilityMode = aState;
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    (void) aData;
    }


// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifSettingsTracker::MBRNotificationClosed( TInt aError, const TDesC8& aData  )
    {
    (void) aError;
    (void) aData;
    iNotification->RemoveObserver();
    iNotification = NULL;
    }
