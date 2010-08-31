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
* Description: Server class for handling commands from clients, and the 
*                central class in btnotif thread.
*
*/

#include "btnotifserver.h"
#include <btservices/btdevrepository.h>
#include "btnotifsession.h"
#include "btnotifconnectiontracker.h"
#include "btnotifsettingstracker.h"
#include "btnotificationmanager.h"
#include "btnotifdeviceselector.h"
#include "btnotifserversecpolicy.h"
#include "btnotifclientserver.h"

/**  Panic category */
_LIT( KBTNotifPanic, "BTNotif panic" );

/**  Timeout (10 sec) for shutting down the server 
 * (when BT power is off and no clients connected). */
const TInt KBTNtoifShutdownTimeout = 10 * 1000 * 1000;

// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Start the server.
// ---------------------------------------------------------------------------
//
static void RunServerL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    
    (void) User::RenameThread( KBTNotifServerName );
    // Create and install the active scheduler for this thread.
    CActiveScheduler* scheduler = new( ELeave ) CActiveScheduler();
    CleanupStack::PushL( scheduler );
    CActiveScheduler::Install( scheduler );
    // create the server (and leave it on the cleanup stack)
    CBTNotifServer* notifServer = CBTNotifServer::NewLC();
    // Initialisation complete, now signal the client
    RProcess::Rendezvous( KErrNone );
        // The server is now up and running.
    BOstrace0( TRACE_NORMAL, DUMMY_DEVLIST, "[BTNOTIF]\t BTNotif server now up and running" );
    // The active scheduler runs during the lifetime of this thread.
    CActiveScheduler::Start();
    // Stopping the active scheduler means terminating the thread.
    // Cleanup the server and scheduler.
    CleanupStack::PopAndDestroy( notifServer );
    CleanupStack::PopAndDestroy( scheduler );
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// Panic the server.
// ---------------------------------------------------------------------------
//
void PanicServer( TInt aReason )
    {
    User::Panic( KBTNotifPanic, aReason );
    }

// ---------------------------------------------------------------------------
// Panic the client through the client-side message.
// ---------------------------------------------------------------------------
//
void PanicClient( const RMessage2& aMessage, TInt aReason )
    {
    aMessage.Panic( KBTNotifPanic, aReason );
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifServer::CBTNotifServer()
:   CPolicyServer( EPriorityUserInput, KBTNotifServerPolicy )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifServer::ConstructL()
    {
    // Add the server to the active scheduler (from CServer2):
    StartL( KBTNotifServerName );
    iAsyncCb = new( ELeave ) CAsyncCallBack( EPriorityHigh );
    TCallBack cb( AsyncConstructCb, this );
    iAsyncCb->Set( cb );
    iAsyncCb->CallBack();
    }

// ---------------------------------------------------------------------------
// Asynchronous 3rd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifServer::AsyncConstructL()
    {
    // The server class owns this registry object and provides it
    // as a singleton in the whole server process.
    // Server itself does not handle any registry events.
    // Classes that want to receive these events must register
    // observers via CBtDevRepository interface.
    iDevRep = CBtDevRepository::NewL();
    iNotificationMgr = CBTNotificationManager::NewL( this );
    iSettingsTracker = CBTNotifSettingsTracker::NewL( this );
    iConnectionTracker = CBTNotifConnectionTracker::NewL( this );
    iTimer = CDeltaTimer::NewL(CActive::EPriorityLow);
    TCallBack shutdownCb( ShutdownTimeout, this );
    iShutdownTimerEntry.Set( shutdownCb );
    }

// ---------------------------------------------------------------------------
// Callback for asynchronous construction.
// ---------------------------------------------------------------------------
//
TInt CBTNotifServer::AsyncConstructCb( TAny* aPtr )
    {
    TRAPD( err, ( (CBTNotifServer*) aPtr )->AsyncConstructL() );
    return err;
    }


// ---------------------------------------------------------------------------
// NewLC.
// ---------------------------------------------------------------------------
//
CBTNotifServer* CBTNotifServer::NewLC()
    {
    CBTNotifServer* self = new( ELeave ) CBTNotifServer();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifServer::~CBTNotifServer()
    {
    delete iDevSelector;
    delete iSettingsTracker;
    delete iConnectionTracker;
    delete iNotificationMgr;
    delete iAsyncCb;
    delete iTimer;
    delete iDevRep;
    }

// ---------------------------------------------------------------------------
// Handle a change in BT power state.
// ---------------------------------------------------------------------------
//
void CBTNotifServer::HandlePowerStateChangeL( TBTPowerStateValue aState )
    {
    CheckIdle( aState );
    }

// ---------------------------------------------------------------------------
// Increase the session count.
// ---------------------------------------------------------------------------
//
void CBTNotifServer::AddSession()
    {
    ++iSessionCount;
    iTimer->Remove( iShutdownTimerEntry );
    }


// ---------------------------------------------------------------------------
// Decrease the session count.
// ---------------------------------------------------------------------------
//
void CBTNotifServer::RemoveSession()
    {
    if ( iSessionCount > 0 )
        {
        // session counter can't be less than 0
        --iSessionCount;
        }
    CheckIdle( iSettingsTracker->GetPowerState() );
    }

// ---------------------------------------------------------------------------
// get the singleton instance of device repository
// ---------------------------------------------------------------------------
//
CBtDevRepository& CBTNotifServer::DevRepository()
    {
    return *iDevRep;
    }

// ---------------------------------------------------------------------------
// get the singleton instance of device search notifier
// ---------------------------------------------------------------------------
//
CBTNotifDeviceSelector& CBTNotifServer::DeviceSelectorL()
    {
    if ( ! iDevSelector )
        {
        iDevSelector = CBTNotifDeviceSelector::NewL( *this );
        }
    return *iDevSelector;
    }

// ---------------------------------------------------------------------------
// From class CPolicyServer.
// Create a new session object.
// ---------------------------------------------------------------------------
//
CSession2* CBTNotifServer::NewSessionL( const TVersion& aVersion, 
    const RMessage2& aMessage ) const
    {
    (void) aMessage;
    // Compare our version with client-side version, CServer2 requires that 
    // we leave if they are not compatible. 
    TVersion srvVersion( KBTNotifServerVersionMajor, KBTNotifServerVersionMinor, 
                          KBTNotifServerVersionBuild );

    if( !User::QueryVersionSupported( aVersion, srvVersion ) )
        {
        // EFalse is returned if our version is not less than or 
        // equal to the client version.
        User::Leave( KErrNotSupported );
        }
    return CBTNotifSession::NewL();
    }

void CBTNotifServer::CheckIdle( TBTPowerStateValue aState )
    {
    // In special scenarios, we do not have to remove the timer and queue it 
    // again, but these scenarios rarely happen in end-user use cases. 
    iTimer->Remove( iShutdownTimerEntry );
    if ( iSessionCount == 0 && aState == EBTPowerOff )
        {
        // BT power is off, start the shutdown timer.
        TTimeIntervalMicroSeconds32 interval = KBTNtoifShutdownTimeout;
        iTimer->Queue( interval, iShutdownTimerEntry );
        }
    }

TInt CBTNotifServer::ShutdownTimeout( TAny* aPtr )
    {
    (void) aPtr;
    CActiveScheduler::Stop();
    return KErrNone;    
    }

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Main function of the executable.
// ---------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    // Disabled until memory leak in QT/Open C are fixed
    // __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt err = KErrNoMemory;
    if ( cleanup )
        {
        TRAP( err, RunServerL() );
        delete cleanup;
        }
    // Disabled until memory leak in QT/Open C are fixed
    // __UHEAP_MARKEND;
    return err;
    }


