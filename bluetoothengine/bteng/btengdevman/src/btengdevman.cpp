/*
* Copyright (c) 2006, 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of the Bluetooth Engine Device Management API
*
*/



#include <featmgr.h>

#include "btengdevman.h"
#include "btengconstants.h"
#include "debug.h"
#include "btengdevmanpanic.h"

void Panic(TBtEngDevManPanicCode aPanic)
    {
    User::Panic(KBtEngDevManPanic, aPanic);
    }


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngDevMan::CBTEngDevMan( MBTEngDevManObserver* aObserver )
:   CActive( EPriorityStandard ),
    iState( EStateIdle ),
    iCreatingView( EFalse ),
    iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngDevMan::ConstructL()
    {
    TRACE_FUNC_ENTRY
        // Check if BT is supported at all
    FeatureManager::InitializeLibL();
    TBool btSupported = FeatureManager::FeatureSupported( KFeatureIdBt );
    FeatureManager::UnInitializeLib();
    if( !btSupported )
        {
        TRACE_INFO( ( _L( "[BTENGDEVMAN]\t ConstructL: BT not supported" ) ) )
        User::Leave( KErrNotSupported );
        }

    User::LeaveIfError( iRegServ.Connect() );
    User::LeaveIfError( iRegistry.Open( iRegServ ) );

    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngDevMan* CBTEngDevMan::NewL( MBTEngDevManObserver* aObserver )
    {
    CBTEngDevMan* self = CBTEngDevMan::NewLC( aObserver );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngDevMan* CBTEngDevMan::NewLC( MBTEngDevManObserver* aObserver )
    {
    CBTEngDevMan* self = new( ELeave ) CBTEngDevMan( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngDevMan::~CBTEngDevMan()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iRegistry.Close();
    iRegServ.Close();
    delete iResponse;
    delete iModifiedDevice;
    }


// ---------------------------------------------------------------------------
// Add a device into the Bluetooth device registry.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDevMan::AddDevice( const CBTDevice& aDevice )
    {
    TRACE_FUNC_ENTRY
    __ASSERT_ALWAYS( iObserver, Panic(EBtEngDevManPanicNullObserver) );
    TInt err = KErrNone;
    if( iState != EStateIdle )
        {
        err = KErrInUse;
        }
    else
        {
        TRAP( err, iRegistry.AddDeviceL( aDevice, iStatus ) );
        if( err == KErrNone )
            {
            iState = EStateAddDevice;
            SetActive();
            }
        }
    TRACE_FUNC_EXIT
    return err;
    }


// ---------------------------------------------------------------------------
// Get an array of devices which matches the search criteria.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDevMan::GetDevices( const TBTRegistrySearch& aCriteria, 
    CBTDeviceArray* aResultArray )
    {
    TRACE_FUNC_ENTRY
    __ASSERT_ALWAYS( aResultArray, Panic(EBtEngDevManPanicNullResultArray) );
    TInt err = KErrNone;
    if( iState != EStateIdle )
        {
        err = KErrInUse;
        }
    else
        {
        ASSERT( !iResultArray && !iAsyncWaiter );
        iResultArray = aResultArray;
        iRegistry.CreateView( aCriteria, iStatus );
        iState = EStateGetDevices;
        iCreatingView = ETrue;
        SetActive();
        if( !iObserver )
            {
                // Synchronous version of GetDevices, will not use callback.
            iAsyncWaiter = new CActiveSchedulerWait();
            if( !iAsyncWaiter )
                {
                Cancel();
                return KErrNoMemory;
                }
            iAsyncWaiter->Start();
            err = iStatus.Int();
            iResultArray = NULL;
            delete iAsyncWaiter;
            iAsyncWaiter = NULL;
            }
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// Delete the devices which matches the search criteria.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDevMan::DeleteDevices( const TBTRegistrySearch& aCriteria )
    {
    TRACE_FUNC_ENTRY
    __ASSERT_ALWAYS( iObserver, Panic(EBtEngDevManPanicNullObserver) );
    TInt err = KErrNone;
    if( iState != EStateIdle )
        {
        err = KErrInUse;
        }
    else
        {
        iRegistry.CreateView( aCriteria, iStatus );
        iState = EStateDeleteDevices;
        iCreatingView = ETrue;
        SetActive();
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// Modify a device in the Bluetooth device registry.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngDevMan::ModifyDevice( const CBTDevice& aDevice )
    {
    TRACE_FUNC_ENTRY
    __ASSERT_ALWAYS( iObserver, Panic(EBtEngDevManPanicNullObserver) );
    TInt err = KErrNone;
    if( iState != EStateIdle )
        {
        err = KErrInUse;
        }
    else
        {
        ASSERT( !iModifiedDevice );
        TRAP( err, iModifiedDevice = aDevice.CopyL() );
        }
    if( !err )
        {
        TBTRegistrySearch searchPattern;
        searchPattern.FindAddress( iModifiedDevice->BDAddr() );
        iRegistry.CreateView( searchPattern, iStatus );
        iState = EStateModifyDevice;
        iModifyState = ECheckPairing;
        iCreatingView = ETrue;
        SetActive();
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Handle request completion, eventually making the callback.
// ---------------------------------------------------------------------------
//
void CBTEngDevMan::RunL()
    {
    TRACE_FUNC_ARG( ( _L( "status: %d" ), iStatus.Int() ) )
    TInt count = User::LeaveIfError(iStatus.Int());
    if( iCreatingView )
        {
            // First stage completed, got a view on the registry.
        iCreatingView = EFalse;
            // When creating a view, iStatus represents 
            // the number of matching results.
        if( count )
            {
            ASSERT(iState >= EStateIdle && iState <= EStateDeleteDevices);
            if( iState == EStateDeleteDevices )
                {
                iRegistry.DeleteAllInView( iStatus );
                SetActive();
                }
            else
                {
                iResponse = CBTRegistryResponse::NewL( iRegistry );
                iResponse->Start( iStatus );
                SetActive();
                }
            }
        else
            {
            if( iState == EStateGetDevices )
                {
                iState = EStateIdle;
                if( iObserver )
                    {
                    CBTDeviceArray* clientArray = iResultArray;
                    iResultArray = NULL;
                    iObserver->HandleGetDevicesComplete( KErrNone, clientArray );
                    }
                else
                    {
                    __ASSERT_ALWAYS(iAsyncWaiter,Panic(EBtEngDevManPanicNullWaiter));
                    iAsyncWaiter->AsyncStop(); // Synchronous version
                    }
                }
            else
                {
                iState = EStateIdle;
                iObserver->HandleDevManComplete( KErrNone );
                }
            }
        }
    else
        {
            // Second stage completed.
        switch( iState )
            {
            case EStateGetDevices:
                {
                (void) iRegistry.CloseView();
                RBTDeviceArray& resultArray = iResponse->Results();
                while (resultArray.Count())
                    {
                    CBTDevice* dev = resultArray[ 0 ];
                    resultArray.Remove( 0 );
                    iResultArray->AppendL( dev );
                    }
                delete iResponse;
                iResponse = NULL;
                iState = EStateIdle;
                if( iObserver )
                    {
                    CBTDeviceArray* clientArray = iResultArray;
                    iResultArray = NULL;
                    iObserver->HandleGetDevicesComplete( count, clientArray );
                    }
                else
                    {
                    __ASSERT_ALWAYS(iAsyncWaiter,Panic(EBtEngDevManPanicNullWaiter));
                    iAsyncWaiter->AsyncStop(); // Synchronous version
                    }
                }
                break;
            case EStateModifyDevice:
                {
                DoModifyDeviceL();
                if( iModifyState != ECheckNone )
                    {
                    break;
                    }
                delete iModifiedDevice;
                iModifiedDevice = NULL;
                delete iResponse;
                iResponse = NULL;
                }   // Fall through
            case EStateDeleteDevices:
                {
                (void) iRegistry.CloseView();
                }   // Fall through
            case EStateAddDevice:
                {
                iState = EStateIdle;
                iObserver->HandleDevManComplete( count );
                }
                break;
            case EStateIdle:
            default:
                {
                ASSERT(EFalse);
                TRACE_INFO( ( _L( "[BTEngDevMan]\t RunL - bad state! %d" ), iState ) )
                }
                break;
            }
        }
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called if something leaves in RunL.
// ---------------------------------------------------------------------------
//
TInt CBTEngDevMan::RunError( TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "state: %d (modifystate: %d); error: %d" ), 
                      iState, iModifyState, aError ) )
        // Reset all state information
    iModifyState = ECheckNone;
    iCreatingView = EFalse;
        // Clean up all used resources
    (void) iRegistry.CloseView();   // An error is returned if there is no 
                                    // open view -> can be safely ignored.
    delete iResponse;
    iResponse = NULL;
        // Inform the client
    if( iState == EStateGetDevices )
        {
        iState = EStateIdle;
        if( iObserver )
            {
            CBTDeviceArray* clientArray = iResultArray;
            iResultArray = NULL;
            iObserver->HandleGetDevicesComplete( aError, clientArray );
            }
        else
            {
            __ASSERT_ALWAYS(iAsyncWaiter,Panic(EBtEngDevManPanicNullWaiter));
            iAsyncWaiter->AsyncStop(); // Synchronous version
            }
        }
    else
        {
        iState = EStateIdle;
        iObserver->HandleDevManComplete( aError );
        }
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Cancel current outstanding operation, if any.
// ---------------------------------------------------------------------------
//
void CBTEngDevMan::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iRegistry.CancelRequest( iStatus );
    (void) iRegistry.CloseView();
    iState = EStateIdle;
    iModifyState = ECheckNone;
    iCreatingView = EFalse;
    if( iResponse )
        {
        iResponse->Cancel();
        delete iResponse;
        iResponse = NULL;
        }
    delete iModifiedDevice;
    iModifiedDevice = NULL;
    if( iAsyncWaiter )
        {
        iAsyncWaiter->AsyncStop();
        }
    iResultArray = NULL;
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Process result of a registry search, when GetDevice or
// ModifyDevice has been called.
// ---------------------------------------------------------------------------
//
void CBTEngDevMan::DoModifyDeviceL()
    {
    TRACE_FUNC_ENTRY
    if( iModifyState == ECheckFinal )
        {
            // Modifications have completed.
        iModifyState = ECheckNone;
        return;
        }

        // We searched for a device address, so assume that 
        // there is only one match.
    ASSERT(iResponse->Results().Count() > 0);
    CBTDevice* regDevice = iResponse->Results()[ 0 ];
        // Modifications to CBTDevice parameteres could include multiple 
        // asynchronous operations. Therefore each possibilbe modification 
        // is checked. The order is always the same so that we can know what
        // the next step is.
    if( iModifyState == ECheckPairing )
        {
            // IsValidPaired returns if the pairing bit has been set.
            // IsPaired returns if the device is actually paired.
        if( ( regDevice->IsValidPaired() && regDevice->IsPaired() ) &&
            ( !iModifiedDevice->IsValidPaired() || !iModifiedDevice->IsPaired() ) )
            {
            TRACE_INFO( _L( "[BTEngDevMan]\t DoModifyDeviceL UnpairDevice ") );
            // check if client request to ban or unpair a device paired with Just Works
            if ( regDevice->LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable &&
                 regDevice->IsValidUiCookie() &&
                 ( regDevice->UiCookie() & EBTUiCookieJustWorksPaired ) )
                {
                // Remove the UI cookie bit for Just Works pairing. Modifying registry
                // will be called at nameless device checking.
                TInt32 cookie = regDevice->UiCookie() & ~EBTUiCookieJustWorksPaired;
                iModifiedDevice->SetUiCookie( cookie );
                TRACE_INFO( _L( "[BTEngDevMan]\t DoModifyDeviceL removed JW cookie ") );
                }
            // Unpairing is requested.
            iRegistry.UnpairDevice( iModifiedDevice->BDAddr(), iStatus );
            SetActive();
                //  The reverse operation (pairing) is not performed through DevMan.
            }
        iModifyState = ECheckDevName;
        }
    if( !IsActive() && iModifyState == ECheckDevName )
        {
            // Check that there is a valid device name, and that it is not 
            // the same as the registered name, or that the registry does not
            // contain a valid name yet.
        if( iModifiedDevice->IsValidDeviceName() && 
            ( !regDevice->IsValidDeviceName() || 
            ( iModifiedDevice->DeviceName() != regDevice->DeviceName() ) ) )
            {
            TRACE_INFO( _L( "[BTEngDevMan]\t DoModifyDeviceL ModifyBluetoothDeviceName ") );
            iRegistry.ModifyBluetoothDeviceNameL( iModifiedDevice->BDAddr(), 
                                                   iModifiedDevice->DeviceName(), 
                                                   iStatus );
            SetActive();
            }
        iModifyState = ECheckFriendlyName;
        }
    if( !IsActive() && iModifyState == ECheckFriendlyName )
        {
            // Check that there is a valid friendly name, and that it is not 
            // the same as the registered name, or that the friendly name 
            // is removed.
        if( ( iModifiedDevice->IsValidFriendlyName() && 
            ( iModifiedDevice->FriendlyName() != regDevice->FriendlyName() ) ) || 
            ( !iModifiedDevice->IsValidFriendlyName() &&  
              regDevice->IsValidFriendlyName() ) )
            {
            TRACE_INFO( _L( "[BTEngDevMan]\t DoModifyDeviceL ModifyFriendlyDeviceName ") );
                // If CBTDevice does not contain a valid friendly name, 
                // KNullDesC is returned.
            iRegistry.ModifyFriendlyDeviceNameL( iModifiedDevice->BDAddr(), 
                                                  iModifiedDevice->FriendlyName(), 
                                                  iStatus );
            SetActive();
            }
            iModifyState = ECheckNameless;
        }
    if( !IsActive() && iModifyState == ECheckNameless )
        {
        iModifyState = ECheckNone;
        if( iModifiedDevice->AsNamelessDevice() != regDevice->AsNamelessDevice() )
            {
            TRACE_INFO( _L( "[BTEngDevMan]\t DoModifyDeviceL Modify NamelessDevice ") );           
                // Update all other parameters.
            iRegistry.ModifyDevice( iModifiedDevice->AsNamelessDevice(), iStatus );
            SetActive();
            iModifyState = ECheckFinal; // One more cycle needed to complete.
            }
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// This is the default implementation of the MBTEngDevManObserver methods 
// HandleDevManComplete just returns.
// ---------------------------------------------------------------------------
//
EXPORT_C void MBTEngDevManObserver::HandleDevManComplete( TInt /*aErr*/ )
    {
    }


// ---------------------------------------------------------------------------
// This is the default implementation of the MBTEngDevManObserver methods 
// HandleGetDevicesComplete just returns.
// ---------------------------------------------------------------------------
//
EXPORT_C void MBTEngDevManObserver::HandleGetDevicesComplete( TInt /*aErr*/, 
    CBTDeviceArray* /*aDeviceArray*/ )
    {
    }
