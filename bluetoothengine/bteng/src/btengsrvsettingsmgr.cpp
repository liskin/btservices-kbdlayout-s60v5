/*
* Copyright (c) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Class to manage Bluetooth hardware and stack settings.
*
*/

#include "btengsrvsettingsmgr.h"
#include <bthci.h>
#include <bt_subscribe_partner.h>
#include <btnotif.h>
#include <btengdomainpskeys.h>
#include <centralrepository.h>
#include <featmgr.h>
#include "btengserver.h"
#include "btengsrvpluginmgr.h"
#include "btengsrvbbconnectionmgr.h"
#include "btengsrvstate.h"
#include "debug.h"
#include <btindicatorconstants.h>
#include <hbindicatorsymbian.h>
#include <hbsymbianvariant.h>

/** ID of active object helper */
const TInt KBTEngSettingsActive = 30;
/** Constant for converting minutes to microseconds */
//const TInt64 KMinutesInMicroSecs = 60000000;
const TInt64 KMinutesInMicroSecs = MAKE_TINT64( 0, 60000000 );
/**  Timeout for disabling Simple Pairing debug mode. The value is 30 minutes. */
//const TInt KBTEngSspDebugModeTimeout = 1800000000;
const TInt64 KBTEngSspDebugModeTimeout = MAKE_TINT64( 0, 1800000000 );
/**  Timeout for turning BT off automatically. The value is 10.5 seconds. */
const TInt KBTEngBtAutoOffTimeout = 10500000;
 
// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSrvSettingsMgr::CBTEngSrvSettingsMgr( CBTEngServer* aServer )
:   iServer( aServer ),iIndicatorState(-1)
    {
    }


// ---------------------------------------------------------------------------
// Symbian second-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iActive = CBTEngActive::NewL( *this, KBTEngSettingsActive );
    iBTIndicator = CHbIndicatorSymbian::NewL(); 
    LoadBTPowerManagerL();
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
CBTEngSrvSettingsMgr* CBTEngSrvSettingsMgr::NewL( CBTEngServer* aServer )
    {
    CBTEngSrvSettingsMgr* self = new( ELeave ) CBTEngSrvSettingsMgr( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSrvSettingsMgr::~CBTEngSrvSettingsMgr()
    {
    delete iActive;  
    delete iBTIndicator;
    iPowerMgr.Close();
    }


// ---------------------------------------------------------------------------
// Power Bluetooth hardware on or off.
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvSettingsMgr::SetHwPowerState( TBTPowerState aState )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
#ifndef __WINS__
    if( iActive->IsActive() )
        {
        // Cancel the outstanding request.
        iPowerMgr.Cancel();
        iActive->Cancel();
        }
    iPowerMgr.SetPower( aState, NULL, iActive->RequestStatus() );
    iActive->GoActive();
#else   //__WINS__
    iPowerState = aState;
#endif  //__WINS__

   if( !err && aState == EBTOn )
        {
        TInt dutMode = EBTDutOff;
        err = RProperty::Get( KPSUidBluetoothTestingMode, KBTDutEnabled, dutMode );
        if( !err && dutMode == EBTDutOn )
            {
            // Set the DUT mode key to OFF since DUT mode is disabled at this point
            err = RProperty::Set( KPSUidBluetoothTestingMode, KBTDutEnabled, EBTDutOff );
            }
        }
    if( err )
        {
        // Power off if an error occurred during power on sequence.
#ifndef __WINS__
            // This cannot happen in emulator environment.
        iPowerMgr.Cancel();
        iActive->Cancel();
        iPowerMgr.SetPower( EBTOff, NULL, iActive->RequestStatus() );
        User::WaitForRequest( iActive->RequestStatus() );
        // Result will be communicated through the caller of this function (by leaving).
#else   //__WINS__
        iPowerState = EBTOff;
#endif  //__WINS__
        } 
    
#ifdef __WINS__
    TRequestStatus* status = &(iActive->RequestStatus());
    iActive->GoActive();
    User::RequestComplete(status,KErrNone);
#endif  //__WINS__
    
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// Turn BT on or off.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::SetPowerStateL( TBTPowerState aState, TBool aTemporary )
    {
    TRACE_FUNC_ARG( ( _L( "%d (temporary=%d" ), aState, aTemporary ) )
    if ( aState == EBTOn && iEnterpriseEnablementMode == BluetoothFeatures::EDisabled )
        {
        TRACE_INFO( ( _L( "no we're not... Bluetooth is enterprise-IT-disabled" ) ) )
        User::Leave(KErrNotSupported);
        }
    TBool idle = ( iServer->StateMachine()->CurrentOperation() == CBTEngSrvState::ESrvOpIdle );
    TBTPowerState currentState = EBTOff;
    CheckTemporaryPowerStateL( currentState, aState, aTemporary );

    if ( ( currentState == aState || ( aTemporary && aState == EBTOff ) ) && idle )
        {
        // The requested power state is already active, ignore silently.
        // We don't return an error here, as there is no error situation.
        TRACE_INFO( ( _L( "SetPowerStateL: nothing to do" ) ) )
        if ( currentState == aState )
            {
            // Make sure that the CenRep key is in sync.
            // During boot-up, the power is set from the CenRep key, so we could 
            // end up out-of-sync.
            TRACE_INFO( ( _L( "SetPowerStateL: currentState == aState" ) ) )
            HandleHwPowerChangeL( aState );
            }
        return;
        }
    if ( aState == EBTOn )
        {
        // Hardware power on is the first step.
        User::LeaveIfError( SetHwPowerState( aState ) );
        }
    else
        {
        //Prevent BT visibility in the situation when we turn OFF BT Engine 
        //but FM Radio is still alive
        SetVisibilityModeL( EBTVisibilityModeNoScans, 0 );
        // Hardware power off is the last step.
        // First disconnect all plug-ins.
        iServer->PluginManager()->DisconnectAllPlugins();
        }
    // We only signal that BT is on after everything has completed (through 
    // the CenRep power state key), so that all services are initialized.
    // We signal that BT is off immediately though, so that our clients will 
    // not try to use BT during power down.
    iServer->StateMachine()->StartStateMachineL( aState );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Turn BT on or off.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::SetPowerStateL(const RMessage2 aMessage )
    {
    TRACE_FUNC_ENTRY
    __ASSERT_ALWAYS( aMessage.Function() == EBTEngSetPowerState, 
            PanicServer( EBTEngPanicExpectSetPowerOpcode ) );
    if ( !iMessage.IsNull() )
        {
        // A power management request from a client is outstanding.
        User::Leave( KErrAlreadyExists );
        }
 
    TBTPowerStateValue arg = (TBTPowerStateValue) aMessage.Int0();
    // TBTPowerState power state type is inverted from TBTPowerStateValue,
    // which is used by the client to pass the parameter...
    TBTPowerState reqedPowerState( EBTOff );
    if ( arg == EBTPowerOn )
        {
        reqedPowerState = EBTOn;
        }
    TBool tempPowerOn = (TBool) aMessage.Int1();
    
    SetPowerStateL( reqedPowerState, tempPowerOn );
    if ( iServer->StateMachine()->CurrentOperation() == CBTEngSrvState::ESrvOpIdle )
        {
        // The request is accepted but the state machine is not running. This means the
        // requested power state is already active. Request is done.
        aMessage.Complete( KErrNone );
        }
    else
        {
        iMessage = RMessage2( aMessage );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Initialize Bluetooth stack settings.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::InitBTStackL()
    {
    TRACE_FUNC_ENTRY
    iServer->BasebandConnectionManager()->Subscribe();
    TBTVisibilityMode visibility = EBTVisibilityModeHidden;
    CRepository* cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
    TInt err = cenRep->Get( KBTDiscoverable, (TInt&) visibility );
    delete cenRep;
    if (iRestoreVisibility == EFalse)
        {
        if( err || visibility == EBTVisibilityModeTemporary &&
            !( iServer->IsTimerQueued( CBTEngServer::EScanModeTimer ) ) )
            {
            visibility = EBTVisibilityModeHidden;
            }
        SetVisibilityModeL( visibility, 0 );
        }

    TBool sspDebugMode = EFalse;
    (void) RProperty::Get( KPropertyUidBluetoothCategory,
                KPropertyKeyBluetoothGetSimplePairingDebugMode, (TInt&) sspDebugMode );
        // Only set debug mode to off if it is on, to prevent a loop notifications.
    if( sspDebugMode )
        {
        sspDebugMode = EFalse;
        // Add LeaveIfError if unsuccessful
        (void) RProperty::Set(KPropertyUidBluetoothCategory,
                    KPropertyKeyBluetoothSetSimplePairingDebugMode, (TInt) sspDebugMode );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Reset settings and disconnect all links.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::StopBTStackL()
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
    iServer->BasebandConnectionManager()->Unsubscribe();
        // Disconnect all links
    TCallBack cb( CBTEngServer::DisconnectAllCallBack, iServer );
    iServer->BasebandConnectionManager()->DisconnectAllLinksL( cb );
        // Results in a callback (which is called directly when there are no links).
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Update the power state CenRep key, and start BTNotif server if BT is on.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::HandleHwPowerChangeL( TBTPowerState aState )
    {
    TRACE_FUNC_ENTRY
    CRepository* cenrep = CRepository::NewLC( KCRUidBluetoothPowerState );
    // TBTPowerState power state type is inverted from TBTPowerStateValue...
    TBTPowerStateValue power = ( aState == EBTOn ) ? EBTPowerOn : EBTPowerOff;
    User::LeaveIfError( cenrep->Set( KBTPowerState, (TInt) power ) );
    CleanupStack::PopAndDestroy( cenrep );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::SetVisibilityModeL( TBTVisibilityMode aMode, TInt aTime )
    {
    TRACE_FUNC_ARG( ( _L( "[aMode: %d" ), aMode ) )
    if ( aMode != EBTVisibilityModeNoScans && iEnterpriseEnablementMode == BluetoothFeatures::EDisabled )
        {
        TRACE_INFO( ( _L( "\tnot changing anything... Bluetooth is enterprise-IT-disabled" ) ) )
        User::Leave( KErrNotSupported );
        }

    TInt err = KErrNone;
    iServer->RemoveTimer( CBTEngServer::EScanModeTimer );
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
            // Queue callback to set the visibility back to hidden.
        iServer->QueueTimer( CBTEngServer::EScanModeTimer, (TInt64) timeMicroSec );
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
// Timed visible mode has expired.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::ScanModeTimerCompletedL()
    {
    TRACE_FUNC_ENTRY
    SetVisibilityModeL( EBTVisibilityModeHidden, 0 );
    TBTPowerState power = EBTOff;
    TInt err = GetHwPowerState( power );
    // See p2.1.2.1.2 of the UI spec
    // NB:  Please distinguish between TBTPowerState::EBTOff (defined in hcitypes.h)
    //      and TBTPowerStateValue::EBTPowerOff (defined in btserversdkcrkeys.h)
    //      because they have the opposite values
    if( !err && (power != EBTOff))
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
// Check the secure simple pairing debug mode and turn it off if needed.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::CheckSspDebugModeL( TBool aDebugMode )
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
        iServer->RemoveTimer( CBTEngServer::ESspDebugModeTimer );
            // Only set debug mode to off if it is on, to prevent a loop notifications.
        if( currentMode )
            {
            (void) RProperty::Set( KPropertyUidBluetoothCategory, 
                                    KPropertyKeyBluetoothSetSimplePairingDebugMode,
                                    (TInt) aDebugMode );
            }
            // In case of an error in getting the power state, turn BT off. 
            // If BT is already off, this call will be ignored.
        SetPowerStateL( EBTOff, EFalse );
        }
    else if( aDebugMode )
        {
            // Ignore if there already is a timer queued.
        if( !( iServer->IsTimerQueued( CBTEngServer::ESspDebugModeTimer ) ) )
            {
            iServer->QueueTimer( CBTEngServer::ESspDebugModeTimer, (TInt64) KBTEngSspDebugModeTimeout );
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
            SetPowerStateL( EBTOff, EFalse );
            }
        else
            {
            // There are still existing connections, queue the
            // timer again for half the period.
            TInt64 interval = KBTEngSspDebugModeTimeout / 2;
            iServer->QueueTimer( CBTEngServer::ESspDebugModeTimer, interval );
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// A session will be ended, completes the pending request for this session.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::SessionClosed( CSession2* aSession )
    {
    TRACE_FUNC_ARG( ( _L( " session %x"), aSession ) )
    if ( !iMessage.IsNull() && iMessage.Session() == aSession )
        {
        iMessage.Complete( KErrCancel );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Check the power state and if BT gets turned off automatically.
// This method is invoked either when the timer has expired, or
// if there are no more connections while the timer was running.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::CheckAutoPowerOffL()
    {
    TRACE_FUNC_ENTRY
    if ( iAutoOffClients > 0 )
        {
        TRACE_INFO( ( _L( "[CBTEngServer]\t iAutoOffClients %d"), iAutoOffClients ) )
        return;
        }
    TInt linkCount = 0;
    TInt err = RProperty::Get( KPropertyUidBluetoothCategory, 
                                KPropertyKeyBluetoothGetPHYCount, linkCount );
    if( !err && !linkCount )
        {
        TRACE_INFO( ( _L( "[CBTEngServer]\t SetPowerStateL( EBTOff, EFalse );")))
        SetPowerStateL( EBTOff, EFalse );
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
        if ( !iServer->PluginManager()->CheckAudioConnectionsL() )
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
        iServer->BasebandConnectionManager()->SetAutoSwitchOff( EFalse, cb );
        }
    TRACE_FUNC_EXIT 
    }


// ---------------------------------------------------------------------------
// The method is called when BT stack scanning mode P&S key is changed
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::UpdateVisibilityModeL( TInt aStackScanMode )
    {
    TRACE_FUNC_ARG( ( _L( "[aStackScanMode: %d" ), aStackScanMode ) )
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
        iServer->RemoveTimer( CBTEngServer::EScanModeTimer );
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
// Set Device Under Test mode.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::SetDutMode( TInt aDutMode )
    {
    TRACE_FUNC_ARG( ( _L( "DUT mode %d" ), aDutMode ) )

    if (aDutMode == EBTDutOff)
        {
        return;
        }

    TInt powerState = EBTOff;
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
        RBluetoothDutMode dutMode;
        TInt err = dutMode.Open();
        TRACE_FUNC_ARG( ( _L( "Open DUT mode handle err %d" ), err) )
        if(!err) 
            {
            dutMode.ActivateDutMode();
            dutMode.Close();
            }
#endif  //__WINS__                        
        }
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Callback to notify that an outstanding request has completed.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::RequestCompletedL( CBTEngActive* aActive, TInt aStatus )
    {
    __ASSERT_ALWAYS( aActive->RequestId() == KBTEngSettingsActive, PanicServer( EBTEngPanicCorrupt ) );
    TRACE_FUNC_ENTRY
    (void) aActive;
    if ( aStatus != KErrNone && aStatus != KErrAlreadyExists && aStatus != KErrCancel )
        {
        // Something went wrong, so we turn BT off again.
        SetPowerStateL( EBTOff, EFalse );
        }
    if ( !iMessage.IsNull())
        {
        iMessage.Complete( aStatus );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles cancelation of an outstanding request
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::CancelRequest( TInt aRequestId )
    {
    TRACE_FUNC_ARG( ( _L( "reqID %d" ), aRequestId ) );
    if ( aRequestId == KBTEngSettingsActive )
        {
        iPowerMgr.Cancel();
        }
    TRACE_FUNC_EXIT 
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Callback to notify that an error has occurred in RunL.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::HandleError( CBTEngActive* aActive, TInt aError )
    {
    (void) aActive;
    if ( !iMessage.IsNull())
        {
        iMessage.Complete( aError );
        }
    }


// ---------------------------------------------------------------------------
// Loads the BT Power Manager; leaves if it cannot be loaded.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::LoadBTPowerManagerL()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO( ( _L( "[CBTEngSrvSettingsMgr]\t Using HCI API v2 power manager" ) ) )
    User::LeaveIfError( iPowerMgr.Open() );
#ifndef __WINS__
    TRequestStatus reqStatus;
    iPowerMgr.SetPower( EBTOff, NULL, reqStatus );
    User::WaitForRequest( reqStatus );
    TInt status = ( reqStatus.Int() == KErrAlreadyExists ? KErrNone : reqStatus.Int() ); 
    User::LeaveIfError( status );
#else   //__WINS__
    iPowerState = EBTOff;
#endif  //__WINS__
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::SetUiIndicatorsL()
    {
    TRACE_FUNC_ENTRY
    TBTPowerStateValue powerState = EBTPowerOff;
    TBTVisibilityMode visibilityMode = EBTVisibilityModeHidden;
    CRepository* cenrep = NULL;
    TInt phys = 0;
 //   TInt connecting = 0;

    cenrep = CRepository::NewLC( KCRUidBluetoothPowerState );
    User::LeaveIfError( cenrep->Get( KBTPowerState, (TInt&) powerState ) );
    CleanupStack::PopAndDestroy( cenrep );
    
    if( powerState == EBTPowerOff )
        {
        SetIndicatorStateL(EBTIndicatorOff);
        }
    else
        {
        // Power is on.
        RProperty::Get( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothPHYCount, phys );
 //       RProperty::Get( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothConnecting, connecting );
        
        cenrep = CRepository::NewLC( KCRUidBTEngPrivateSettings );
        User::LeaveIfError( cenrep->Get( KBTDiscoverable, (TInt&) visibilityMode ) );
        CleanupStack::PopAndDestroy( cenrep );
        
        if( visibilityMode == EBTVisibilityModeHidden )
            {
            if ( phys > 0 ) // BT connection active and hidden     
                {
                SetIndicatorStateL(EBTIndicatorHiddenConnected);
                }
            else  // BT connection not active and hidden
                {
                SetIndicatorStateL(EBTIndicatorOnHidden);
                }
            }           
        else if( visibilityMode == EBTVisibilityModeGeneral || visibilityMode == EBTVisibilityModeTemporary )
            {     
            if ( phys > 0 ) // BT connection active and visible 
                {
                SetIndicatorStateL(EBTIndicatorVisibleConnected);
                }
            else  // BT connection not active and visible
                {
                SetIndicatorStateL(EBTIndicatorOnVisible);
                }
            }
        }
    TRACE_FUNC_EXIT
    }

void CBTEngSrvSettingsMgr::SetIndicatorStateL( const TInt aState )
    {
    TBool success = EFalse;
    if(iIndicatorState != aState)
        {
        CHbSymbianVariant* parameters = CHbSymbianVariant::NewL(&aState,CHbSymbianVariant::EInt );
        success = iBTIndicator->Activate(KIndicatorType(),parameters); 
        delete parameters;
        if(!success)
            {
            User::Leave(iBTIndicator->Error());
            }
        iIndicatorState = aState;
        }
    }


// ---------------------------------------------------------------------------
// Gets the current HW power state.
// For now this is a separate method to isolate the different variations.
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvSettingsMgr::GetHwPowerState( TBTPowerState& aState )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    
#ifndef __WINS__
    err = iPowerMgr.GetPower( aState, NULL );
#else   //__WINS__
    aState = iPowerState;
#endif  //__WINS__
    TRACE_FUNC_ARG( ( _L( "Power state is %d, result %d" ), (TInt) aState, err ) )
    return err;
    }

// ---------------------------------------------------------------------------
// Check the power state and if BT gets turned off automatically.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSettingsMgr::CheckTemporaryPowerStateL( TBTPowerState& aCurrentState, 
    TBTPowerState aNewState, TBool aTemporary )
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
        iServer->BasebandConnectionManager()->SetAutoSwitchOff( EFalse, cb );
        if( iRestoreVisibility && aCurrentState == EBTOn )
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
            if( iAutoSwitchOff && aNewState == EBTOn )
                {
                iAutoOffClients++;
                iServer->RemoveTimer( CBTEngServer::EAutoPowerOffTimer );
                }
            }
        else if( iAutoSwitchOff || aNewState == EBTOn )
            {
            aNewState == EBTOff ? iAutoOffClients-- : iAutoOffClients++;
            iAutoSwitchOff = ETrue;
            if( aNewState == EBTOff && iAutoOffClients <= 0 )
                {
                TCallBack powerOffCb( CBTEngServer::AutoPowerOffCallBack, iServer );
                iServer->BasebandConnectionManager()->SetAutoSwitchOff( ETrue, powerOffCb );
                TInt64 interval = KBTEngBtAutoOffTimeout;
                iServer->QueueTimer( CBTEngServer::EAutoPowerOffTimer, interval );
                }
            else if( aNewState == EBTOn )
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
                iServer->RemoveTimer( CBTEngServer::EAutoPowerOffTimer );
                }
            }
        }
    if( iAutoOffClients < 0 )
        {
        iAutoOffClients = 0;
        }
    TRACE_FUNC_EXIT 
    }

