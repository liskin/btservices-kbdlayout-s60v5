/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT determines pairing status 
*
*/

#include "btengincpair.h"
#include "btengpairman.h"
#include "btengotgpair.h"
#include "btengconstants.h"
#include "debug.h"

const TInt KBTEngWaitingForPairingOkDelay = 500000; // 0.5s

enum TPairingStageId
    {
    /**
     * is monitoring physical link status
     */
    EPhysicalLinkNotify = EDevicePairUserNotification + 1,
    EWaitingForPairingOk,
    };

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngIncPair::CBTEngIncPair( CBTEngPairMan& aParent, 
    const TBTDevAddr& aAddr) : CBTEngPairBase( aParent, aAddr )
    {
    }

// ---------------------------------------------------------------------------
// 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::ConstructL()
    {
    BaseConstructL();
    iActivePairingOk = CBTEngActive::NewL(*this, EWaitingForPairingOk, CActive::EPriorityStandard);
    User::LeaveIfError( iPairingOkTimer.CreateLocal() );
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngIncPair* CBTEngIncPair::NewL( CBTEngPairMan& aParent, 
    const TBTDevAddr& aAddr)
    {
    CBTEngIncPair* self = new (ELeave) CBTEngIncPair(aParent, aAddr);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngIncPair::~CBTEngIncPair()
    {
    TRACE_FUNC_ENTRY
    // Cancel all outstanding requests
    CancelPlaNotification();
    iPla.Close();
    iPairingOkTimer.Cancel();
    iPairingOkTimer.Close();
    if(iActivePairingOk)
        {
        iActivePairingOk->CancelRequest();
        delete iActivePairingOk;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Accept this message only if the specified device is the same as this is
// dealing with.
// ---------------------------------------------------------------------------
//
TInt CBTEngIncPair::ObserveIncomingPair( const TBTDevAddr& aAddr )
    {
    TInt err( KErrServerBusy );
    if ( iAddr == aAddr )
        {
        err = KErrNone;
        if ( !iActive->IsActive() && !OpenPhysicalLinkAdaptor() )
            {
            // If we are observing physical link, or showing user a note,
            // we won't interrupt it.
            UnSetPairResult();
            MonitorPhysicalLink();
            }
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Assign the responsibility of outgoing pair handling to CBTEngOtgPair
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::HandleOutgoingPairL( const TBTDevAddr& aAddr, TUint aCod )
    {
    TRACE_FUNC_ENTRY
    // Outgoing pairing always takes highest priority:
    CBTEngPairBase* pairer = CBTEngOtgPair::NewL( iParent, aAddr );
    pairer->HandleOutgoingPairL( aAddr, aCod );
    iParent.RenewPairer( pairer );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Accept this message only if the specified device is the same as this is
// dealing with.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::StopPairHandling( const TBTDevAddr& aAddr )
    {
    if ( aAddr == iAddr )
        {
        TRACE_FUNC_ENTRY
        iParent.RenewPairer( NULL );
        TRACE_FUNC_EXIT
        }
    }

// ---------------------------------------------------------------------------
// Notify user if pairing failed.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::DoHandlePairServerResult( TInt aResult )
    {
    CancelPlaNotification();
    // For a successful pairing, we need wait for registry table change.
    if( aResult != KErrNone && aResult != KHCIErrorBase )
        {
        // Pair failure situation.
        SetPairResult( aResult );
        ShowPairingNoteAndAuthorizeQuery();
        }
    }

// ---------------------------------------------------------------------------
// Kill this if the linkkey type indicates OBEX authentication.
// Otherwise notify user the pair result.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::DoHandleRegistryNewPairedEvent( const TBTNamelessDevice& aDev )
    {
    TRACE_FUNC_ENTRY
    
    // First of all cancel the iPairingOkTimer timer, if active
    if (iActivePairingOk->IsActive())
        {
        iPairingOkTimer.Cancel();
        iActivePairingOk->CancelRequest();
        UnSetPairResult();  // we might have set it before (if the link went down) so we want to reset it.   
        }
    switch ( aDev.LinkKeyType() )
        {
        case ELinkKeyUnauthenticatedNonUpgradable:
            {
            // If an application uses btengconnman API to connect a service of 
            // this device and JW pairing occurred as part of security enforcement,
            // it shall be a user aware pairing, and we shall add this device in paired
            // view. In this way, user is able to disconnect the device from our UI.
            // Otherwise the link key has been created by a device without IO requesting 
            // a service connection with phone. We won't take any action (e.g. remove 
            // link key) in this case. As the result, this device can't be seen in our UI, 
            // however other applications are still freely to use its services.
            TRACE_INFO(_L("[BTEng]: CBTEngIncPair: JW pairing with no IO device" ) )
            TBTEngConnectionStatus status = iParent.IsDeviceConnected( aDev.Address() );
            if ( status == EBTEngConnecting || status == EBTEngConnected )
                {
                // the return error is ingore as we can not have other proper 
                // exception handling option:
                (void) iParent.AddUiCookieJustWorksPaired( aDev );
                }
            iParent.RenewPairer( NULL );
            break;
            }
        case ELinkKeyUnauthenticatedUpgradable:
            {
            // The linkkey has been created  by an incoming OBEX service request
            // which resulted a pairing event received from pair server.
            TRACE_INFO(_L("[BTEng]: CBTEngIncPair: JW pairing with IO device" ) )
            iParent.RenewPairer( NULL );
            break;
            }
        default:
            {
            // Other pairing model than Just Works:
            CancelPlaNotification();
            SetPairResult( KErrNone );
            ShowPairingNoteAndAuthorizeQuery();
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::RequestCompletedL( CBTEngActive* /*aActive*/, TInt aId, TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "aId: %d, aStatus: %d"), aId, aStatus ) )
        // Check which request completed.
    switch( aId )
        {
        case EPhysicalLinkNotify:
            {
                // Check if the link has disconnected.
            HandlePhysicalLinkResultL( aStatus );
            break;
            }
        case EDevicePairUserNotification:
            {
                // the user has been informed of the result, kill this:
            TRACE_INFO(_L("[BTENG]:CBTEngIncPair authorization notifier completed") )
            iParent.RenewPairer( NULL );
            break;
            }
        case EWaitingForPairingOk:
            {
            // pairing failed, inform user:
            if (iPairResult == KErrNone)
                {
                // iPairResult must have been set as an error. if it's not it means somewhere else
                // it has been reset. But we need to have it set to an error as we are notifying 
                // the "unable to pair" message.
                SetPairResult(KErrGeneral);
                }
            ShowPairingNoteAndAuthorizeQuery();
            break;
            }    
        default:
                // Should not be possible, but no need for handling.
            TRACE_INFO( (_L("[BTEng]: CBTEngIncPair::RequestCompletedL unhandled event!!") ) )
            break;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles a leave in RequestCompleted by simply self-destructing.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::HandleError( CBTEngActive* aActive, TInt aId, TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "request id: %d, error: %d" ), aId, aError ) )
    (void) aActive;
    (void) aId;
    (void) aError;
        // Our error handling is to just stop observing. 
        // Nothing critical to be preserved herer, the user 
        // just won't get any notification of pairing result.
    iParent.RenewPairer( NULL );
    }

// ---------------------------------------------------------------------------
// Subscribe to physical link notifications. 
// physical link must exist when calling this function.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::MonitorPhysicalLink()
    {
    TRACE_FUNC_ENTRY
    iActive->SetRequestId( EPhysicalLinkNotify );
        // Subscribe to disconnect and error events.
    iPla.NotifyNextBasebandChangeEvent( iBbEvent, 
                            iActive->RequestStatus(), 
                            ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError );
    iActive->GoActive();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Opens the adaptor if physical link exists.
// ---------------------------------------------------------------------------
//
TInt CBTEngIncPair::OpenPhysicalLinkAdaptor()
    {
    TRACE_FUNC_ENTRY
    TInt err ( KErrNone );
    if( !iPla.IsOpen() )
        {
            // Try to open the adapter in case it failed earlier.
            // This can happen for outgoing dedicated bonding with 
            // non-SSP device, as the PIN dialog can be kept open even 
            // though the link has dropped because of a time-out.
        err = iPla.Open( iParent.SocketServ(), iAddr );
        }
    TRACE_INFO( (_L("[BTEng]: CBTEngIncPair::HasPhysicalLink ? %d"), iPla.IsOpen() ) )
    return err;
    }

// ---------------------------------------------------------------------------
// Cancel outstanding physical link notification
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::CancelPlaNotification()
    {
    TRACE_FUNC_ENTRY
    if( iActive && iActive->IsActive() && 
            iActive->RequestId() == EPhysicalLinkNotify )
        {
        // cancel Baseband monitor
        iPla.CancelNextBasebandChangeEventNotifier();
        iActive->Cancel();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Handle a physical link event. Notify pair failed if physical link is down.
// ---------------------------------------------------------------------------
//
void CBTEngIncPair::HandlePhysicalLinkResultL( TInt aResult )
    {
    TRACE_FUNC_ARG( ( _L( " BBEvent 0x%08X, code %d"), 
                            iBbEvent().EventType(), iBbEvent().SymbianErrorCode() ) )
        // Check if the connection is still alive.
    TBool physicalLinkDown = 
        ( iBbEvent().EventType() == ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError );

    if( aResult || physicalLinkDown )
        {
        // link went down. It might be because of pairing failed or the remote device disconnected the
        // physical link after a successful pairing.
        // we wait for 0.5 secs before notifying the "unable to pair" message as, if the pair is 
        // successful, we manage it to show the right confirmation message.
        SetPairResult( (aResult == 0) ? KErrGeneral : aResult );
        iPairingOkTimer.After(iActivePairingOk->iStatus, KBTEngWaitingForPairingOkDelay);
        iActivePairingOk->GoActive();
        }
    else
        {
        // Uninteresting event, re-subscribe.
        MonitorPhysicalLink();
        }
    TRACE_FUNC_EXIT
    }
