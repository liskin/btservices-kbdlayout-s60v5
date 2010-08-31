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
* Description:  Bluetooth Engine API for connection management functionality.
*
*/



#include <e32base.h>
#include <featmgr.h>
#include <centralrepository.h>

#include "btengconnman.h"
#include "btengconnhandler.h"
#include "btengpairinghandler.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngConnMan::CBTEngConnMan( MBTEngConnObserver* aObserver )
:   iObserver( aObserver )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngConnMan::ConstructL()
    {
    TRACE_FUNC_ENTRY
        // Check if BT is supported at all
    FeatureManager::InitializeLibL();
    TBool btSupported = FeatureManager::FeatureSupported( KFeatureIdBt );
    FeatureManager::UnInitializeLib();
    if( !btSupported )
        {
        TRACE_INFO( ( _L( "[BTENGSETTINGS]\t ConstructL: BT not supported!!" ) ) )
        User::Leave( KErrNotSupported );
        }
    iConnHandler = CBTEngConnHandler::NewL( iObserver );
    
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngConnMan* CBTEngConnMan::NewL( MBTEngConnObserver* aObserver )
    {
    CBTEngConnMan* self = CBTEngConnMan::NewLC( aObserver );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngConnMan* CBTEngConnMan::NewLC( MBTEngConnObserver* aObserver )
    {
    CBTEngConnMan* self = new( ELeave ) CBTEngConnMan( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngConnMan::~CBTEngConnMan()
    {
    TRACE_FUNC_ENTRY
    delete iConnHandler;
    delete iPairingHandler;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::Connect( const TBTDevAddr& aAddr, 
    const TBTDeviceClass& aDeviceClass )
    {
    TRACE_FUNC_ENTRY
    return iConnHandler->ConnectDevice( aAddr, aDeviceClass );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngConnMan::CancelConnect( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    (void) iConnHandler->CancelConnectDevice( aAddr );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::Disconnect( const TBTDevAddr& aAddr, 
    TBTDisconnectType aDiscType )
    {
    TRACE_FUNC_ENTRY
    return iConnHandler->DisconnectDevice( aAddr, aDiscType );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::IsConnected( const TBTDevAddr& aAddr, 
    TBTEngConnectionStatus& aConnected )
    {
    TRACE_FUNC_ENTRY
    return iConnHandler->IsDeviceConnected( aAddr, aConnected );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::IsConnectable( const TBTDeviceClass& aDeviceClass, 
    TBool& aConnectable )
    {
    TRACE_FUNC_ENTRY
    return iConnHandler->IsDeviceConnectable( aDeviceClass, aConnectable );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::SetObserver( MBTEngConnObserver* aObserver )
    {
    TRACE_FUNC_ENTRY
    return iConnHandler->NotifyConnectionEvents( aObserver );
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngConnMan::RemoveObserver()
    {
    TRACE_FUNC_ENTRY
    iConnHandler->CancelNotifyConnectionEvents();
    }


// ---------------------------------------------------------------------------
// Get the connected addresses of all baseband connections.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::GetConnectedAddresses( RBTDevAddrArray& aAddrArray )
    {
    TRACE_FUNC_ENTRY
    TRAPD( err, iConnHandler->GetConnectedAddressesL( aAddrArray, 
                                                       EBTProfileUndefined ) );
    return err;
    }


// ---------------------------------------------------------------------------
// Get the connected addresses for a specified profile (that is managed 
// through BT Engine).
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::GetConnectedAddresses( RBTDevAddrArray& aAddrArray, 
    TBTProfile aConnectedProfile )
    {
    TRACE_FUNC_ENTRY
    TRAPD( err, iConnHandler->GetConnectedAddressesL( aAddrArray, 
                                                       aConnectedProfile ) );
    return err;
    }


// ---------------------------------------------------------------------------
// Initiate pairing with the specified device
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::PairDevice( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    TBTDeviceClass deviceClass;
    return PairDevice( aAddr, deviceClass );
    }


// ---------------------------------------------------------------------------
// Initiate pairing with the specified device (which advertises
// specified service class).
// ---------------------------------------------------------------------------
//    
EXPORT_C TInt CBTEngConnMan::PairDevice( const TBTDevAddr& aAddr, TBTDeviceClass aDeviceClass )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    if( !iPairingHandler )
        {
        TRAP( err, iPairingHandler = CBTEngPairingHandler::NewL( iObserver, this ) );
        }
    if( !err )
        {
        iPairingHandler->CancelPairing();

        TRAP( err, iPairingHandler->StartPairingL( aAddr, aDeviceClass ) );
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }

// ---------------------------------------------------------------------------
// Cancels an ongoing pairing.
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngConnMan::CancelPairDevice()
    {
    TRACE_FUNC_ENTRY
    if( iPairingHandler )
        {
        iPairingHandler->CancelPairing();
        delete iPairingHandler;
        iPairingHandler = NULL;
        }
    }


// ---------------------------------------------------------------------------
// Tell BTEng to start observing the status of an ongoing pairing.
// Deprecated since Symbian^4.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::StartPairingObserver( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    (void) aAddr;
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// Tell BTEng to stop observing the status of an ongoing pairing.
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTEngConnMan::PrepareDiscovery()
    {
    TRACE_FUNC_ENTRY
    (void) iConnHandler->PrepareDiscovery();
    }

// ---------------------------------------------------------------------------
// Deprecated since Symbian^4.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::StopPairingObserver( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    (void) aAddr;
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Check if the remote device is connectable or not.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngConnMan::IsConnectable( const TBTDevAddr& aAddr, 
        const TBTDeviceClass& aDeviceClass, TBool& aConnectable )
    {
    TRACE_FUNC_ENTRY
    return iConnHandler->IsDeviceConnectable( aAddr, aDeviceClass, aConnectable );
    }

// ---------------------------------------------------------------------------
// Default implementation of BTEng ConnMan result observer.
// ---------------------------------------------------------------------------
//
EXPORT_C void MBTEngConnObserver::PairingComplete( TBTDevAddr& aAddr, TInt aErr )
    {
    (void) aAddr;
    (void) aErr;
    }
