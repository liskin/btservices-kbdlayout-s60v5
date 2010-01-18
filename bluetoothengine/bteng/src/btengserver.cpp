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



#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <bthci.h>
#include <bt_subscribe_partner.h>
#endif

#include <e32base.h>
#include <btmanclient.h>
#include <es_sock.h>
#include <btnotif.h>
#include <utf.h>
#include <ecom/ecom.h>
#include <centralrepository.h>
#include <featmgr.h>
#include <AknSmallIndicator.h>
#include <avkon.hrh>
#include <bt_subscribe.h>
#include "btengserver.h"
#include "btengsrvstate.h"
#include "btengsrvsession.h"
#include "btengsrvpluginmgr.h"
#include "btengsrvbbconnectionmgr.h"
#include "btengsrvkeywatcher.h"
#include "btengsdpdbhandler.h"
#include "btengclientserver.h"
#include "btengsecpolicy.h"
#include "btengprivatecrkeys.h"
#include "btengprivatepskeys.h"
#include "btengplugin.h"
#include "btengpairman.h"
#include "debug.h"

/**  Bluetooth Engine server thread name */
_LIT( KBTEngThreadName, "BTEngine" );

/**  Constant for converting minutes to microseconds */
const TInt64 KMinutesInMicroSecs = MAKE_TINT64( 0, 60000000 );

/**  Idle timeout for shutting down the server (when power is off, 
 *   and no clients are connected). The value is 3 seconds.
 */
const TInt KBTEngSrvIdleTimeout = 3000000;

/**  Timeout for disabling Simple Pairing debug mode. The value is 30 minutes. */
const TInt KBTEngSspDebugModeTimeout = 1800000000;

/**  Timeout for determining that BT is not turned off automatically. 
 *   The value is 10.5 seconds.
 */
const TInt KBTEngBtAutoOffTimeout = 10500000;

/**  Enumeration of bitmask for keeping track of different timers. */
enum TTimerQueued
    {
    ENone               = 0x00,
    EScanModeTimer      = 0x01,
    EIdleTimer          = 0x02,
    EAutoPowerOffTimer  = 0x04,
    ESspDebugModeTimer  = 0x08 
    };

/**  PubSub key read and write policies */
_LIT_SECURITY_POLICY_C2( KBTEngPSKeyReadPolicy, 
                          ECapabilityLocalServices, ECapabilityReadDeviceData );
_LIT_SECURITY_POLICY_C2( KBTEngPSKeyWritePolicy, 
                          ECapabilityLocalServices, ECapabilityWriteDeviceData );

// Default values for Major and Minor Device Class
const TUint16 KCoDDefaultServiceClass = EMajorServiceTelephony | EMajorServiceObjectTransfer | EMajorServiceNetworking;
const TUint8 KCoDDefaultMajorDeviceClass = EMajorDevicePhone;
const TUint8 KCoDDefaultMinorDeviceClass = EMinorDevicePhoneSmartPhone;

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
    iEnableDutMode = EFalse;
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
    LoadBTPowerManagerL();
        // The server is only started by its client, so leave the state 
        // machine in Init state. A request to turn power on will follow 
        // usually immediately.
    iServerState = CBTEngSrvState::NewL( this );
    iWatcher = CBTEngSrvKeyWatcher::NewL( this );
    iPluginMgr = CBTEngSrvPluginMgr::NewL( this );
    iBBConnMgr = CBTEngSrvBBConnMgr::NewL( iSocketServ );
    
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
    
    iEnterpriseEnablementMode = BluetoothFeatures::EnterpriseEnablementL();
	TRACE_INFO( ( _L( "iEnterpriseEnablementMode = %d" ), iEnterpriseEnablementMode) )
    if ( iEnterpriseEnablementMode == BluetoothFeatures::EDisabled )
        {
        SetVisibilityModeL( EBTVisibilityModeNoScans, 0 );
        }
    
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
        iTimer->Remove( iIdleCallBack);
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
    delete iPluginMgr;
    delete iBBConnMgr;
    delete iServerState;
    delete iPairMan;
    iPowerMgr.Close();
    iSocketServ.Close();
    iBTRegServ.Close();
    }


// ---------------------------------------------------------------------------
// Turn BT on or off.
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetPowerStateL( TBTPowerStateValue aState, TBool aTemporary )
    {
    TRACE_FUNC_ARG( ( _L( "setting power state %d" ), (TInt) aState ) )
    if ( aState == EBTPowerOn && iEnterpriseEnablementMode == BluetoothFeatures::EDisabled )
        {
        TRACE_INFO( ( _L( "\tno we're not... Bluetooth is enterprise-IT-disabled" ) ) )
        User::Leave(KErrNotSupported);
        }
    
    TBTPowerStateValue currentState = EBTPowerOff;
    CheckTemporaryPowerStateL( currentState, aState, aTemporary );

    if( ( currentState == aState || ( aTemporary && aState == EBTPowerOff ) ) && iServerState->CurrentOperation() == CBTEngSrvState::ESrvOpIdle )
        {
		// The requested power state is already active, ignore silently.
		// We don't return an error here, as there is no error situation.
        TRACE_INFO( ( _L( "SetPowerStateL: nothing to do" ) ) )
        if(currentState == aState)
            {
            // Make sure that the CenRep key is in sync.
            // During boot-up, the pwoer is set from the CenRep key, so we could 
            // end up out-of-sync.
            TRACE_INFO( ( _L( "SetPowerStateL: currentState == aState" ) ) )
            UpdateCenRepPowerKeyL( aState );
            } 
        return;
        }
    if( aState )
        {
            // Hardware power on is the first step.
        User::LeaveIfError( SetPowerState( aState ) );
        }
    else
        {
        //Prevent BT visibility in the situation when we turn OFF BT Engine 
        //but FM Radio is still alive
        SetVisibilityModeL( EBTVisibilityModeNoScans, 0 );
            // Hardware power off is the last step.
            // First disconnect all plug-ins.
        iPluginMgr->DisconnectAllPlugins();
        }
        // We only signal that BT is on after everything has completed (through 
        // the CenRep power state key), so that all services are initialized.
        // We signal that BT is off immediately though, so that our clients will 
        // not try to use BT during power down.
    iServerState->StartStateMachineL( (TBool) aState );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetVisibilityModeL( TBTVisibilityMode aMode, TInt aTime )
    {
    TRACE_FUNC_ENTRY
    
    if ( aMode != EBTVisibilityModeNoScans && iEnterpriseEnablementMode == BluetoothFeatures::EDisabled )
        {
        TRACE_INFO( ( _L( "\tnot changing anything... Bluetooth is enterprise-IT-disabled" ) ) )
        User::Leave(KErrNotSupported);
        }

    TInt err = KErrNone;
    iTimerQueued &= ~EScanModeTimer;
    iTimer->Remove( iScanModeCallBack );
    if( aMode != EBTVisibilityModeNoScans )
        {
        CRepository* cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
        err = cenRep->Set( KBTDiscoverable, aMode );
        delete cenRep;
        }
    if( !err && aMode == EBTVisibilityModeTemporary )
        {
            // We need TInt64 here, as the max. time in microseconds for the 
            // max. value (1 hour) is larger than KMaxTInt32.
        TInt64 timeMicroSec = MAKE_TINT64( 0, aTime );
        timeMicroSec = timeMicroSec * KMinutesInMicroSecs;
        TTimeIntervalMicroSeconds interval( timeMicroSec );
            // Queue callback to set the visibility back to hidden.
        err = iTimer->QueueLong( interval, iScanModeCallBack );
        iTimerQueued |= EScanModeTimer;
        aMode = EBTVisibilityModeGeneral;
		}
    else if( !err && iRestoreVisibility )
        {
            // The user overrides, do not restore visibility mode anymore.
        iRestoreVisibility = EFalse;
        }
    if( !err )
        {
        err = RProperty::Set( KUidSystemCategory, 
                               KPropertyKeyBluetoothSetScanningStatus, aMode );
        }
    TBool hiddenMode = ( aMode == EBTVisibilityModeHidden );
    if( !err && aMode != EBTVisibilityModeNoScans )
        {
            // In hidden mode, we only accept connections from paired devices.
        err = RProperty::Set( KUidSystemCategory, 
                               KPropertyKeyBluetoothSetAcceptPairedOnlyMode, 
                               hiddenMode );
        }

    User::LeaveIfError( err );  // To communicate the result to the client.
    TRACE_FUNC_EXIT
    }
    
// ---------------------------------------------------------------------------
// The method is called when BT stack scanning mode P&S key is changed
// ---------------------------------------------------------------------------
//
void CBTEngServer::UpdateVisibilityModeL( TInt aStackScanMode )
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO( ( _L( "[BTEng]\t aStackScanMode: %d" ), aStackScanMode ) )
    TBTVisibilityMode currentMode;
    
    CRepository* cenRep = CRepository::NewLC( KCRUidBTEngPrivateSettings );
    User::LeaveIfError( cenRep->Get( KBTDiscoverable, (TInt&) currentMode ) );
    
        // In case we are in temp visibility mode, we cannot always know whether the BT stack
        // scan mode key was set by some external party or by us in SetVisibilityModeL above.
        // Therefore we cannot stop the timer in case aMode is EBTVisibilityModeGeneral and 
        // currentmode is EBTVisibilityModeTemporary
    if( !( currentMode == EBTVisibilityModeTemporary && aStackScanMode == EBTVisibilityModeGeneral ) )
        {
            // Cancel the timer and queue it again if needed.
        iTimerQueued &= ~EScanModeTimer;
        iTimer->Remove( iScanModeCallBack );
        if( currentMode != aStackScanMode )
            {
            if( aStackScanMode == EPageScanOnly || aStackScanMode == EInquiryAndPageScan )
                {
                User::LeaveIfError( cenRep->Set( KBTDiscoverable, aStackScanMode ) );
                }
            else if( aStackScanMode == EInquiryScanOnly )
                {
                    // We don't support ENoScansEnabled nor EInquiryScanOnly mode
                    // -> Consider these as same as Hidden 
                User::LeaveIfError( cenRep->Set( KBTDiscoverable, EBTVisibilityModeHidden ) );
                }
            else if( aStackScanMode == ENoScansEnabled )
                {
                //We don't change KBTDiscoverable here, because ENoScansEnabled
                //indicates BT/SYSTEM shutdown is happening
                }
            }
        }
    SetUiIndicatorsL();
    CleanupStack::PopAndDestroy( cenRep );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::DisconnectAllL()
    {
    TRACE_FUNC_ENTRY
    iPluginMgr->DisconnectAllPlugins();
    TCallBack cb( DisconnectAllCallBack, this );
    iBBConnMgr->DisconnectAllLinksL( cb );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetDutMode( TInt aDutMode )
    {
    TRACE_FUNC_ARG( ( _L( "DUT mode %d" ), aDutMode ) )

    if (aDutMode == EBTDutOff)
        {
        return;
        }

    TInt powerState = EBTPowerOff;
    CRepository* cenrep = NULL;
    
    TRAPD(err, cenrep = CRepository::NewL(KCRUidBluetoothPowerState));
    
    if (!err && cenrep)
        {
        cenrep->Get(KBTPowerState, powerState);
        delete cenrep;
        cenrep = NULL;
        }
    else
        {
        return;
        }

    if (powerState == EBTPowerOn)
        {

#ifndef __WINS__

        TInt err = iDutMode.Open();
        TRACE_FUNC_ARG( ( _L( "Open DUT mode handle err %d" ), err) )
        if(!err) 
            {
            iDutMode.ActivateDutMode();
            iDutMode.Close();
            }
#endif  //__WINS__                        
        }
        iEnableDutMode = EFalse;    
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::ScanModeTimerCompletedL()
    {
    TRACE_FUNC_ENTRY
    // The timer has completed, so remove our reference as well.
    iTimerQueued &= ~EScanModeTimer;
    iTimer->Remove( iScanModeCallBack );
    SetVisibilityModeL( EBTVisibilityModeHidden, 0 );
    TBTPowerStateValue power = EBTPowerOff;
    TInt err = GetHwPowerState( (TBTPowerStateValue&) power );
    if( !err && power )
        {
        // Show a notification to the user
        TBTGenericInfoNotiferParamsPckg pckg;
        pckg().iMessageType = EBTVisibilityTimeout;
        
        RNotifier notifier;
        TInt err = notifier.Connect();
        if( !err )
            {
            err = notifier.StartNotifier( KBTGenericInfoNotifierUid, pckg );
            notifier.Close();
            }
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
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
        (void) SetPowerState( EFalse );
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
// A session has been ended, update the session count.
// ---------------------------------------------------------------------------
//
void CBTEngServer::RemoveSession( TBool aAutoOff )
    {
	TRACE_FUNC_ENTRY
	TRACE_INFO( ( _L( "[CBTEngServer]\t aAutoOff %d"), aAutoOff ))
	TRACE_INFO( ( _L( "[CBTEngServer]\t iSessionCount %d"), iSessionCount ))
    iSessionCount--;
    if( aAutoOff )
        {
        TRAP_IGNORE( SetPowerStateL( EBTPowerOff, ETrue ) );
        }
    else
        {
        CheckIdle();
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
        TBTPowerStateValue pwr = EBTPowerOff;
        TInt err = GetHwPowerState( pwr );
        TRACE_INFO( ( _L( "[BTEng]\t No sessions; power state: %d" ), pwr ) )
        if( !err &&!pwr 
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
TInt CBTEngServer::SetPowerState( TBool aState )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    // HCIv2 power state type is inverted from BTPM-defined type...
    TBTPowerState powerState = (TBTPowerState) !aState;
    TRequestStatus status;
    
#ifndef __WINS__
    iPowerMgr.SetPower( powerState, NULL, status );
    User::WaitForRequest( status );
    err = status.Int();
#else   //__WINS__
    iPowerState = powerState;
    err = KErrNone;
#endif  //__WINS__

   if( !err && aState )
        {
        TInt dutMode;
        err = RProperty::Get( KPSUidBluetoothTestingMode, KBTDutEnabled, dutMode );
        if( !err && dutMode == EBTDutOn )
            {
                // Set the DUT mode key to OFF since DUT mode is disabled at this point
            err = RProperty::Set( KPSUidBluetoothTestingMode, KBTDutEnabled, EBTDutOff );
            }
            // Set the local name straight away, so that an error 
            // in the loading of the BT stack will be detected here.
            // Note that the HCIv2 power manager already loads the BT stack, 
            // so we do not need to do that here.
        TRAP(err,SetLocalNameL());
        }
    if( err )
        {
        // Power off if an error occurred during power on sequence.
#ifndef __WINS__
            // This cannot happen in emulator environment.
        iPowerMgr.SetPower( EBTOff, NULL, status );
        User::WaitForRequest( status );
#endif  //__WINS__
        } 
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::SetLocalNameL()
    {
    TRACE_FUNC_ENTRY
    RHostResolver hostResolver;
    TBuf<KMaxBluetoothNameLen> name;
    name.Zero();
    TBuf<KMaxBluetoothNameLen> tmpName;
    
    TInt err = RProperty::Get(KPropertyUidBluetoothCategory, 
                                KPropertyKeyBluetoothGetDeviceName, name);
    if(err == KErrNone)
        {
        err = RProperty::Get(KPropertyUidBluetoothCategory, 
                               KPropertyKeyBluetoothSetDeviceName, tmpName);
        if (tmpName.Compare(name))
            {
            // The name has not yet been updated. Use the new one.
            name.Copy(tmpName);
            }
        }
    // if name hasn't been set, check whats in the registry
    if (err || !name.Length())
        {
        GetLocalNameFromRegistryL(name);
        }

    TRACE_INFO( ( _L( "[CBTEngServer]\t localDev.DeviceName(): '%S'" ), &name))
    
    CleanupClosePushL(hostResolver);
    User::LeaveIfError(hostResolver.Open(iSocketServ, KBTAddrFamily, KBTLinkManager));
    User::LeaveIfError(hostResolver.SetHostName(name));    
    CleanupStack::PopAndDestroy(&hostResolver);

    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Checks whats written in the registry in order to set host name 
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::GetLocalNameFromRegistryL(TDes& aName)
    {
    TRACE_FUNC_ENTRY
    RBTRegServ btRegServ;
    RBTLocalDevice btReg;
    TBTLocalDevice localDev;
    
    CleanupClosePushL(btRegServ);
    CleanupClosePushL(btReg);
    
    // In case of error, read local name from registry
    aName.Zero();
    User::LeaveIfError(btRegServ.Connect());
    User::LeaveIfError(btReg.Open(btRegServ));
    
    // Read the BT local name from BT Registry.
    User::LeaveIfError(btReg.Get(localDev));       

    CleanupStack::PopAndDestroy(2,&btRegServ);
 
    // BT registry keeps the device name in UTF-8 format, convert to unicode.
    // The error can be > 0 if there are unconverted characters.
    TInt err = CnvUtfConverter::ConvertToUnicodeFromUtf8(aName, localDev.DeviceName());
    if (err != KErrNone)
        User::Leave(err);
    return KErrNone; 
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetClassOfDeviceL()
    {
    TRACE_FUNC_ENTRY
    TUint16 serviceClass = KCoDDefaultServiceClass;
        // Check from feature manager if stereo audio is enabled.
    FeatureManager::InitializeLibL();
    TBool supported = FeatureManager::FeatureSupported( KFeatureIdBtStereoAudio );
    FeatureManager::UnInitializeLib();
    if( supported )
        {
        // A2DP spec says we should set this bit as we are a SRC
        serviceClass |= EMajorServiceCapturing;
        }
        // These values may nayway be overridden by HCI
    TBTDeviceClass cod( serviceClass, KCoDDefaultMajorDeviceClass, 
                         KCoDDefaultMinorDeviceClass );
        // Ignore error, it is non-critical
    (void) RProperty::Set( KPropertyUidBluetoothControlCategory, 
                            KPropertyKeyBluetoothSetDeviceClass, cod.DeviceClass() );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::InitBTStackL()
    {
    TRACE_FUNC_ENTRY
    iBBConnMgr->Subscribe();
    TBTVisibilityMode visibility = EBTVisibilityModeHidden;
    CRepository* cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
    TInt err = cenRep->Get( KBTDiscoverable, (TInt&) visibility );
    delete cenRep;
    if (iRestoreVisibility == EFalse)
        {
        if( err || visibility == EBTVisibilityModeTemporary && !( iTimerQueued & EScanModeTimer ) )
            {
            visibility = EBTVisibilityModeHidden;
            }
        SetVisibilityModeL( visibility, 0 );
        }
    SetClassOfDeviceL();
    TBool sspDebugMode = EFalse;
    (void) RProperty::Get( KPropertyUidBluetoothCategory, 
                            KPropertyKeyBluetoothGetSimplePairingDebugMode, 
                            (TInt&) sspDebugMode );
        // Only set debug mode to off if it is on, to prevent a loop notifications.
    if( sspDebugMode )
        {
        sspDebugMode = EFalse;
        // Add LeaveIfError if unsuccessful
        (void) RProperty::Set(KPropertyUidBluetoothCategory, 
                               KPropertyKeyBluetoothSetSimplePairingDebugMode,
                               (TInt) sspDebugMode );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::StopBTStackL()
    {
    TRACE_FUNC_ENTRY
    TBTVisibilityMode visibility = EBTVisibilityModeHidden;
    CRepository* cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
          // Ignore error here; if we can't read it, likely we can't set it either.
    (void) cenRep->Get( KBTDiscoverable, (TInt&) visibility );
    delete cenRep;
    if( visibility == EBTVisibilityModeTemporary )
        {
        visibility = EBTVisibilityModeHidden;
        SetVisibilityModeL( visibility, 0 );    // Also cancels scan mode timer.
        }

        // Stop listening to events
    iBBConnMgr->Unsubscribe();
        // Disconnect all links
    TCallBack cb( DisconnectAllCallBack, this );
    iBBConnMgr->DisconnectAllLinksL( cb );
        // Results in a callback (which is called directly when there are no links).
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Update the power state CenRep key.
// ---------------------------------------------------------------------------
//
void CBTEngServer::UpdateCenRepPowerKeyL( TBTPowerStateValue aState )
    {
    TRACE_FUNC_ENTRY
    CRepository* cenrep = CRepository::NewLC( KCRUidBluetoothPowerState );
    User::LeaveIfError( cenrep->Set( KBTPowerState, (TInt) aState ) );
    CleanupStack::PopAndDestroy( cenrep );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Loads the BT Power Manager; leaves if it cannot be loaded.
// ---------------------------------------------------------------------------
//
void CBTEngServer::LoadBTPowerManagerL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO( ( _L( "[CBTEngServer]\t Using HCI API v2 power manager" ) ) )
    User::LeaveIfError( iPowerMgr.Open() );
#ifndef __WINS__
    TRequestStatus status( KRequestPending );
    iPowerMgr.SetPower( EBTOff, NULL, status );
    User::WaitForRequest( status );
    status = ( status.Int() == KErrAlreadyExists ? KErrNone : status.Int() ); 
    User::LeaveIfError( status.Int() );
#else   //__WINS__
    iPowerState = EBTOff;
#endif  //__WINS__
    TRACE_FUNC_EXIT
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
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetUiIndicatorsL()
    {
    TRACE_FUNC_ENTRY
    TInt powerState = EBTPowerOff;
    TBTVisibilityMode visibilityMode = EBTVisibilityModeHidden;
    CRepository* cenrep = NULL;
    TInt phys = 0;
    TInt connecting = 0;
    
    cenrep = CRepository::NewLC( KCRUidBluetoothPowerState );
    User::LeaveIfError( cenrep->Get( KBTPowerState, powerState ) );
    CleanupStack::PopAndDestroy( cenrep );  
    
    if( powerState == EBTPowerOff )
        {
        SetIndicatorStateL( EAknIndicatorBluetoothModuleOn, EAknIndicatorStateOff );
        SetIndicatorStateL( EAknIndicatorBluetooth, EAknIndicatorStateOff );
        SetIndicatorStateL( EAknIndicatorBluetoothModuleOnVisible, EAknIndicatorStateOff );
        SetIndicatorStateL( EAknIndicatorBluetoothVisible, EAknIndicatorStateOff );
        }
    else if( powerState == EBTPowerOn )
        {
        RProperty::Get( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothPHYCount, phys );
        RProperty::Get( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothConnecting, connecting );
        
        cenrep = CRepository::NewLC( KCRUidBTEngPrivateSettings );
        User::LeaveIfError( cenrep->Get( KBTDiscoverable, (TInt&) visibilityMode ) );
        CleanupStack::PopAndDestroy( cenrep );
        
        if( visibilityMode == EBTVisibilityModeHidden )
            {
             if ( connecting ) // BT connecting and hidden
                {
                SetIndicatorStateL( EAknIndicatorBluetoothModuleOn, EAknIndicatorStateOff );
                SetIndicatorStateL( EAknIndicatorBluetooth, EAknIndicatorStateAnimate );
                }
            else if ( phys > 0 ) // BT connection active and hidden     
                {
                SetIndicatorStateL( EAknIndicatorBluetoothModuleOn, EAknIndicatorStateOff );
                SetIndicatorStateL( EAknIndicatorBluetooth, EAknIndicatorStateOn );
                }
            else  // BT connection not active and hidden
                {
                SetIndicatorStateL( EAknIndicatorBluetoothModuleOn, EAknIndicatorStateOn );
                SetIndicatorStateL( EAknIndicatorBluetooth, EAknIndicatorStateOff );
                }
            SetIndicatorStateL( EAknIndicatorBluetoothModuleOnVisible, EAknIndicatorStateOff );
            SetIndicatorStateL( EAknIndicatorBluetoothVisible, EAknIndicatorStateOff );
            }           
        else if( visibilityMode == EBTVisibilityModeGeneral || visibilityMode == EBTVisibilityModeTemporary )
            {     
            if ( connecting ) // BT connecting and visible
                {
                SetIndicatorStateL( EAknIndicatorBluetoothModuleOnVisible, EAknIndicatorStateOff );
                SetIndicatorStateL( EAknIndicatorBluetoothVisible, EAknIndicatorStateAnimate );
                }
            else if ( phys > 0 ) // BT connection active and visible 
                {
                SetIndicatorStateL( EAknIndicatorBluetoothModuleOnVisible, EAknIndicatorStateOff );
                SetIndicatorStateL( EAknIndicatorBluetoothVisible, EAknIndicatorStateOn );
                }
            else  // BT connection not active and visible
                {
                SetIndicatorStateL( EAknIndicatorBluetoothModuleOnVisible, EAknIndicatorStateOn );
                SetIndicatorStateL( EAknIndicatorBluetoothVisible, EAknIndicatorStateOff );
                }
            SetIndicatorStateL( EAknIndicatorBluetoothModuleOn, EAknIndicatorStateOff );
            SetIndicatorStateL( EAknIndicatorBluetooth, EAknIndicatorStateOff );
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::SetIndicatorStateL( const TInt aIndicator, const TInt aState )
    {
    CAknSmallIndicator* indicator = CAknSmallIndicator::NewLC( TUid::Uid( aIndicator ) );
    indicator->SetIndicatorStateL( aState );
    CleanupStack::PopAndDestroy( indicator ); //indicator
    }

// ---------------------------------------------------------------------------
// Gets the current HW power state.
// For now this is a separate method to isolate the different variations.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::GetHwPowerState( TBTPowerStateValue& aState )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    
#ifndef __WINS__
    err = iPowerMgr.GetPower( (TBTPowerState&) aState, NULL );
#else   //__WINS__
    aState = (TBTPowerStateValue) iPowerState;
#endif  //__WINS__
    
        // HCIv2 power state type is inverted from BTPM-defined type...
    aState = (TBTPowerStateValue) !aState;
    TRACE_FUNC_ARG( ( _L( "Power state is %d, result %d" ), (TInt) aState, err ) )
    return err;
    }

// ---------------------------------------------------------------------------
// Check the power state and if BT gets turned off automatically.
// ---------------------------------------------------------------------------
//
void CBTEngServer::CheckTemporaryPowerStateL( TBTPowerStateValue& aCurrentState, 
    TBTPowerStateValue aNewState, TBool aTemporary )
    {
	TRACE_FUNC_ENTRY
    User::LeaveIfError( GetHwPowerState( aCurrentState ) );
    if( !aTemporary )
        {
            // Force the new power state, so clear all auto switch off flags.
            // If power is off, this will anyway be ignored.
        iAutoOffClients = 0;
        iAutoSwitchOff = EFalse;
        TCallBack cb;
        iBBConnMgr->SetAutoSwitchOff( EFalse, cb );
        if( iRestoreVisibility && aCurrentState == EBTPowerOn )
            {
                // Set visibility mode back to the value selected by the user.
            SetVisibilityModeL( EBTVisibilityModeGeneral, 0 );
            iRestoreVisibility = EFalse;
            }
        }
    else
        {
        if( aCurrentState == aNewState )
            {
            if( iAutoSwitchOff && aNewState == EBTPowerOn )
                {
                iAutoOffClients++;
                if( iTimerQueued & EAutoPowerOffTimer )
                   {
                   iTimer->Remove( iPowerOffCallBack );
                   iTimerQueued &= ~EAutoPowerOffTimer;
                   }
                }
            }
        else if( iAutoSwitchOff || aNewState == EBTPowerOn )
            {
            aNewState == EBTPowerOff ? iAutoOffClients-- : iAutoOffClients++;
            iAutoSwitchOff = ETrue;
            if( aNewState == EBTPowerOff && iAutoOffClients <= 0 )
                {
                TCallBack powerOffCb( AutoPowerOffCallBack, this );
                iBBConnMgr->SetAutoSwitchOff( ETrue, powerOffCb );
                iTimer->Queue( KBTEngBtAutoOffTimeout, iPowerOffCallBack );
                iTimerQueued |= EAutoPowerOffTimer;
                }
            else if( aNewState == EBTPowerOn )
                {
                CRepository* cenRep = CRepository::NewLC( KCRUidBTEngPrivateSettings );
                TBTVisibilityMode visibility = EBTVisibilityModeGeneral;
                TInt err = cenRep->Get( KBTDiscoverable, (TInt&) visibility );
                CleanupStack::PopAndDestroy( cenRep );
                if( !err && visibility == EBTVisibilityModeGeneral )
                    {
                    SetVisibilityModeL( EBTVisibilityModeHidden, 0 );
                    iRestoreVisibility = ETrue;
                    }
                if( iTimerQueued & EAutoPowerOffTimer )
                   {
                   iTimer->Remove( iPowerOffCallBack );
                   iTimerQueued &= ~EAutoPowerOffTimer;
                   }
                }
            }
        }
    if( iAutoOffClients < 0 )
        {
        iAutoOffClients = 0;
        }
	TRACE_FUNC_EXIT	
    }

// ---------------------------------------------------------------------------
// Check the power state and if BT gets turned off automatically.
// This method is invoked either when the timer has expired, or
// if there are no more connections while the timer was running.
// ---------------------------------------------------------------------------
//
void CBTEngServer::CheckAutoPowerOffL()
    {
   	TRACE_FUNC_ENTRY
	if ( iAutoOffClients > 0 )
	{
	TRACE_INFO( ( _L( "[CBTEngServer]\t iAutoOffClients %d"), iAutoOffClients ))
	return;
	}
    TInt linkCount = 0;
    TInt err = RProperty::Get( KPropertyUidBluetoothCategory, 
                                KPropertyKeyBluetoothGetPHYCount, linkCount );
    if( !err && !linkCount )
        {
        TRACE_INFO( ( _L( "[CBTEngServer]\t SetPowerStateL( EBTPowerOff, EFalse );")))
        SetPowerStateL( EBTPowerOff, EFalse );
        }
    else
        {
        if( iRestoreVisibility )
            {
                // Set visibility mode back to the value selected by the user.
            SetVisibilityModeL( EBTVisibilityModeGeneral, 0 );
            iRestoreVisibility = EFalse;
            }
            // show note if non-audio connection exists
        if ( !iPluginMgr->CheckAudioConnectionsL() )
            {
	        RNotifier notifier;
	        TInt err = notifier.Connect();
	        if( !err )
	            {
				TRequestStatus status;
				TBTGenericInfoNotiferParamsPckg pckg;
				pckg().iMessageType = EBTStayPowerOn;
				TBuf8<sizeof(TInt)> result;
	            //notifier.StartNotifier( KBTGenericInfoNotifierUid, pckg, result );
				notifier.StartNotifierAndGetResponse( status, 
                                                  KBTGenericInfoNotifierUid, 
                                                  pckg, result );   // Reply buffer not used.
				User::WaitForRequest( status );
	            notifier.Close();
		        }
            }
        iAutoOffClients = 0;
        iAutoSwitchOff = EFalse;
        TCallBack cb;
        iBBConnMgr->SetAutoSwitchOff( EFalse, cb );
        }
	TRACE_FUNC_EXIT	
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngServer::CheckSspDebugModeL( TBool aDebugMode )
    {
    TRACE_FUNC_ARG( ( _L( "SSP debug mode state %d" ), (TInt) aDebugMode ) )
    TBTPowerState pwr = EBTOff;
    TBool currentMode = EFalse;
    TInt err = RProperty::Get( KPropertyUidBluetoothCategory, 
                                KPropertyKeyBluetoothGetSimplePairingDebugMode, 
                                (TInt&) currentMode );
#ifndef __WINS__
    err = iPowerMgr.GetPower( pwr, NULL ); // Treat error as power off.
#else   //__WINS__
    pwr = iPowerState;
#endif  //__WINS__
    if( err || pwr == EBTOff )
        {
        iTimerQueued &= ~ESspDebugModeTimer;
        iTimer->Remove( iDebugModeCallBack );
            // Only set debug mode to off if it is on, to prevent a loop notifications.
        if( currentMode )
            {
            (void) RProperty::Set( KPropertyUidBluetoothCategory, 
                                    KPropertyKeyBluetoothSetSimplePairingDebugMode,
                                    (TInt) aDebugMode );
            }
            // In case of an error in getting the power state, turn BT off. 
            // If BT is already off, this call will be ignored.
        SetPowerStateL( EBTPowerOff, EFalse );
        }
    else if( aDebugMode )
        {
            // Ignore if there already is a timer queued.
        if( !( iTimerQueued & ESspDebugModeTimer ) )
            {
            iTimer->Queue( TTimeIntervalMicroSeconds32( KBTEngSspDebugModeTimeout ), 
                            iDebugModeCallBack );
            iTimerQueued &= ESspDebugModeTimer;
            err =  RProperty::Set(KPropertyUidBluetoothCategory, 
                                   KPropertyKeyBluetoothSetSimplePairingDebugMode, 
                                   (TInt) aDebugMode );
            }
        }
    else
        {
            // Power is on, and debug mode is set to off.
        TInt linkCount = 0;
        err = RProperty::Get( KPropertyUidBluetoothCategory, 
                               KPropertyKeyBluetoothGetPHYCount, linkCount );
        if( err || !linkCount )
            {
            (void) RProperty::Set(KPropertyUidBluetoothCategory, 
                                   KPropertyKeyBluetoothSetSimplePairingDebugMode,
                                   (TInt) aDebugMode );
            SetPowerStateL( EBTPowerOff, EFalse );
            }
        else
            {
                // There are still existing connections, queue the 
                // timer again for half the period.
            iTimer->Queue( TTimeIntervalMicroSeconds32( KBTEngSspDebugModeTimeout / 2 ), 
                            iDebugModeCallBack );
            iTimerQueued &= ESspDebugModeTimer;
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// returns the reference of pairing manager
// ---------------------------------------------------------------------------
//
CBTEngPairMan& CBTEngServer::PairManager()
    {
    return *iPairMan;
    }

// ---------------------------------------------------------------------------
// Delegate the information to pairing manager
// ---------------------------------------------------------------------------
//
void CBTEngServer::RemoteRegistryChangeDetected()
    {
    iPairMan->RemoteRegistryChangeDetected();
    }

// ---------------------------------------------------------------------------
// Access the reference of RSockServ
// ---------------------------------------------------------------------------
//
RSocketServ& CBTEngServer::SocketServ()
    {
    return iSocketServ;
    }

// ---------------------------------------------------------------------------
// Access the reference of RBTRegSrv
// ---------------------------------------------------------------------------
//
RBTRegServ& CBTEngServer::BTRegServ()
    {
    return iBTRegServ;
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
    TRAPD( err, ( (CBTEngServer*) aPtr )->ScanModeTimerCompletedL() );
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
    // Set our internal debug mode key to off. Ignore error, not critical here.
    (void) RProperty::Set( KPSUidBluetoothTestingMode, KBTSspDebugmode, EFalse );
    
    __ASSERT_ALWAYS(aPtr, PanicServer(EBTEngPanicArgumentIsNull) );
    
    TRAP_IGNORE( ( (CBTEngServer*) aPtr )->CheckSspDebugModeL( EFalse ) );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Static callback for auto power off.
// ---------------------------------------------------------------------------
//
TInt CBTEngServer::AutoPowerOffCallBack( TAny* aPtr )
    {
	TRACE_FUNC_ENTRY
	
	__ASSERT_ALWAYS(aPtr, PanicServer(EBTEngPanicArgumentIsNull) );
	
    CBTEngServer* server = (CBTEngServer*) aPtr;
    server->iTimerQueued &= ~EAutoPowerOffTimer;
    TRAPD( err, server->CheckAutoPowerOffL() );
    return err;
    }

BluetoothFeatures::TEnterpriseEnablementMode CBTEngServer::EnterpriseEnablementMode() const
    {
    return iEnterpriseEnablementMode;
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
