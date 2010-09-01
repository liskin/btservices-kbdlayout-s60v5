/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Server-side implementation of BTEng
*
*/


#include <e32base.h>
#include <btmanclient.h>
#include <es_sock.h>
#include <btnotif.h>
#include <utf.h>
#include <ecom/ecom.h>
#include <centralrepository.h>
#include <btengdomaincrkeys.h>
#include <bt_subscribe.h>
#include "btengserver.h"
#include "btengsrvstate.h"
#include "btengsrvsession.h"
#include "btengsrvpluginmgr.h"
#include "btengsrvbbconnectionmgr.h"
#include "btengsrvsettingsmgr.h"
#include "btengsrvkeywatcher.h"
#include "btengsdpdbhandler.h"
#include "btengclientserver.h"
#include "btengsecpolicy.h"
#include "btengprivatepskeys.h"
#include "btengplugin.h"
#include "btengpairman.h"
#include "debug.h"

/**  Bluetooth Engine server thread name */
_LIT( KBTEngThreadName, "BTEngine" );
/**  Timeout (3 sec) for shutting down the server (when power is off and no clients connected). */
const TInt KBTEngSrvIdleTimeout = 3000000;
/**  PubSub key read and write policies */
_LIT_SECURITY_POLICY_C2( KBTEngPSKeyReadPolicy, ECapabilityLocalServices, ECapabilityReadDeviceData );
_LIT_SECURITY_POLICY_C2( KBTEngPSKeyWritePolicy, ECapabilityLocalServices, ECapabilityWriteDeviceData );


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructs and returns an application object.
// ---------------------------------------------------------------------------
//
void RunServerL()
    {
    TRACE_FUNC_ENTRY
    User::RenameThread( KBTEngThreadName );
        // Create and install the active scheduler for this thread.
    CActiveScheduler* scheduler = new( ELeave ) CActiveScheduler();
    CleanupStack::PushL( scheduler );
    CActiveScheduler::Install( scheduler );
        // create the server (leave it on the cleanup stack)
	CBTEngServer* btServer = CBTEngServer::NewLC();
        // Initialisation complete, now signal the client
    RProcess::Rendezvous( KErrNone );
        // The server is not up and running.
    TRACE_INFO( ( _L( "[BTENG]\t BTEng server now up and running" ) ) )
	    // The active scheduler runs during the lifetime of this thread.
    CActiveScheduler::Start();
        // Cleanup the server and scheduler.
    CleanupStack::PopAndDestroy( btServer );
    CleanupStack::PopAndDestroy( scheduler );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Completes the message and panics the client.
// ---------------------------------------------------------------------------
//
void PanicClient( const RMessage2& aMessage, TInt aPanic )
    {
    TRACE_INFO( ( _L( "[BTENG]\t PanicClient: Reason %d" ), aPanic ) )
    aMessage.Panic( KBTEngPanic, aPanic );
    }


// ---------------------------------------------------------------------------
// Panic the server.
// ---------------------------------------------------------------------------
//
void PanicServer( TInt aPanic )
    {
    TRACE_INFO( ( _L( "[BTENG]\t PanicClient: Reason %d" ), aPanic ) )
    User::Panic( KBTEngPanic, aPanic );
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// (Priority is not as high as in prev. architecture, but should be enough)
// ---------------------------------------------------------------------------
//
CBTEngServer::CBTEngServer()
:   CPolicyServer( EPriorityHigh, KBTEngServerPolicy )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngServer::ConstructL()
    {
    TRACE_FUNC_ENTRY
        // No need to check _BT flag here, as the server can 
        // only be started by BTEng components.

        // Add the server to the active scheduler (from CServer2):
    StartL( KBTEngServerName );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothTestingMode, KBTDutEnabled, 
                                            RProperty::EInt, KBTEngPSKeyReadPolicy, 
                                            KBTEngPSKeyWritePolicy ) );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothTestingMode, KBTSspDebugmode, 
                                            RProperty::EInt, KBTEngPSKeyReadPolicy, 
                                            KBTEngPSKeyWritePolicy ) );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothEnginePrivateCategory, 
                                            KBTBlockDevAddr, RProperty::EText, 
                                            KBTEngPSKeyReadPolicy, 
                                            KBTEngPSKeyWritePolicy ) );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothEnginePrivateCategory,
                                            KBTOutgoingPairing,
                                            RProperty::EByteArray,
                                            KBTEngPSKeyReadPolicy,
                                            KBTEngPSKeyWritePolicy) );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothEnginePrivateCategory,
                                            KBTConnectionTimeStamp,
                                            RProperty::EByteArray,
                                            KBTEngPSKeyReadPolicy,
                                            KBTEngPSKeyWritePolicy) );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothEnginePrivateCategory,
                                            KBTTurnBTOffQueryOn,
                                            RProperty::EInt,
                                            KBTEngPSKeyReadPolicy,
                                            KBTEngPSKeyWritePolicy) );
    User::LeaveIfError( RProperty::Define( KPSUidBluetoothEnginePrivateCategory,
                                            KBTNotifierLocks,
                                            RProperty::EByteArray,
                                            KBTEngPSKeyReadPolicy,
                                            KBTEngPSKeyWritePolicy) );    
    
    User::LeaveIfError( iSocketServ.Connect() );

        // The server is only started by its client, so leave the state 
        // machine in Init state. A request to turn power on will follow 
        // usually immediately.
    iServerState = CBTEngSrvState::NewL( this );
    iWatcher = CBTEngSrvKeyWatcher::NewL( this );
    iSettingsMgr = CBTEngSrvSettingsMgr::NewL( this );
    iPluginMgr = CBTEngSrvPluginMgr::NewL( this );
    iBBConnMgr = CBTEngSrvBBConnMgr::NewL( this, iSocketServ );

    User::LeaveIfError( iBTRegServ.Connect() );
    iPairMan = CBTEngPairMan::NewL( *this );

    TCallBack idleCb( IdleTimerCallBack, this );
    iIdleCallBack.Set( idleCb );
    TCallBack sspCb( DebugModeTimerCallBack, this );
    iDebugModeCallBack.Set( sspCb );
    TCallBack scanModeCb( ScanModeTimerCallBack, this );
    iScanModeCallBack.Set( scanModeCb );
    TCallBack powerOffCb( AutoPowerOffCallBack, this );
    iPowerOffCallBack.Set( powerOffCb );
    iTimer = CDeltaTimer::NewL(CActive::EPriorityLow);
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngServer* CBTEngServer::NewLC()
    {
    CBTEngServer* self = new( ELeave ) CBTEngServer();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngServer::~CBTEngServer()
    {
    TRACE_FUNC_ENTRY
    if( iTimer )
        {
        iTimerQueued = ENone;
        iTimer->Remove( iScanModeCallBack );
        iTimer->Remove( iPowerOffCallBack );
        iTimer->Remove( iIdleCallBack );
        iTimer->Remove( iDebugModeCallBack );
        }
    RProperty::Delete( KPSUidBluetoothTestingMode, KBTDutEnabled );
    RProperty::Delete( KPSUidBluetoothTestingMode, KBTSspDebugmode );
    RProperty::Delete( KPSUidBluetoothEnginePrivateCategory, KBTBlockDevAddr );
    RProperty::Delete( KPSUidBluetoothEnginePrivateCategory, KBTOutgoingPairing );
    RProperty::Delete( KPSUidBluetoothEnginePrivateCategory, KBTConnectionTimeStamp );
    RProperty::Delete( KPSUidBluetoothEnginePrivateCategory, KBTTurnBTOffQueryOn );
    RProperty::Delete( KPSUidBluetoothEnginePrivateCategory, KBTNotifierLocks );
    delete iTimer;
    delete iSdpDbHandler;
    delete iWatcher;
    delete iSettingsMgr;
    delete iPluginMgr;
    delete iBBConnMgr;
    delete iServerState;
    delete iPairMan;
    iSocketServ.Close();
    iBTRegServ.Close();
    }

// ---------------------------------------------------------------------------
// Turn BT on or off.
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetPowerStateL( TBTPowerState aState, TBool aTemporary )
    {
    TRACE_FUNC_ARG( ( _L( "setting power state %d (temporary=%d" ), (TInt) aState, aTemporary ) )
    iSettingsMgr->SetPowerStateL( aState, aTemporary );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Turn BT on or off.
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetPowerStateL(const RMessage2 aMessage )
    {
    TRACE_FUNC_ENTRY
    iSettingsMgr->SetPowerStateL( aMessage );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::DisconnectAllForPowerOffL()
    {
    TRACE_FUNC_ENTRY
    TCallBack cb( DisconnectAllCallBack, this );
    iBBConnMgr->DisconnectAllLinksForPowerOffL( cb );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::DisconnectAllCompleted()
    {
        // Check if we are powering off. Otherwise we have just been 
        // requested to disconnect all, e.g. for system shutdown.
    if( iServerState->CurrentOperation() == CBTEngSrvState::EPowerOff )
        {
        (void) iSettingsMgr->SetHwPowerState( EBTOff );
            // Put the state machine into idle state.
        iServerState->ChangeState();
        }
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::DispatchPluginMessageL( const RMessage2& aMessage )
    {
    iPluginMgr->ProcessCommandL( aMessage );
    }


// ---------------------------------------------------------------------------
// A new session has been added, update the session count.
// ---------------------------------------------------------------------------
//
void CBTEngServer::AddSession()
    {
    iSessionCount++;
        // Remove the idle timer.
    iTimerQueued &= ~EIdleTimer;
    iTimer->Remove( iIdleCallBack );
    }


// ---------------------------------------------------------------------------
// A session has been ended, update the session count and inform others.
// ---------------------------------------------------------------------------
//
void CBTEngServer::RemoveSession( CSession2* aSession, TBool aAutoOff )
    {
	TRACE_FUNC_ENTRY
	TRACE_INFO( ( _L( "[CBTEngServer]\t aAutoOff %d"), aAutoOff ))
	TRACE_INFO( ( _L( "[CBTEngServer]\t iSessionCount %d"), iSessionCount ))
    iSessionCount--;
	iSettingsMgr->SessionClosed( aSession );
	iPairMan->SessionClosed( aSession );
    if( aAutoOff )
        {
        TRAP_IGNORE( SetPowerStateL( EBTOff, ETrue ) );
        }
    else
        {
        CheckIdle();
        }
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Queue a new timer.
// ---------------------------------------------------------------------------
//
void CBTEngServer::QueueTimer( CBTEngServer::TTimerQueued aTimer, TInt64 aInterval )
    {
    TRACE_FUNC_ARG( ( _L( "queueing timer %d" ), (TInt) aTimer ) )
    __ASSERT_DEBUG( iTimer, PanicServer( EBTEngPanicMemberVarIsNull ) );
    iTimerQueued |= aTimer;
    TTimeIntervalMicroSeconds interval( aInterval );
    switch( aTimer )
        {
        case EScanModeTimer:
            (void) iTimer->QueueLong( interval, iScanModeCallBack );
            break;
        case EIdleTimer:
            iTimer->QueueLong( aInterval, iIdleCallBack );
            break;
        case EAutoPowerOffTimer:
            iTimer->QueueLong( aInterval, iPowerOffCallBack );
            break;
        case ESspDebugModeTimer:
            iTimer->QueueLong( aInterval, iDebugModeCallBack );
            break;
        default:
            PanicServer( EBTEngPanicCorrupt );
        }
    }


// ---------------------------------------------------------------------------
// Remove a queued timer.
// ---------------------------------------------------------------------------
//
void CBTEngServer::RemoveTimer( CBTEngServer::TTimerQueued aTimer )
    {
    TRACE_FUNC_ARG( ( _L( "removing timer %d" ), (TInt) aTimer ) )
    if(!iTimer)
        {
        return; // not fully constructed yet, don't do anything
        }
    iTimerQueued &= ~aTimer;
    // Timers can be removed without being queued, no need to check.
    switch( aTimer )
        {
        case EScanModeTimer:
            iTimer->Remove( iScanModeCallBack );
            break;
        case EIdleTimer:
            iTimer->Remove( iIdleCallBack );
            break;
        case EAutoPowerOffTimer:
            iTimer->Remove( iPowerOffCallBack );
            break;
        case ESspDebugModeTimer:
            iTimer->Remove( iDebugModeCallBack );
            break;
        default:
            PanicServer( EBTEngPanicCorrupt );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class CPolicyServer.
// Checks version compatibility and constructs a new session object.
// ---------------------------------------------------------------------------
//
CSession2* CBTEngServer::NewSessionL( const TVersion& aVersion, 
    const RMessage2& aMessage ) const
    {
    TRACE_FUNC_ENTRY
    (void) aMessage;
    // Compare our version with client-side version, CServer2 requires that 
    // we leave if they are not compatible. 
    // Compatible is interpreted here as equal or greater than our version.
    TVersion srvVersion( KBTEngServerVersionMajor, KBTEngServerVersionMinor, 
                          KBTEngServerVersionBuild );

    if( !User::QueryVersionSupported( aVersion, srvVersion ) )
        {
        // EFalse is returned if our version is not less than or 
        // equal to the client version.
        User::Leave( KErrNotSupported );
        }
    return CBTEngSrvSession::NewL();
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::CheckIdle()
    {
    TRACE_FUNC_ENTRY
    __ASSERT_DEBUG( iTimer, PanicServer( EBTEngPanicMemberVarIsNull ) );
    iTimerQueued &= ~EIdleTimer;
    iTimer->Remove( iIdleCallBack );
    if( iSessionCount <= 0 )
        {
            // No more sessions, check the power state.
        TBTPowerState power = EBTOff;
        TInt err = iSettingsMgr->GetHwPowerState( power );
        TRACE_INFO( ( _L( "[BTEng]\t No sessions; power state: %d" ), power ) )
        if( !err && power == EBTOff
            && iServerState->CurrentOperation() == CBTEngSrvState::ESrvOpIdle )
            {
            TRACE_INFO( ( _L( "[BTEng]\t Power off; starting shutdown timer" ) ) )
                // power is off, start the shutdown timer.
            TTimeIntervalMicroSeconds32 interval = KBTEngSrvIdleTimeout;
            iTimer->Queue( interval, iIdleCallBack );
            iTimerQueued |= EIdleTimer;
            }
        }
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::ManageDIServiceL( TBool aState )
    {
    TRACE_FUNC_ENTRY
        // The session with the SDP database is kept open during the 
        // lifetime of BTEng server. This ensures that the database server 
        // will not destroy itself (and the SDP record) when there are no 
        // sessions anymore (which it would do). This also ensures that other 
        // sessions (of BT Engine Discovery API) can be closed without the SDP
        // database being destroyed, and so saving memory.
    TInt err = KErrNone;
    if( !iSdpDbHandler )
        {
        iSdpDbHandler = CBTEngSdpDbHandler::NewL();
        }
    if( aState && !iDiSdpRecHandle )
        {
            // Use TRAP here, because the function will leave if 
            // the required CenRep keys are not available.
        TInt vendorId = 0;
        TInt productId = 0;
        TRAP( err, GetProductIdsL( vendorId, productId ) );
#ifdef  __WINS__
        err = KErrNone; // Ignore error and load with S60 RnD default values.
#endif  //__WINS__
        if( !err )
            {
            TUUID uuid( EBTProfileDI );
            iSdpDbHandler->RegisterSdpRecordL( uuid, vendorId, productId, 
                                                iDiSdpRecHandle );
            }
        }
    if( ( !aState || err ) && iDiSdpRecHandle )
        {
            // Either we are shutting down, or we did not manage 
            // to fill the record with required data -> delete it.
        iSdpDbHandler->DeleteSdpRecordL( iDiSdpRecHandle );
        iDiSdpRecHandle = 0;
        delete iSdpDbHandler;
        iSdpDbHandler = NULL;
        }
    User::LeaveIfError( err );  // To pass back the result of the operation.
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::GetProductIdsL( TInt& aVendorId, TInt& aProductId )
    {
    TRACE_FUNC_ENTRY
    CRepository* cenRep = CRepository::NewLC( KCRUidBluetoothEngine );
    User::LeaveIfError( cenRep->Get( KBTVendorID, aVendorId ) );
    User::LeaveIfError( cenRep->Get( KBTProductID, aProductId ) );
    CleanupStack::PopAndDestroy( cenRep );  //cenRep
    }


// ---------------------------------------------------------------------------
// Ask plugin manager the connection status of the specified device
// ---------------------------------------------------------------------------
//
TBTEngConnectionStatus CBTEngServer::IsDeviceConnected( const TBTDevAddr& aAddr )
    {
    TBTEngConnectionStatus status = EBTEngNotConnected;
    if ( iPluginMgr )
        {
        status = iPluginMgr->IsDeviceConnected( aAddr );
        }
    return status;
    }


// ---------------------------------------------------------------------------
// Static callback for temporary visibility mode.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::ScanModeTimerCallBack( TAny* aPtr )
    {
    __ASSERT_ALWAYS(aPtr, PanicServer(EBTEngPanicArgumentIsNull) );
    // The timer has completed, so remove our reference as well.
    CBTEngServer* server = (CBTEngServer*) aPtr;
    server->RemoveTimer( EScanModeTimer );
    TRAPD( err, server->SettingsManager()->ScanModeTimerCompletedL() );
    return err;
    }


// ---------------------------------------------------------------------------
// Static callback for disconnecting all Baseband connections.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::DisconnectAllCallBack( TAny* aPtr )
    {
    __ASSERT_ALWAYS(aPtr, PanicServer(EBTEngPanicArgumentIsNull) );
    ( (CBTEngServer*) aPtr )->DisconnectAllCompleted();
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Static callback for idle timer timeout. We are shutting down ourselves.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::IdleTimerCallBack( TAny* aPtr )
    {
    (void) aPtr;
    CActiveScheduler::Stop();   // Will destroy CBTEngServer
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Static callback for idle timer timeout. Turn off BT to get it out of 
// debug mode. If there are existing connections, queue the timer again.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::DebugModeTimerCallBack( TAny* aPtr )
    {
    __ASSERT_ALWAYS(aPtr, PanicServer(EBTEngPanicArgumentIsNull) );
    // Set our internal debug mode key to off. Ignore error, not critical here.
    (void) RProperty::Set( KPSUidBluetoothTestingMode, KBTSspDebugmode, EFalse );
    CBTEngServer* server = (CBTEngServer*) aPtr;
    TRAP_IGNORE( server->SettingsManager()->CheckSspDebugModeL( EFalse ) );
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Static callback for auto power off.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::AutoPowerOffCallBack( TAny* aPtr )
    {
    __ASSERT_ALWAYS(aPtr, PanicServer(EBTEngPanicArgumentIsNull) );
    // The timer has completed, so remove our reference as well.
    CBTEngServer* server = (CBTEngServer*) aPtr;
    server->RemoveTimer( EAutoPowerOffTimer );
    TRAPD( err, server->SettingsManager()->CheckAutoPowerOffL() );
    return err;
    }


// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// Main function of the executable.
// ---------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
    {
    __UHEAP_MARK;
    TRACE_FUNC_ENTRY
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt err = KErrNoMemory;
    if ( cleanup )
        {
        TRAP( err, RunServerL() );
        delete cleanup;
        }
    __UHEAP_MARKEND;
    return err;
    }
