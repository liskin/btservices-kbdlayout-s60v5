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
* Description: Pairing handler for local device initiated pairing
*
*/

#include "btnotifoutgoingpairinghandler.h"
#include <btengconstants.h>
#include <btservices/btdevextension.h>
#include "btnotifsecuritymanager.h"
#include "bluetoothtrace.h"
#include "bluetoothnotification.h"
#include "btnotifserver.h"
#include "btnotifconnectiontracker.h"
#include "btnotificationmanager.h"

/**  Length of the default PIN. */
const TInt KDefaultHeadsetPinLength = 4;

/** Maximum repeated outgoing pairing attempt.
 *  if the pairing fails the UI specs says
 *  we can ask twice the user if he/she want 
 *  to retry pairing. 
 */
const TInt KMaxRepeatedPairingAttempt = 2;

enum TPairingStageId
    {
    /**
     * no pairing operation ongoing
     */
    ENoBonding = 0,
    
    /**
     * pair with dedicated bonding method
     */
    EDedicatedBonding = 200,
    
    /**
     * pair with general bonding by establishing L2CAP connection.
     */
    EGeneralBonding,  
    
    /**
     * delaying next pairing request for a while
     */
    EGeneralBondingRetryTimer,
    
    /**
     * The last pairing retry
     */
    EGeneralBondingRetry,
    
    /**
     * disconnecting physical link after pairing operation.
     * 
     * todo: not used yet.
     */
    EDisconnectLinkAfterBonding,
    };

/**  SDP PSM (used for pairing) */
const TInt KSDPPSM = 0x0001;

// Delay time to void Repeated Attempts on pairing
const TInt KGeneralBondingRetryDelayMicroSeconds = 5000000; // 5.0s

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifOutgoingPairingHandler::CBTNotifOutgoingPairingHandler( CBTNotifSecurityManager& aParent, const TBTDevAddr& aAddr)
    :  CBTNotifBasePairingHandler( aParent, aAddr )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::ConstructL()
    {
    BaseConstructL();
    User::LeaveIfError( iTimer.CreateLocal() );
    iPairingAttempt = KMaxRepeatedPairingAttempt;
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTNotifBasePairingHandler* CBTNotifOutgoingPairingHandler::NewL( CBTNotifSecurityManager& aParent, 
        const TBTDevAddr& aAddr )
    {
    CBTNotifOutgoingPairingHandler* self = new( ELeave ) CBTNotifOutgoingPairingHandler( aParent, aAddr );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifOutgoingPairingHandler::~CBTNotifOutgoingPairingHandler()
    {
    if ( iActive ) 
        {
        iActive->Cancel();
        }
    iBondingSession.Close();
    iSocket.Close();
    iTimer.Close();
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Simply deny the request as this is handing outgoing pairing
// ---------------------------------------------------------------------------
//
TInt CBTNotifOutgoingPairingHandler::ObserveIncomingPair( const TBTDevAddr& aAddr )
    {
    (void)aAddr;
    return KErrServerBusy;
    }

// ---------------------------------------------------------------------------
// Accept the request only this device is not busy with another pairing request.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::HandleOutgoingPairL( const TBTDevAddr& aAddr, TUint aCod )
    {
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST," cod 0x%08x", aCod );
    if ( iActive->IsActive() || aAddr != iAddr )
        {
        // we don't allow another pairing request.
        User::Leave( KErrServerBusy );
        }
    iAddr = aAddr;
    iCod = TBTDeviceClass( aCod );
    UnSetPairResult();
    iParent.UnpairDevice( iAddr );
    if ( CBtDevExtension::IsHeadset( iCod ) )
        {
        // If the devie is a headset, set to 0000 pin auto pairing
        iPairMode = EBTOutgoingHeadsetAutoPairing;
        }
    else
        {
        iPairMode = EBTOutgoingNoneHeadsetPairing;
        }
    DoPairingL();
    }

// ---------------------------------------------------------------------------
// Cancels an outstanding pair request by self-destruct
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::CancelOutgoingPair()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iParent.RenewPairingHandler( NULL );
    if( iNotification )
        {
        // Cancel the user query
        // This will also unregister us from the notification.
        TInt err = iNotification->Close();
        NOTIF_NOTHANDLED( !err )
        iNotification = NULL;
        }
    }


// ---------------------------------------------------------------------------
// when phone initiated a pairing request towards a headset,
// Pin code 0000 is first tried.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::GetPinCode( 
        TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    aPin().iLength = 0;
    if ( aMinPinLength <= KDefaultHeadsetPinLength 
            && aAddr == iAddr
            && iPairMode == EBTOutgoingHeadsetAutoPairing)
        {
        // if the pairing requires a stronger security level (indicated
        // by aMinPinLength), 
        // 0000 will not be supplied as it does not mmet the security
        // requirements
        const TUint8 KZeroPinValue = '0';
        for (TInt i = 0; i < KDefaultHeadsetPinLength; ++i)
            {
                aPin().iPIN[i] = KZeroPinValue;
            }
        aPin().iLength = KDefaultHeadsetPinLength;
        }
    }

// ---------------------------------------------------------------------------
// Abort pairing handling, request the owner to destroy this.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::StopPairHandling( const TBTDevAddr& aAddr )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    if ( aAddr == iAddr )
        {
        iParent.OutgoingPairCompleted( KErrCancel );
        iParent.RenewPairingHandler( NULL );
        if( iNotification )
            {
            // Cancel the user query
            // This will also unregister us from the notification.
            TInt err = iNotification->Close();
            NOTIF_NOTHANDLED( !err )
            iNotification = NULL;
            }
        }
    }

// ---------------------------------------------------------------------------
// Pairing result will be received when pairing operation completes.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::DoHandlePairServerResult( TInt aResult )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
	if (aResult == (KHCIErrorBase-EPairingNotAllowed))
		{
		// if EPairingNotAllowed is recieved then any further pairing attempts will fail
		// so don't attampt to pair
        iPairMode = EBTOutgoingPairNone;
		}
    }

// ---------------------------------------------------------------------------
// Cancels possible outstanding pairing and notify user pair success.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::DoHandleRegistryNewPairedEvent( 
        const TBTNamelessDevice& aDev )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TInt err( KErrNone );
    // If pairing was performed using Just Works mode, we set a 
    // UICookie to indicate that the device is successfully 
    // bonded so that this device will be listed in paired device view of
    // bluetooth application:
    if ( aDev.LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable )
        {
        BOstrace0(TRACE_DEBUG,DUMMY_DEVLIST,"[BTNOTIF] Outgoing Pairing, Just Works pairing");
        err = iParent.AddUiCookieJustWorksPaired( aDev );
        }
    iActive->Cancel();
    SetPairResult( err ? err : KErrNone );
    if(err == KErrNone){
    TRAP_IGNORE(ShowPairingResultNoteL(err));
    }
    iParent.OutgoingPairCompleted( err );
    iParent.RenewPairingHandler( NULL );
    }

// ---------------------------------------------------------------------------
// From class MBTNotifPairingAOObserver.
// Based on the result code, decides the next operation, either try pairing 
// with another mode, or complete pair request.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::RequestCompletedL( 
        CBtSimpleActive* aActive, TInt aStatus )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    BOstraceExt3(TRACE_DEBUG,DUMMY_DEVLIST,"reqid %d, status: %d, pair mode %d ", aActive->RequestId(), aStatus, iPairMode);
    if( aActive->RequestId() == EDedicatedBonding && 
				( aStatus == KErrRemoteDeviceIndicatedNoBonding || 
					( aStatus && iPairMode != EBTOutgoingNoneHeadsetPairing && iPairMode != EBTOutgoingPairNone ) )   )
        {
        // try general pairing if the remote doesn't have dedicated bonding, or
        // pairing fails with a headset.
        DoPairingL();
        }
    else if ( aStatus && iPairMode == EBTOutgoingHeadsetAutoPairing )
        {
        iPairMode = EBTOutgoingHeadsetManualPairing;
        // auto pairing with headset failed, try to pair again with manual pin:
        BOstrace0(TRACE_DEBUG,DUMMY_DEVLIST," auto pairing failed, switch to manual pairing");     
        DoPairingL();
        }
    else if ( aStatus && aActive->RequestId() == EGeneralBonding && 
              iPairMode == EBTOutgoingHeadsetManualPairing )
        {
        // pairing headset with manual pin failed, wait for a while and try again:
        iActive->SetRequestId( EGeneralBondingRetryTimer );
        iTimer.After( iActive->iStatus, KGeneralBondingRetryDelayMicroSeconds );
        iActive->GoActive();
        }
    else if( aActive->RequestId() == EGeneralBondingRetryTimer )
        {
        // try to pair headset again with manual pin again:
        DoPairingL();
        }
    else if ( aStatus )
        {
        // we only starts showing note if pairing failed.
        // For a successful pair, we must wait until registry has been updated.
        if ( !IsPairResultSet() )
            {
            SetPairResult( aStatus );
            }
        if ( aStatus )
            {
            // retry pairing
            if(aStatus && iPairingAttempt > 0)
                {
                if(aActive->RequestId() == EGeneralBondingRetry && iPairMode == EBTOutgoingHeadsetManualPairing)
                    {
                    // Headset pairing failed, reset and try again from auto pairing
                    iActive->SetRequestId(EGeneralBonding);
                    }
                iPairingAttempt --;
                ShowPairingRetryDialog();
                }
            else
                {
                iPairingAttempt --;
                ShowPairingFailureDialog();
                }
             }
        }
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// cancels an outstanding request according to the given id.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::CancelRequest( TInt aRequestId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    switch ( aRequestId )
        {
        case EDedicatedBonding:
            {
            iBondingSession.Close();
            break;
            }
        case EGeneralBonding:
        case EGeneralBondingRetry:
            {
            iSocket.CancelConnect();
            iSocket.Close();
            break;
            }
        case EGeneralBondingRetryTimer:
            {
            iTimer.Cancel();
            break;
            }     
        }
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles a leave in RequestCompleted by self-destructing.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::HandleError( 
        CBtSimpleActive* aActive, TInt aError )
    {
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"error: %d", aError );
    // Our RunL can actually not leave, so we should never reach here.
    (void) aActive;
    iParent.OutgoingPairCompleted( aError );
    iParent.RenewPairingHandler( NULL );
    if( iNotification )
        {
        // Cancel the user query
        // This will also unregister us from the notification.
        TInt err = iNotification->Close();
        NOTIF_NOTHANDLED( !err )
        iNotification = NULL;
        }
    }

// ---------------------------------------------------------------------------
// decide the next state and issue pair request
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::DoPairingL()
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    TPairingStageId currentMode = ( TPairingStageId ) iActive->RequestId();
    ASSERT( !iActive->IsActive() );
    TPairingStageId nextMode( EGeneralBonding );
    
    // if running BTv2.0 stack, dedicated bonding method 
    // is not available.
    if ( currentMode == ENoBonding && iParent.PairingServer() != NULL )
        {
        nextMode = EDedicatedBonding;
        }
    else if(currentMode == EGeneralBondingRetryTimer)
        {
        nextMode = EGeneralBondingRetry;
        }
    
    BOstraceExt2(TRACE_DEBUG,DUMMY_DEVLIST,"[BTENG] CBTEngOtgPair::DoPairingL: bonding mode: pre %d, next %d", currentMode, nextMode);
    
    iActive->SetRequestId( nextMode );
    if ( nextMode == EDedicatedBonding )
        {
        iBondingSession.Start( *iParent.PairingServer(), iAddr, iActive->RequestStatus() );          
        }
    else
        {
        TBTServiceSecurity sec;
        sec.SetAuthentication( ETrue );
        iSockAddr.SetBTAddr( iAddr );
        iSockAddr.SetPort(KSDPPSM);
        iSockAddr.SetSecurity( sec );    
        iSocket.Close();
        User::LeaveIfError( iSocket.Open( iParent.SocketServ(), KL2CAPDesC ) );
        iSocket.Connect( iSockAddr, iActive->RequestStatus() );
        }
    iActive->GoActive();
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// Handle a result from a user query.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    (void) aData;
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// From class MBTNotificationResult.
// The notification is finished.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::MBRNotificationClosed( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    // First unregister from the notification, so we can already get the next one.
    iNotification->RemoveObserver();
    iNotification = NULL;
    TRAP_IGNORE( NotificationClosedL( aError, aData ) );
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Get and configure a notification.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::PrepareNotificationL( TBluetoothDialogParams::TBTDialogType aType,
    TBTDialogResourceId aResourceId )
    {
    BOstraceFunctionEntry0( DUMMY_DEVLIST );
    iNotification = 
            iParent.ConnectionTracker().NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );
    const CBtDevExtension* dev = iParent.BTDevRepository().Device(iAddr);
    if(dev)
        {
        User::LeaveIfError(iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, dev->Alias()));
        }
    else
        {
        TBTDeviceName name;
        iAddr.GetReadable(name);
        User::LeaveIfError(iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, name));
        NOTIF_NOTHANDLED( !err )            
        }
    iParent.ConnectionTracker().NotificationManager()->QueueNotificationL( iNotification);
    NOTIF_NOTHANDLED( !err )
    BOstraceFunctionExit0( DUMMY_DEVLIST );
    }

// ---------------------------------------------------------------------------
// The notification is finished, handle the result.
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::NotificationClosedL( TInt aError, const TDesC8& aData )
    {
    BOstraceFunctionEntryExt( DUMMY_DEVLIST, this, aError );
    // Read the result.
    TPckgC<TBool> result( EFalse );
    result.Set( aData.Ptr(), result.Length() ); // Read the part containing the result
    // Set a pointer descriptor to capture the remaining data, if any.
    TPtrC8 dataPtr( aData.Mid( result.Length() ) );

    if(result() && iPairingAttempt >= 0)
        {
            HandleOutgoingPairL(iAddr,iCod.DeviceClass());
        }
    else
        {
            iPairingAttempt = KMaxRepeatedPairingAttempt; // reset the counter
            iParent.OutgoingPairCompleted( KErrCancel );
            iParent.RenewPairingHandler( NULL );
        }
    BOstraceFunctionExit1( DUMMY_DEVLIST, this );
    }

// ---------------------------------------------------------------------------
// Show a dialog to ask the user to retry the pairing
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::ShowPairingRetryDialog()
    {
    PrepareNotificationL( TBluetoothDialogParams::EQuery, EPairingFailureRetry);
    }

// ---------------------------------------------------------------------------
// Show a dialog to tell the user pairing retry attempt failed
// ---------------------------------------------------------------------------
//
void CBTNotifOutgoingPairingHandler::ShowPairingFailureDialog()
    {
    PrepareNotificationL( TBluetoothDialogParams::EQuery, EPairingFailureOk );
    }



