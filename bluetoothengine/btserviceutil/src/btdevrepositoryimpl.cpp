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
* Description: respository of remote Bluetooth devices.
*
*/

#include "btdevrepositoryimpl.h"
#include <e32property.h>
#include <bt_subscribe.h>
#include <btservices/btdevrepository.h>
#include "btserviceutilconsts.h"

// ---------------------------------------------------------------------------
// Tells if two CBTDevice instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool CompareDeviceByAddress( const CBTDevice& aDevA, const CBTDevice& aDevB )
    {
    return aDevA.BDAddr() == aDevB.BDAddr();
    }

// ---------------------------------------------------------------------------
// Tells if these two instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool MatchDeviceAddress(const TBTDevAddr* aAddr, const CBTDevice& aDev)
    {
    return *aAddr == aDev.BDAddr();
    }

// ---------------------------------------------------------------------------
// Tells if these two instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool MatchDeviceAddress(const TBTDevAddr* aAddr, const CBtDevExtension& aDev)
    {
    return *aAddr == aDev.Device().BDAddr();
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBtDevRepositoryImpl::CBtDevRepositoryImpl()
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::ConstructL()
    {
    // connect to registry
    User::LeaveIfError( iBTRegServ.Connect() );
    User::LeaveIfError( iBTRegistry.Open( iBTRegServ ) );
    iRegistryActive = CBtSimpleActive::NewL(
           *this, BtServiceUtil::ECreateRemoteDeviceViewRequest );
    // Start to get the list of devices from registry.
    CreateRemoteDeviceView();
    
    User::LeaveIfError( iBtRegistryKey.Attach(
            KPropertyUidBluetoothCategory, 
            KPropertyKeyBluetoothGetRegistryTableChange ) );

    iRegistryKeyActive = CBtSimpleActive::NewL( *this, BtServiceUtil::ERegistryPubSubWatcher );
    iBtRegistryKey.Subscribe( iRegistryKeyActive->RequestStatus() );
    iRegistryKeyActive->GoActive();
    iBtengConn = CBTEngConnMan::NewL( this );
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBtDevRepositoryImpl* CBtDevRepositoryImpl::NewL()
    {
    CBtDevRepositoryImpl* self = NULL;
    self = new (ELeave) CBtDevRepositoryImpl();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBtDevRepositoryImpl::~CBtDevRepositoryImpl()
    {
    iObservers.Close();
    delete iBtengConn;
    delete iRegistryActive;
    delete iRegRespRemoteDevices;
    iDevices.ResetAndDestroy();
    iDevices.Close();
    iBTRegistry.Close();
    iBTRegServ.Close();
    delete iRegistryKeyActive;
    iBtRegistryKey.Close();
    }

// ---------------------------------------------------------------------------
// AddObserverL
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::AddObserverL( MBtDevRepositoryObserver* aObserver )
    {
    // Do not allow null pointer.
    if ( aObserver )
        {
        iObservers.AppendL( aObserver );
        }
    }

// ---------------------------------------------------------------------------
// RemoveObserver
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::RemoveObserver( MBtDevRepositoryObserver* aObserver )
    {
    TInt i = iObservers.Find( aObserver );
    if ( i >= 0 )
        {
        iObservers.Remove( i );
        }
    }

// ---------------------------------------------------------------------------
// IsInitialized
// ---------------------------------------------------------------------------
//
TBool CBtDevRepositoryImpl::IsInitialized() const
    {
    return iInitialized;
    }

// ---------------------------------------------------------------------------
// AllDevices
// ---------------------------------------------------------------------------
//
const RDevExtensionArray& CBtDevRepositoryImpl::AllDevices() const
    {
    return iDevices;
    }

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------
//
const CBtDevExtension* CBtDevRepositoryImpl::Device( 
        const TBTDevAddr& aAddr ) const
    {
    TInt pos = iDevices.Find( aAddr, MatchDeviceAddress);
    if ( pos > -1 )
        {
        return iDevices[pos];
        }
    return NULL;
    }

// ---------------------------------------------------------------------------
// ReInitialize
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::ReInitialize()
    {
    iInitialized = EFalse;
    if ( !iRegistryActive->IsActive() )
        {
        CreateRemoteDeviceView();
        }
    else
        {
        // This counter-increasing
        // will force to re-create a registry view later.
        ++iNotHandledRegEventCounter;
        }
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Checks if there is an authentication result.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus )
    {
    if ( aActive->RequestId() == BtServiceUtil::ECreateRemoteDeviceViewRequest )
        {
        HandleCreateRemoteDeviceViewCompletedL( aStatus );
        }
    else if ( aActive->RequestId() == BtServiceUtil::EGetRemoteDevicesRequest )
        {
        HandleGetRemoteDevicesCompletedL( aStatus );
        }
    else if ( aActive->RequestId() == BtServiceUtil::ERegistryPubSubWatcher )
        {
        TInt myChangedTable;
        iBtRegistryKey.Subscribe( aActive->RequestStatus() );
        aActive->GoActive();
        TInt err = iBtRegistryKey.Get( myChangedTable );
        if( !err && myChangedTable == KRegistryChangeRemoteTable )
            {
            if ( !iRegistryActive->IsActive() )
                {
                CreateRemoteDeviceView();
                }
            else
                {
                iNotHandledRegEventCounter++;
                }
            }
        }
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// Checks if there is an authentication result.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::CancelRequest( TInt aRequestId )
    {
    if ( aRequestId == BtServiceUtil::ECreateRemoteDeviceViewRequest )
        {
        iBTRegistry.CancelRequest(iRegistryActive->RequestStatus());
        }
    else if ( aRequestId == BtServiceUtil::EGetRemoteDevicesRequest )
        {
        iRegRespRemoteDevices->Cancel();
        }
    else if ( aRequestId == BtServiceUtil::ERegistryPubSubWatcher )
        {
        iBtRegistryKey.Cancel();
        }
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::HandleError( CBtSimpleActive* aActive, TInt aError )
    {
    (void) aError;
    if ( aActive->RequestId() == BtServiceUtil::ECreateRemoteDeviceViewRequest || 
            aActive->RequestId() == BtServiceUtil::EGetRemoteDevicesRequest )
        {// leave happened in registry operation, delete registry response:
        delete iRegRespRemoteDevices;
        iRegRespRemoteDevices = NULL;
        }
    }

// ---------------------------------------------------------------------------
// From class MBTEngConnObserver.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::ConnectComplete( TBTDevAddr& aAddr, TInt aErr, 
                               RBTDevAddrArray* aConflicts)
    {
    // connection is single profile based, to make sure getting the correct status, 
    // we always retrieve it from btengconnman:
    (void)aConflicts;
    (void) aErr;
    TInt pos = iDevices.Find( aAddr, MatchDeviceAddress );
    if ( pos > -1 )
        {
        TBTEngConnectionStatus old = iDevices[pos]->ServiceConnectionStatus();
        TBTEngConnectionStatus  status = EBTEngNotConnected;
        // error returned from the call is treated as not connected.
        (void) iBtengConn->IsConnected( aAddr,  status );
        iDevices[pos]->SetServiceConnectionStatus( status );
        
        if ( old != status &&
             ( status == EBTEngConnected ||
               status == EBTEngNotConnected ) )
            {
            for ( TInt i = 0; i < iObservers.Count(); ++i )
                {
                iObservers[i]->ServiceConnectionChanged( 
                        *(iDevices[pos]), status == EBTEngConnected );
                }
            }
        }
    }

// ---------------------------------------------------------------------------
// From class MBTEngConnObserver.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::DisconnectComplete( TBTDevAddr& aAddr, TInt aErr )
    {
    // unified handling for connections status events:
    ConnectComplete( aAddr, aErr, NULL);
    }

// ---------------------------------------------------------------------------
// issue creating a remote device view from the registry
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::CreateRemoteDeviceView()
    {
    iNotHandledRegEventCounter = 0;
    (void) iBTRegistry.CloseView();
    TBTRegistrySearch searchPattern;
    searchPattern.FindAll();
    iRegistryActive->SetRequestId( BtServiceUtil::ECreateRemoteDeviceViewRequest );
    iBTRegistry.CreateView( searchPattern, iRegistryActive->iStatus );
    iRegistryActive->GoActive();
    }

// ---------------------------------------------------------------------------
// gets the paired devices from the view created by CreatePairedDevicesView
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::GetRemoteDevicesL()
    {
    delete iRegRespRemoteDevices;
    iRegRespRemoteDevices = NULL;
    iRegRespRemoteDevices = CBTRegistryResponse::NewL( iBTRegistry );
    iRegistryActive->SetRequestId( BtServiceUtil::EGetRemoteDevicesRequest );
    iRegRespRemoteDevices->Start( iRegistryActive->iStatus );
    iRegistryActive->GoActive();
    }

// ---------------------------------------------------------------------------
// re-create a paired device view if registry was changed during the previous
// operation. otherwise if the view is not empty, get the remote devices.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::HandleCreateRemoteDeviceViewCompletedL( TInt aStatus )
    {
    // aStatus may indicate the number of devices from registry.
    // However, our algorithm does not rely on this return error 
    // for implementation simplicity.
    (void) aStatus;
    if (iNotHandledRegEventCounter)
        { // more registry change detected, create paired device view again:
        CreateRemoteDeviceView( );
        }
    else
        {
        GetRemoteDevicesL( );
        }
    }

// ---------------------------------------------------------------------------
// update remote device list. if registry was changed, create a new view.
// otherwise inform client for any changes.
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::HandleGetRemoteDevicesCompletedL( TInt aStatus )
    {
    // aStatus may indicate the number of devices from registry.
    // However, our algorithm does not rely on this return error.
    (void) aStatus;
    if (iNotHandledRegEventCounter)
        { // more registry change detected, create paired device view again:
        CreateRemoteDeviceView( );
        return;
        }

    UpdateRemoteDeviceRepositoryL();
    if ( !iInitialized )
        {
        iInitialized = ETrue;
        // The first time we have got the device lists from registry,
        // Get the connections statuses of these devices from bteng.
        for ( TInt i = iDevices.Count() - 1; i > -1; --i )
            {
            TBTEngConnectionStatus  status = EBTEngNotConnected;
            // error returned from the call is treated as not connected.
            (void) iBtengConn->IsConnected( iDevices[i]->Addr(),  status );
            iDevices[i]->SetServiceConnectionStatus( status );
            }
        for ( TInt i = 0; i < iObservers.Count(); ++i )
            {
            iObservers[i]->RepositoryInitialized();
            }
        }
    }

// ---------------------------------------------------------------------------
// update remote devices in local array with the latest data
// ---------------------------------------------------------------------------
//
void CBtDevRepositoryImpl::UpdateRemoteDeviceRepositoryL()
    {
    TIdentityRelation<CBTDevice> addrComp( CompareDeviceByAddress );
    RBTDeviceArray& devsFromReg = iRegRespRemoteDevices->Results();
    for ( TInt i = iDevices.Count() - 1; i > -1; --i )
        {
        TInt pos = devsFromReg.Find( &(iDevices[i]->Device()), addrComp );
        if ( pos > KErrNotFound )
            {
            // Device is found in registry, check if its properties have been changed
            TUint similarity = devsFromReg[pos]->CompareTo( iDevices[i]->Device() );
            TBool changed = EFalse;
            if ( similarity != 
                    ( CBTDevice::EAllNameProperties | 
                      TBTNamelessDevice::EAllNamelessProperties) )
                {
                // This device was updated in registry.
                // move its ownership to local store
                iDevices[i]->SetDeviceL( devsFromReg[pos] );
                changed = ETrue;
                }
            else
                {
                // This device has no update:
                delete devsFromReg[pos];
                }
            // either the instance at pos has been moved or deleted.
            devsFromReg.Remove( pos );
            if ( iInitialized && changed )
                {
                for ( TInt counter = 0; counter < iObservers.Count(); ++counter )
                    {
                    iObservers[counter]->ChangedInRegistry( *iDevices[i], similarity );
                    }
                }
            }
        else
            {
            // This device was deleted from registry.
            // Free it from the local store before informing client.
            TBTDevAddr addr = iDevices[i]->Addr();
            delete iDevices[i];
            iDevices.Remove( i );
            if ( iInitialized )
                {
                for ( TInt counter = 0; counter < iObservers.Count(); ++counter )
                    {
                    iObservers[counter]->DeletedFromRegistry( addr );
                    }
                }
            }
        }
    
    // Remaining devices in iRegRespRemoteDevices are new devices:
    for ( TInt i = devsFromReg.Count()- 1; i > -1 ; --i )
        {
        CBtDevExtension* devExt = CBtDevExtension::NewLC( devsFromReg[i] );
        iDevices.AppendL( devExt );
        CleanupStack::Pop( devExt );
        devsFromReg.Remove( i );
        if ( iInitialized )
            {
            for ( TInt counter = 0; counter < iObservers.Count(); ++counter )
                {
                iObservers[counter]->AddedToRegistry( *devExt );
                }
            }
        }
    // the devices in devsFromReg was either deleted, or moved.
    delete iRegRespRemoteDevices;
    iRegRespRemoteDevices = NULL;
    }
