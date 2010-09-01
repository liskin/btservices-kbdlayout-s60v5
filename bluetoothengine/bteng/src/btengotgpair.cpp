/*
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class for performing pairing (i.e. bonding) 
*                with another device.
*
*/

#include "btengotgpair.h"
#include <btengconstants.h>
#include "btengpairman.h"
#include "btotgpairpub.inl"
#include "debug.h"

enum TPairingStageId
    {
    /**
     * no pairing operation ongoing
     */
    ENoBonding = 0,
    
    /**
     * pair with dedicated bonding method
     */
    EDedicatedBonding = EDevicePairUserNotification + 1,
    
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
    EGeneralBondingRetry
    };

/**  SDP PSM (used for pairing) */
const TInt KSDPPSM = 0x0001;

// Delay time to void Repeated Attempts on pairing
const TInt KBTEngGeneralBondingRetryDelayMicroSeconds = 5000000; // 5.0s

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngOtgPair::CBTEngOtgPair( CBTEngPairMan& aParent, const TBTDevAddr& aAddr)
    :  CBTEngPairBase( aParent, aAddr )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::ConstructL()
    {
    TRACE_FUNC_ENTRY
    BaseConstructL();
    User::LeaveIfError( iOutgoProperty.Attach( KPSUidBluetoothEnginePrivateCategory, 
            KBTOutgoingPairing ) );
    User::LeaveIfError( iTimer.CreateLocal() );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngPairBase* CBTEngOtgPair::NewL( CBTEngPairMan& aParent, 
        const TBTDevAddr& aAddr )
    {
    CBTEngOtgPair* self = new( ELeave ) CBTEngOtgPair( aParent, aAddr );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngOtgPair::~CBTEngOtgPair()
    {
    TRACE_FUNC_ENTRY
    SetOutgoPairProperty( iOutgoProperty, TBTDevAddr(), EBTOutgoingPairNone );
    CancelNotifier();
    DoCancelOutgoingPair();
    iBondingSession.Close();
    iSocket.Close();
    iTimer.Close();
    iOutgoProperty.Close();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Simply deny the request as this is handing outgoing pairing
// ---------------------------------------------------------------------------
//
TInt CBTEngOtgPair::ObserveIncomingPair( const TBTDevAddr& /*aAddr*/ )
    {
    return KErrServerBusy;
    }

// ---------------------------------------------------------------------------
// Accept the request only this device is not busy with another pairing request.
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::HandleOutgoingPairL( const TBTDevAddr& aAddr, TUint aCod )
    {
    TRACE_FUNC_ARG( ( _L(" cod 0x%08x"), aCod ) )
    if ( iActive->IsActive() || aAddr != iAddr )
        {
        // we don't allow another pairing request.
        User::Leave( KErrServerBusy );
        }
    
    iAddr = aAddr;
    iCod = aCod;
    UnSetPairResult();
    iParent.UnpairDevice( iAddr );
    TBTDeviceClass decls( iCod );
    if ( decls.MajorDeviceClass() == EMajorDeviceAV && 
            decls.MinorDeviceClass() != EMinorDeviceAVHandsfree )
        {
        // If the devie is a headset, set to 0000 pin auto pairing
        iPairMode = EBTOutgoingHeadsetAutoPairing;
        }
    else
        {
        iPairMode = EBTOutgoingNoneHeadsetPairing;
        }
    SetOutgoPairProperty( iOutgoProperty, iAddr, iPairMode );
    DoPairingL();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancels an outstanding pair request by self-destruct
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::CancelOutgoingPair()
    {
    TRACE_FUNC_ENTRY
    iParent.RenewPairer( NULL );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Abort pairing handling, request the owner to destroy this.
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::StopPairHandling( const TBTDevAddr& aAddr )
    {
    if ( aAddr == iAddr )
        {
        TRACE_FUNC_ENTRY
        iParent.OutgoingPairCompleted( KErrCancel );
        iParent.RenewPairer( NULL );
        TRACE_FUNC_EXIT
        }
    }

// ---------------------------------------------------------------------------
// Pairing result will be received when pairing operation completes.
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::DoHandlePairServerResult( TInt aResult )
    {
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
void CBTEngOtgPair::DoHandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev )
    {
    TRACE_FUNC_ENTRY
    if ( !IsNotifyingPairResult() )
        {
        TInt err( KErrNone );
        // If pairing was performed using Just Works mode, we set a 
        // UICookie to indicate that the device is successfully 
        // bonded so that this device will be listed in paired device view of
        // bluetooth application:
        if ( aDev.LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable )
            {
            TRACE_INFO( ( _L( "[BTENG] CBTEngOtgPair, Just Works pairing" ) ) );
            err = iParent.AddUiCookieJustWorksPaired( aDev );
            }
        DoCancelOutgoingPair();
        SetPairResult( err ? err : KErrNone );
        ShowPairingNoteAndAuthorizeQuery();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Based on the result code, decides the next operation, either try pairing 
// with another mode, or complete pair request.
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::RequestCompletedL( CBTEngActive* aActive, TInt aId, 
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "reqid %d, status: %d, pair mode %d " ), aId, aStatus, iPairMode ) )
    (void) aActive;
    (void) aId;
 
    if ( aId == EDevicePairUserNotification)
        {
        // user notification completes:
        iParent.OutgoingPairCompleted( aStatus ? aStatus : iPairResult );
        iParent.RenewPairer( NULL );
        return;
        }
    
    if( aId == EDedicatedBonding && 
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
        ( void ) SetOutgoPairProperty( iOutgoProperty, iAddr, iPairMode );
        TRACE_INFO( _L( " auto pairing failed, switch to manual pairing") );     
        DoPairingL();
        }
    else if ( aStatus && aId == EGeneralBonding && 
              iPairMode == EBTOutgoingHeadsetManualPairing )
        {
        // pairing headset with manual pin failed, wait for a while and try again:
        iActive->SetRequestId( EGeneralBondingRetryTimer );
        iTimer.After( iActive->iStatus, KBTEngGeneralBondingRetryDelayMicroSeconds );
        iActive->GoActive();
        }
    else if( aId == EGeneralBondingRetryTimer )
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
            ShowPairingNoteAndAuthorizeQuery();
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles a leave in RequestCompleted by self-destructing.
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::HandleError( CBTEngActive* aActive, TInt aId, 
    TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "error: %d" ), aError ) )
    // Our RunL can actually not leave, so we should never reach here.
    (void) aActive;
    (void) aId;
    iParent.OutgoingPairCompleted( aError );
    iParent.RenewPairer( NULL );
    }

// ---------------------------------------------------------------------------
// decide the next state and issue pair request
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::DoPairingL()
    {
    TRACE_FUNC_ENTRY
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
    
    TRACE_INFO( ( _L( "[BTENG] CBTEngOtgPair::DoPairingL: bonding mode: pre %d, next %d"), currentMode, nextMode ) );
    
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
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Make sure no outstanding pairing request is existing
// ---------------------------------------------------------------------------
//
void CBTEngOtgPair::DoCancelOutgoingPair()
    {
    TRACE_FUNC_ENTRY
    if( iActive->IsActive() )
        {
        iBondingSession.Close();
        if( iSocket.SubSessionHandle() )
            {
            iSocket.CancelConnect();
            iSocket.Close();
            }
        iTimer.Cancel();
        iActive->CancelRequest();
        }
    TRACE_FUNC_EXIT
    }
