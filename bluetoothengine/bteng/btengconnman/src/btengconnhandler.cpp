/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implements the actual connection management, and receives 
*                notifications of Bluetooth connection events.
*
*/



#include <e32base.h>

#include "btengconnhandler.h"
#include "btengconnman.h"
#include "debug.h"

/**  ID to identify the outstanding asynchronous request. */
enum TRequestId
    {
    EConnectionEventId = 50,
    EPairDeviceId
    };

/**  Max. number of addresses that can be passed to the server
     side for getting connected addresses */
const TInt KBTEngMaxAddrArraySize = 10;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngConnHandler::CBTEngConnHandler( MBTEngConnObserver* aObserver )
:   iObserver( aObserver )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngConnHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    User::LeaveIfError( iBTEng.Connect() );
    if( iObserver )
        {
        NotifyConnectionEvents( iObserver );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngConnHandler* CBTEngConnHandler::NewL( MBTEngConnObserver* aObserver )
    {
    CBTEngConnHandler* self = new( ELeave ) CBTEngConnHandler( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngConnHandler::~CBTEngConnHandler()
    {
    TRACE_FUNC_ENTRY
    CancelNotifyConnectionEvents();
    CancelPairing();
    iBTEng.Close();
    }

// ---------------------------------------------------------------------------
// Gets the connected device addresses; If a profile is defined, only 
// connections for that profile are returned.
// ---------------------------------------------------------------------------
//
void CBTEngConnHandler::GetConnectedAddressesL( RBTDevAddrArray& aAddrArray, 
    TBTProfile aConnectedProfile )
    {
    aAddrArray.Reset();
    TInt devAddrSize = sizeof( TBTDevAddr );
    HBufC8* addrBuf = HBufC8::NewLC( KBTEngMaxAddrArraySize * devAddrSize );
    TPtr8 ptr = addrBuf->Des();
    TPckgBuf<TInt> profilePkg( aConnectedProfile );
    User::LeaveIfError( iBTEng.GetConnectedAddresses( ptr, profilePkg ) );
    while( ptr.Length() >= KBTDevAddrSize )
        {
        TBTDevAddr addr( ptr.Mid( ptr.Length() - devAddrSize, KBTDevAddrSize ) );
		ptr.SetLength( Max( ptr.Length() - devAddrSize, 0 ) );
		aAddrArray.Append( addr );
		}
    CleanupStack::PopAndDestroy( addrBuf );
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngConnHandler::NotifyConnectionEvents( MBTEngConnObserver* aObserver )
    {
    TRACE_FUNC_ENTRY
    ASSERT( aObserver );
    iObserver = aObserver;
    TInt err = KErrNone;
    if( iConnEventActive && iConnEventActive->IsActive() )
        {
        CancelNotifyConnectionEvents();
        }
    if( !iConnEventActive )
        {
            // Use a higher prioritty than normal, because we want 
            // to be notified fast (e.g. to update the UI).
        TRAP( err, iConnEventActive = CBTEngActive::NewL( *this, EConnectionEventId, 
                                                  CActive::EPriorityUserInput ) );
        }
    if( !err && iConnEventActive->RequestStatus() != KErrServerTerminated )
        {
        err = iBTEng.NotifyConnectionEvents( iEventPkg, iConnEventActive->RequestStatus() );
		if( !err )
			{
			iConnEventActive->GoActive();
			}
        }
    return err;
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngConnHandler::CancelNotifyConnectionEvents()
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    if( iConnEventActive && iConnEventActive->IsActive() )
        {
        err = iBTEng.CancelNotifyConnectionEvents();
        iConnEventActive->CancelRequest();
        }
    delete iConnEventActive;
    iConnEventActive = NULL;
    return err;
    }

// -----------------------------------------------------------------------------
// Request server side to activate/deactivate a pair observer
// -----------------------------------------------------------------------------
//
TInt CBTEngConnHandler::SetPairingObserver( const TBTDevAddr& aAddr, 
    TBool aActivate )
    {
    RBTEng bteng;
    TInt err = bteng.Connect();
    if ( !err )
        {
        err = bteng.SetPairingObserver( aAddr, aActivate );
        }
    bteng.Close();
    return err;
    }

// ---------------------------------------------------------------------------
// Request BTEng to pair the device
// ---------------------------------------------------------------------------
//
TInt CBTEngConnHandler::StartPairing( const TBTDevAddr& aAddr, 
    const TBTDeviceClass& aDeviceClass )
    {
    TRACE_FUNC_ENTRY
    TInt err( KErrNone );
    if( iPairActive && iPairActive->IsActive() )
        {
        err = KErrServerBusy;
        }
    
    if( !iPairActive )
        {
            // Use a higher prioritty than normal, because we want 
            // to be notified fast (e.g. to update the UI).
        TRAP( err, iPairActive = CBTEngActive::NewL( *this, EPairDeviceId, 
                                                  CActive::EPriorityUserInput ) );
        }
    if ( !err )
        {
        iPairAddr() = aAddr;
        iPairDevCod = aDeviceClass.DeviceClass();
        iBTEng.PairDevice( iPairAddr, iPairDevCod, iPairActive->RequestStatus() );
        iPairActive->GoActive();
        }
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Cancel any outstanding operation, free resources.
// ---------------------------------------------------------------------------
//
void CBTEngConnHandler::CancelPairing()
    {
    TRACE_FUNC_ENTRY
    if( iPairActive && iPairActive->IsActive() )
        {
        iBTEng.CancelPairDevice();
        }
    delete iPairActive;
    iPairActive = NULL;
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Called by the active object when a change in connection status has occured.
// ---------------------------------------------------------------------------
//
void CBTEngConnHandler::RequestCompletedL( CBTEngActive* aActive, TInt aId, 
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "ID: %d status: %d" ), aId, aStatus ) )

    (void) aActive;
    switch ( aId )
        {
        case EConnectionEventId:
            {
            HandleConnectionEvent( aStatus );
            break;
            }
        case EPairDeviceId:
            {
            if ( iObserver )
                {
                iObserver->PairingComplete( iPairAddr(), aStatus );
                }
            }
        }

    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Called when RequestCompletedL/RunL leaves.
// ---------------------------------------------------------------------------
//
void CBTEngConnHandler::HandleError( CBTEngActive* aActive, TInt aId, TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "error: %d" ), aError ) )
        // Should any info be passed to the client??
    (void) aActive;
    (void) aId;
    (void) aError;
    }

void CBTEngConnHandler::HandleConnectionEvent( TInt aStatus )
    {
    TBTDevAddr addr;
    RBTDevAddrArray conflictsArray;
    TBTEngConnectionStatus connStatus = EBTEngNotConnected;
        // Subscribe to the next event first.
    (void) NotifyConnectionEvents( iObserver );
    addr = iEventPkg().iAddr;
    connStatus = iEventPkg().iConnEvent;
    if( iEventPkg().iConflictsBuf.Length() > 0 )
        {
            // Parse conflicts array buffer
        TInt devAddrSize = sizeof( TBTDevAddr );
        while( iEventPkg().iConflictsBuf.Length() >= KBTDevAddrSize )
            {
            TInt len = iEventPkg().iConflictsBuf.Length();
            TPtrC8 ptr = iEventPkg().iConflictsBuf.Mid( len - devAddrSize, 
                                                         KBTDevAddrSize );
            TBTDevAddr addr( ptr );
            conflictsArray.Append( addr );
            iEventPkg().iConflictsBuf.SetLength( Max( len - devAddrSize, 0 ) );
            }
        }

    switch( connStatus )
        {
            // The enumeration allows for more information than can be
            // signalled to the client. Anyway allow for all cases.
        case EBTEngNotConnected:
        case EBTEngDisconnecting:
            {
            // Add device address logging here!
            TRACE_INFO( ( _L( "[BTENG]\t device X disconnected" ) ) )
            iObserver->DisconnectComplete( addr, aStatus );
            }
            break;
        case EBTEngConnecting:
        case EBTEngConnected:
            {
                // Note! Even though the status can say connected, it can 
                // mean that the device is not connected because an error 
                // occurred during connection establishment!

            // Add device address logging here!
            TRACE_INFO( ( _L( "[BTENG]\t device X connected" ) ) )
			if ( conflictsArray.Count() )
			    {
		        iObserver->ConnectComplete( addr, aStatus, &conflictsArray );
		        }
			else
			    {
				iObserver->ConnectComplete( addr, aStatus, NULL );
				}
            }
            break;
        default:
            {
            TRACE_INFO( ( _L( "[BTENG]\t Wrong connection status (%d)!!" ), 
                          connStatus ) )
            }
            break;
        }
    conflictsArray.Close();
 
    }
