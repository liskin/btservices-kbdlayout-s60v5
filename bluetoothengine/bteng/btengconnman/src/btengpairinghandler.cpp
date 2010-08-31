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
* Description:  Implementation of pairing a Bluetooth device.
*
*/
#include "btengpairinghandler.h"
#include "btengconnman.h"
#include "btengactive.h"
#include "debug.h"

const TInt KPairingRequestId = 60;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngPairingHandler::CBTEngPairingHandler( MBTEngConnObserver* aObserver, 
    CBTEngConnMan* aParent )
:   iObserver( aObserver ),
    iParent( aParent )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngPairingHandler::ConstructL()
    {
    ASSERT( iObserver );
    iActive = CBTEngActive::NewL( *this, KPairingRequestId, 
                                   CActive::EPriorityStandard );
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngPairingHandler* CBTEngPairingHandler::NewL( MBTEngConnObserver* aObserver, 
    CBTEngConnMan* aParent )
    {
    CBTEngPairingHandler* self = new( ELeave ) CBTEngPairingHandler( aObserver, 
                                                                      aParent );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngPairingHandler::~CBTEngPairingHandler()
    {
    delete iActive;
    iBtNotifier.Close();
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngPairingHandler::StartPairingL( const TBTDevAddr& aAddr, 
        TBTDeviceClass& aDeviceClass  )
    {
    TRACE_FUNC_ENTRY
    if ( !iBtNotifier.Handle() ) 
        {
        User::LeaveIfError( iBtNotifier.Connect() );
        }
    iAddr() = aAddr;
    iCod = aDeviceClass;
    iBtNotifier.PairDevice( iAddr, iCod.DeviceClass(), iActive->RequestStatus() );
    iActive->GoActive();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancel a pairing request.
// ---------------------------------------------------------------------------
//
void CBTEngPairingHandler::CancelPairing()
    {
    TRACE_FUNC_ENTRY
    if( iActive->IsActive() )
        {
        iBtNotifier.CancelPairDevice();
        iActive->Cancel();
        }
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngPairingHandler::RequestCompletedL( CBTEngActive* aActive, 
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "status: %d" ), aStatus ) )
    ASSERT( aActive->RequestId() != 0 );
    (void) aActive;
    // Pairing completes, inform client.
    iObserver->PairingComplete( iAddr(), aStatus );
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles cancelation of an outstanding request
// ---------------------------------------------------------------------------
//
void CBTEngPairingHandler::CancelRequest( TInt aRequestId )
    {
    TRACE_FUNC_ARG( ( _L( "reqID %d" ), aRequestId ) );
    if ( aRequestId == KPairingRequestId )
        {
        iBtNotifier.CancelPairDevice();
        }
    TRACE_FUNC_EXIT 
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngPairingHandler::HandleError( CBTEngActive* aActive, 
    TInt aError )
    {
    // Our RunL can actually not leave, so we should never reach here.
    (void) aActive;
    (void) aError;
    }

