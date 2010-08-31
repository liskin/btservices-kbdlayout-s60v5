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
* Description: Bluetooth connection tracker and manager.
*
*/

#include "btnotifconnectiontracker.h"
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif

#include <btservices/btdevextension.h>
#include <btservices/btdevrepository.h>

#include "btnotifsession.h"
#include "btnotifclientserver.h"
#include "bluetoothtrace.h"
#include "btnotifsecuritymanager.h"

/**  Time window for determining if there are too many requests. */
#ifndef __WINS__
#define KDENYTHRESHOLD TTimeIntervalSeconds(3)
#else   //__WINS__
#define KDENYTHRESHOLD TTimeIntervalSeconds(5)
#endif  //__WINS__


// ======== LOCAL FUNCTIONS ========

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker::CBTNotifConnectionTracker( CBTNotifServer* aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::ConstructL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    // Open a handle to the registry server
    User::LeaveIfError( iBTRegistrySession.Connect() );
    // Open a handle to the socket server
    User::LeaveIfError( iSockServ.Connect() );
    iPairingManager = CBTNotifSecurityManager::NewL(*this, iServer->DevRepository() );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }


// ---------------------------------------------------------------------------
// NewL.
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker* CBTNotifConnectionTracker::NewL( CBTNotifServer* aServer )
    {
    CBTNotifConnectionTracker* self = new( ELeave ) CBTNotifConnectionTracker( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker::~CBTNotifConnectionTracker()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iDeniedRequests.Close();
    delete iPairingManager;
    iSockServ.Close();
    iBTRegistrySession.Close();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Process notifier message related to pairing notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleNotifierRequestL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    iPairingManager->HandleNotifierRequestL(aMessage);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Handle a request related to pairing.
// ---------------------------------------------------------------------------
//
void CBTNotifConnectionTracker::HandleBondingRequestL( const RMessage2& aMessage )
    {
    BOstraceFunctionEntryExt ( DUMMY_LIST, this, aMessage.Function() );
    iPairingManager->HandleBondingRequestL(aMessage);
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Check if this device has been denied a connection already before.
// Also check if a previous connection attempt has just been rejected.
// ---------------------------------------------------------------------------
//
TBool CBTNotifConnectionTracker::UpdateBlockingHistoryL( const CBTDevice* aDevice, 
    TBool aAccepted )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    __ASSERT_ALWAYS( aDevice, PanicServer( EBTNotifPanicBadArgument ) );
    // Check the time since the previous event.
    TBool result = RecordConnectionAttempts( aAccepted );
    TInt pos = iDeniedRequests.Find( aDevice->BDAddr() );
    if( !aAccepted )
        {
        if( pos == KErrNotFound )
            {
            // The user denied the request from a new device, record the device address.
            if( aDevice->IsValidPaired() && aDevice->IsPaired() )
                //[MCL]: && iDevice->LinkKeyType() != ELinkKeyUnauthenticatedUpgradable )
                {
                // Paired devices are allowed one time rejection without a prompt for blocking.
                result = EFalse;
                }
            iDeniedRequests.AppendL( aDevice->BDAddr() );
            }
        // Nothing needed here if the address is already in the array.
        }
    else if( pos > KErrNotFound )
        {
        // The user accepted a request, and it was from a device he/she 
        // previously rejected. Clear the history for this device from the array.
        iDeniedRequests.Remove( pos );
        }
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    return result;
    }

// ---------------------------------------------------------------------------
// Record and check the time between connection attempts.
// ---------------------------------------------------------------------------
//
TBool CBTNotifConnectionTracker::RecordConnectionAttempts( TBool aAccepted )
    {
    BOstraceFunctionEntry1( DUMMY_DEVLIST, this );
    TBool result = ETrue;
    TTime now( 0 );
    if( !aAccepted )
        {
        now.UniversalTime();
        if( iLastReject )
            {
            // Check the time between denied connections, that it does not go too fast.
            TTimeIntervalSeconds prev( 0 );
            if( !now.SecondsFrom( TTime( iLastReject ), prev ) )
                {
                if( prev <= KDENYTHRESHOLD )
                    {
                    // We are getting the requests too fast. Present the user with
                    // an option to turn BT off.
                    //iServer->SettingsTracker()->SetPower( EFalse );
                    result = EFalse;
                    }
                }
            }
        }
    // Record the current timestamp.
    // It is reset in case the user accepted the request.
    iLastReject = now.Int64();
    BOstraceFunctionExitExt( DUMMY_DEVLIST, this, result );
    return result;
    }

