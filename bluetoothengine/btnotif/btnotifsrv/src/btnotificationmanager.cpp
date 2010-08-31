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
* Description: Class for managing user notification and query objects, 
* and for serializing access to the notification server.
*
*/

#include "btnotificationmanager.h"
#include "btnotifserver.h"

#include "bluetoothnotification.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotificationManager::CBTNotificationManager( const CBTNotifServer* aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotificationManager* CBTNotificationManager::NewL( const CBTNotifServer* aServer )
    {
    CBTNotificationManager* self = new( ELeave ) CBTNotificationManager( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotificationManager::~CBTNotificationManager()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iNotificationQ.ResetAndDestroy();
    iNotificationQ.Close();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Get a new notification
// ---------------------------------------------------------------------------
//
CBluetoothNotification* CBTNotificationManager::GetNotification()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    CBluetoothNotification* notification = NULL;
    TRAP_IGNORE( notification = CBluetoothNotification::NewL( this ) );
    if( notification )
        {
        if( iNotificationQ.Append( notification ) )
            {
            // In case the appending fails, we just delete the notification.
            // Otherwise we cannot keep track of it anymore.
            delete notification;
            notification = NULL;
            }
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    return notification;
    }


// ---------------------------------------------------------------------------
// Release the notification
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::ReleaseNotification( CBluetoothNotification* aNotification )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    __ASSERT_ALWAYS( aNotification, PanicServer( EBTNotifPanicBadArgument ) );
    TInt pos = iNotificationQ.Find( aNotification );
    __ASSERT_ALWAYS( pos > KErrNotFound, PanicServer( EBTNotifPanicMissing ) );
    delete iNotificationQ[pos];
    iNotificationQ.Remove( pos );
    iNotificationQ.Compress();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// Queue the notification with given priority
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::QueueNotificationL(
        CBluetoothNotification* aNotification,
        TNotificationPriority aPriority )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    (void) aPriority;
    TInt pos = iNotificationQ.Find( aNotification );
    __ASSERT_ALWAYS( pos > KErrNotFound, PanicServer( EBTNotifPanicMissing ) );
    // Always move the newly added notification on top
    if(pos != 0 )
        {
        CBluetoothNotification* notification = NULL;
        notification = iNotificationQ[pos];
        iNotificationQ.Remove( pos );
        iNotificationQ.InsertL(notification,0);
        }
    ProcessNotificationQueueL();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Process the notification queue and launch the next notification.
// ---------------------------------------------------------------------------
//
void CBTNotificationManager::ProcessNotificationQueueL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if( iNotificationQ.Count() )
        {
        iNotificationQ[0]->ShowL();
        }
    else
        {
        // No outstanding notifications
        iNotificationQ.Compress(); // the queue is empty, reset it.
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

