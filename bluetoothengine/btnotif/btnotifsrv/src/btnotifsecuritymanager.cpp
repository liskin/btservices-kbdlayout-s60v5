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
* Description: 
*
*/

#include "btnotifsecuritymanager.h"
#include "btnotifoutgoingpairinghandler.h"
#include "btnotifincomingpairinghandler.h"
#include "btnotifpairnotifier.h"
#include "btnotifclientserver.h"
#include <e32property.h>
#include "btnotifconnectiontracker.h"
#include "btnotifserviceauthorizer.h"

/**  Identification for active object */
enum TPairManActiveRequestId
    {
    ESimplePairingResult,
    EAuthenticationResult,
    ERegistryGetLocalAddress,
    };

// ---------------------------------------------------------------------------
// Tells if two TBTNamelessDevice instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool CompareDeviceByAddress( const TBTNamelessDevice& aDevA, const TBTNamelessDevice& aDevB )
    {
    return aDevA.Address() == aDevB.Address();
    }

// ---------------------------------------------------------------------------
// Tells if these two instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool MatchDeviceAddress(const TBTDevAddr* aAddr, const TBTNamelessDevice& aDev)
    {
    return *aAddr == aDev.Address();
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifSecurityManager::CBTNotifSecurityManager(
        CBTNotifConnectionTracker& aParent,
        CBtDevRepository& aDevRepository)
    : iParent( aParent ), iDevRepository( aDevRepository )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::ConstructL()
    {
    // Connect to pairing server for authentication & simple pairing 
    // results directly from the BT stack.
    // Pairing server doesn't exist if we run BT 2.0 stack:
    iPairingServ = new (ELeave) RBluetoothPairingServer;
    TInt err = iPairingServ->Connect();
    if ( err)
        {
        delete iPairingServ;
        iPairingServ = NULL;
        }
    else
        {
        User::LeaveIfError( iPairingResult.Open( *iPairingServ ) );
        User::LeaveIfError( iAuthenResult.Open( *iPairingServ ) );
        iSSPResultActive = CBtSimpleActive::NewL( *this, ESimplePairingResult );
        iAuthenResultActive = CBtSimpleActive::NewL( *this, EAuthenticationResult );
        SubscribeSspPairingResult();
        SubscribeAuthenticateResult();
        }
    User::LeaveIfError( iRegistry.Open( iParent.RegistryServerSession() ) );
    // RProperty for accessing the local device address
    User::LeaveIfError( iPropertyLocalAddr.Attach(
            KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetLocalDeviceAddress) );
    // Initialise paired devices list
    iLocalAddrActive = CBtSimpleActive::NewL( *this, ERegistryGetLocalAddress );
    SubscribeLocalAddress();
    iPairNotifier = CBTNotifPairNotifier::NewL( *this );
    iDevRepository.AddObserverL( this );
    iServiceAuthorizer = CBTNotifServiceAuthorizer::NewL(*this);
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTNotifSecurityManager* CBTNotifSecurityManager::NewL(
        CBTNotifConnectionTracker& aParent,
        CBtDevRepository& aDevRepository )
    {
    CBTNotifSecurityManager* self = NULL;
    self = new  CBTNotifSecurityManager( aParent, aDevRepository );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifSecurityManager::~CBTNotifSecurityManager()
    {
    delete iSSPResultActive;
    delete iAuthenResultActive;
    delete iPairNotifier;
    delete iPairingHandler;
    iPairedDevices.Close();
    iPairingResult.Close();
    iAuthenResult.Close();
    if ( iPairingServ )
        {
        iPairingServ->Close();
        delete iPairingServ;
        }
    iRegistry.Close();
    delete iLocalAddrActive;
    iPropertyLocalAddr.Close();
    if ( !iMessage.IsNull() )
        {
        iMessage.Complete( KErrCancel );
        }
    delete iServiceAuthorizer;
    }

// ---------------------------------------------------------------------------
// Initialises the paired devices list.
// If the local address is not available from the P&S key 
// KPropertyKeyBluetoothGetLocalDeviceAddress, then the list may need to be 
// updated once the H/W is switched on. This is so that any registry update 
// from a restore operation can be included in the list, without mistaking the 
// new devices for new pairings.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::SubscribeLocalAddress()
    {
    // Check that we have the Bluetooth local address. If we don't then initialise anyway, but subscribe for an update.
    // This allows us to refresh our paired devices list to include updates made to the remote devices table of the 
    // Bluetooth registry from a restore operation. We need to include these devices without mistaking them for new 
    // pairings. We look solely at the P&S key for the address to avoid the condition whereby the address has been
    // entered into the reigstry but the Bluetooth Manager server has not begun the restore process yet. The signalling
    // of the P&S key will cause Bluetooth Manager to update the registry with any restored devices before fulfilling
    // any further requests.

    // Subscribe to local address property in case we need an update.
    iPropertyLocalAddr.Subscribe( iLocalAddrActive->iStatus );
    iLocalAddrActive->SetRequestId( ERegistryGetLocalAddress );
    iLocalAddrActive->GoActive();
    }

// ---------------------------------------------------------------------------
// Tells whether the local address is available from the P&S key 
// KPropertyKeyBluetoothGetLocalDeviceAddress.
// ---------------------------------------------------------------------------
//
TBool CBTNotifSecurityManager::IsLocalAddressAvailable()
    {
    // Attempt to read address from P&S key.
    TBuf8<KBTDevAddrSize> btAddrDes;
    TInt err = iPropertyLocalAddr.Get( btAddrDes );

    // Is the P&S key defined yet? (if not, stack not up yet)
    if ( err == KErrNone )
        {
        // P&S key defined, is local address set? (if not, H/W not initialised yet)
        if ( btAddrDes.Length() == KBTDevAddrSize )
            {
            TBTDevAddr btAddr = btAddrDes;
            if ( btAddr != TBTDevAddr() )
                {
                return ETrue;
                }
            }
        }
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles pairing related requests from BTNotif clients.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::HandleBondingRequestL( const RMessage2& aMessage )
    {
    TInt opcode = aMessage.Function();
    TBTDevAddrPckgBuf addrPkg;
    switch( opcode )
        {
        case EBTNotifPairDevice:
            {
            if ( !iMessage.IsNull() )
                {
                User::Leave( KErrServerBusy );
                }
            TBTDevAddrPckgBuf addrPkg;
            aMessage.ReadL( EBTNotifSrvParamSlot, addrPkg );
            BlockDevice(addrPkg(),EFalse);
            UnpairDevice( addrPkg() );
            PairDeviceL( addrPkg(), aMessage.Int2() );
            iMessage = RMessage2( aMessage );
            break;
            }
        case EBTNotifCancelPairDevice:
            {
            // Only the client who requested pairing can cancel it:
            if ( !iMessage.IsNull() && aMessage.Session() == iMessage.Session() )
                {
                iPairingHandler->CancelOutgoingPair();
                iMessage.Complete( KErrCancel );
                }
            aMessage.Complete( KErrNone );
            break;
            }
        default:
            {
            User::Leave( KErrArgument );
            }
        }
    }

// ---------------------------------------------------------------------------
// Process a client message related to notifiers.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::HandleNotifierRequestL( const RMessage2& aMessage )
    {
    if(aMessage.Int0() == KBTManAuthNotifierUid.iUid)
        {
        iServiceAuthorizer->StartNotifierL( aMessage );
        }
    else
        {
        iPairNotifier->StartPairingNotifierL( aMessage );
        }
    
    BOstraceFunctionExit0( DUMMY_DEVLIST);
    }

// ---------------------------------------------------------------------------
// Returns the RBluetoothPairingServer instance.
// ---------------------------------------------------------------------------
//
RBluetoothPairingServer* CBTNotifSecurityManager::PairingServer()
    {
    return iPairingServ;
    }

// ---------------------------------------------------------------------------
// Access the reference of RSockServ
// ---------------------------------------------------------------------------
//
RSocketServ& CBTNotifSecurityManager::SocketServ()
    {
    return iParent.SocketServerSession();
    }

// ---------------------------------------------------------------------------
// Access the reference of RBTRegSrv
// ---------------------------------------------------------------------------
//
CBtDevRepository& CBTNotifSecurityManager::BTDevRepository()
    {
    return iDevRepository;
    }

// ---------------------------------------------------------------------------
// Access the reference of CBTNotifConnectionTracker
// ---------------------------------------------------------------------------
//
CBTNotifConnectionTracker& CBTNotifSecurityManager::ConnectionTracker()
    {
    return iParent;
    }

// ---------------------------------------------------------------------------
// Deletes the current pairing handler and transfer the responsibility
// to the specified.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::RenewPairingHandler( 
        CBTNotifBasePairingHandler* aPairingHandler )
    {
    delete iPairingHandler;
    iPairingHandler = aPairingHandler;
    }

// ---------------------------------------------------------------------------
// Find the session who requested this and completes its request.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::OutgoingPairCompleted( TInt aErr )
    {
    // the meaning of KHCIErrorBase equals KErrNone. Hide this specific BT stack
	// detail from clients:
    if ( !iMessage.IsNull()  )
        {
        iMessage.Complete( (aErr == KHCIErrorBase) ? KErrNone : aErr );
        }
    }

// ---------------------------------------------------------------------------
// A session will be ended, completes the pending request for this session.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::SessionClosed( CSession2* aSession )
    {
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST," session %x", aSession);
    if ( !iMessage.IsNull() && iMessage.Session() == aSession )
        {
        iMessage.Complete( KErrCancel );
        }
    }

// ---------------------------------------------------------------------------
// Unpair the device from registry
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::UnpairDevice( const TBTDevAddr& aAddr )
    {
    TIdentityRelation<TBTNamelessDevice> addrComp( CompareDeviceByAddress );
    TBTNamelessDevice dev;
    dev.SetAddress( aAddr );
    // only do unpairing if the we have a link key with it.
    TInt index = iPairedDevices.Find( dev, addrComp );
    if ( index > KErrNotFound )
        {
        dev = iPairedDevices[index];
        TRequestStatus status( KRequestPending );
        // Unpair the device in registry (synchronously)
        iRegistry.UnpairDevice( dev.Address(), status );
        User::WaitForRequest( status );
        BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"Delete link key, res %d", status.Int());
        if ( status == KErrNone )
            {
            TBTDeviceSecurity security = dev.GlobalSecurity();
            // Clear trust setting so that correct icon will be shown in ui applications.
            security.SetNoAuthenticate(EFalse );
            security.SetNoAuthorise(EFalse );
            dev.SetGlobalSecurity(security);
            dev.DeleteLinkKey();
            if ( dev.IsValidUiCookie() && 
                 ( dev.UiCookie() & EBTUiCookieJustWorksPaired ) )
                {
                // Remove the UI cookie bit for Just Works pairing.
                TInt32 cookie = dev.UiCookie() & ~EBTUiCookieJustWorksPaired;
                dev.SetUiCookie( cookie );
                BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"UI cookie %x cleared", EBTUiCookieJustWorksPaired );
                }
            // modify the device in registry synchronously
            // status.Int() could be -1 if the device is not in registry 
            // which is totally fine for us.
            (void) UpdateRegDevice( dev );
            }
        }
    }

void CBTNotifSecurityManager::BlockDevice( const TBTDevAddr& aAddr , TBool aBanned)
    {
    TIdentityRelation<TBTNamelessDevice> addrComp( CompareDeviceByAddress );
    TBTNamelessDevice dev;
    dev.SetAddress( aAddr );
    TRequestStatus status( KRequestPending );
    // Unpair the device in registry (synchronously)
    iRegistry.GetDevice(dev,status);
    User::WaitForRequest( status ); 
    if(status == KErrNone)
        {
        TBTDeviceSecurity security = dev.GlobalSecurity();
        security.SetBanned(aBanned);
        if ( aBanned )
            {
            security.SetNoAuthorise(EFalse);
            }
        dev.SetGlobalSecurity(security);
        (void)UpdateRegDevice(dev);
        }
    }

TInt CBTNotifSecurityManager::AddUiCookieJustWorksPaired( const TBTNamelessDevice& aDev )
    {
    TInt err( KErrNone );
    // There might be UI cookies used by other applications,
    // we should not overwrite them. 
    TInt32 cookie = aDev.IsValidUiCookie() ? aDev.UiCookie() : EBTUiCookieUndefined;
    if ( !( cookie & EBTUiCookieJustWorksPaired ) )
        {
        // Only update the cookie if the wanted one is not in registry yet
        // to keep minimal operations with registry.
        TBTNamelessDevice dev = aDev;		
        cookie |= EBTUiCookieJustWorksPaired;
        dev.SetUiCookie( cookie );
        err = UpdateRegDevice( dev );
        BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"Outgoing Pairing write Ui cookie ret %d", err );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// update a nameless device in registry
// ---------------------------------------------------------------------------
//
TInt CBTNotifSecurityManager::UpdateRegDevice( const TBTNamelessDevice& aDev )
    {
    TRequestStatus status( KRequestPending );
    // update the device in registry synchronously
    iRegistry.ModifyDevice( aDev, status );
    User::WaitForRequest( status );
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"UpdateRegDevice, ret %d", status.Int());
    return status.Int();
    }

// ---------------------------------------------------------------------------
// 0000 for outgoing pairing with a headset.
// The placeholder for future extension (pin code passed in for pairing)
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::GetPinCode(
        TBTPinCode& aPin, const TBTDevAddr& aAddr, TInt aMinPinLength )
    {
    if ( iPairingHandler )
        {
        iPairingHandler->GetPinCode( aPin, aAddr, aMinPinLength );
        }
    else
        {
        // make sure not to leave any text as PIN.
        aPin.Zero();
        aPin().iLength = 0;
        }
    }

// ---------------------------------------------------------------------------
// Ask server class the connection status of the specified device
// ---------------------------------------------------------------------------
//
TBTEngConnectionStatus CBTNotifSecurityManager::ConnectStatus( const TBTDevAddr& aAddr )
    {
    const CBtDevExtension* devExt = iDevRepository.Device(aAddr);
    TBTEngConnectionStatus status = EBTEngNotConnected;
    if ( devExt ) 
        {
        status = devExt->ServiceConnectionStatus();
        }
    return status;
    }

// ---------------------------------------------------------------------------
// From class MBTNotifPairingAOObserver.
// Checks if there is an authentication result.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::RequestCompletedL( CBtSimpleActive* aActive, TInt aStatus )
    {
    BOstraceExt2(TRACE_DEBUG,DUMMY_DEVLIST,"aId: %d, aStatus: %d", aActive->RequestId(), aStatus);
    // Check which request is completed.
    switch( aActive->RequestId() )
        {
        case ESimplePairingResult:
            {
            TBTDevAddr tmpAddr = iSimplePairingRemote;
            if (aStatus != KErrServerTerminated)
                {
                SubscribeSspPairingResult();
                }
            HandlePairingResultL( tmpAddr, aStatus );
            break;
            }
        case EAuthenticationResult:
            {
            TBTDevAddr tmpAddr = iAuthenticateRemote;
            if (aStatus != KErrServerTerminated)
                {
                SubscribeAuthenticateResult();
                }
            HandlePairingResultL( tmpAddr, aStatus );
            break;
            }
        case ERegistryGetLocalAddress:
            {
            TBool value = IsLocalAddressAvailable();
            SubscribeLocalAddress();
            if ( value ) 
                {
                // Refresh paired devices list to include any restored devices.
                iDevRepository.ReInitialize();
                }
            break;
            }
        default:
            // Should not be possible, but no need for handling.
            break;
        }
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// cancels an outstanding request according to the given id.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::CancelRequest( TInt aRequestId )
    {
    switch ( aRequestId )
        {
        case ESimplePairingResult:
            {
            // Cancel listening Simple pairing result
            iPairingResult.CancelSimplePairingResult();
            break;
            }
        case EAuthenticationResult:
            {
            // Cancel listening authentication result
            iAuthenResult.CancelAuthenticationResult();
            break;
            }
        case ERegistryGetLocalAddress:
            {
            // cancel listening local address status change
            iPropertyLocalAddr.Cancel();
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// From class MBtSimpleActiveObserver.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::HandleError( CBtSimpleActive* aActive, TInt aError )
    {
    BOstraceExt2(TRACE_DEBUG,DUMMY_DEVLIST,"request id: %d, error: %d", aActive->RequestId(), aError);
    (void) aActive;
    (void) aError;
    }

// ---------------------------------------------------------------------------
// From class MBtDevRepositoryObserver.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::RepositoryInitialized()
    {
    TRAPD(err, UpdatePairedDeviceListL() );
    if ( !err && iPairingHandler )
        {
        // non-null pairing handler means we are involved in a
        // pairing operation already.
        // todo: is some handling for above case needed?
        }
    }

// ---------------------------------------------------------------------------
// From class MBtDevRepositoryObserver.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::DeletedFromRegistry( const TBTDevAddr& aAddr )
    {
    // We are only interested in the removal of a paired device.
    // thus check whether it is in our local paired list:
    TInt i = iPairedDevices.Find( aAddr, MatchDeviceAddress);
    if ( i > KErrNotFound )
        {
        iPairedDevices.Remove( i );
        }
    }

// ---------------------------------------------------------------------------
// From class MBtDevRepositoryObserver.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::AddedToRegistry( const CBtDevExtension& aDevice )
    {
    // We are only interested in paired device.
    if ( CBtDevExtension::IsBonded( aDevice.Device().AsNamelessDevice() ) )
        {
        TRAP_IGNORE( 
                HandleRegistryBondingL( aDevice.Device().AsNamelessDevice() ) );
        }
    }


// ---------------------------------------------------------------------------
// From class MBtDevRepositoryObserver.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::ChangedInRegistry(
        const CBtDevExtension& aDevice, TUint aSimilarity )
    {
    // We are only interested in paired device.
    // thus check whether it is in our local paired list:
    TInt i = iPairedDevices.Find( aDevice.Addr(), MatchDeviceAddress);
    TBool bonded = CBtDevExtension::IsBonded( aDevice.Device().AsNamelessDevice() );
    if ( i == KErrNotFound )
        {
        if ( bonded ) 
            {
            TRAP_IGNORE(
                    HandleRegistryBondingL( 
                            aDevice.Device().AsNamelessDevice() ) );                
            }
        return;
        }
    // Device was inregistry before, but we need to evaluate its bonding
    // status.
    // The given similarity will tell if the linkkey and paired is changed
    // or not.
    TInt pairingProperty = TBTNamelessDevice::EIsPaired 
            | TBTNamelessDevice::ELinkKey;
    if ( ( pairingProperty & aSimilarity) == pairingProperty )
        {
        // no pairing or linkkey change. Nothing to do for pairing handling.
        // but we'd better update local copy just in case other data
        // of this device is needed by someone:
        iPairedDevices[i] = aDevice.Device().AsNamelessDevice();
        return;
        }
    if ( !CBtDevExtension::IsBonded( aDevice.Device().AsNamelessDevice() ) )
        {
        // device is not user-bonded.
        iPairedDevices.Remove( i );
        return;
        }
    // it is a new paired device if its link-key has been upgraded
    if ( aDevice.Device().LinkKeyType() != ELinkKeyUnauthenticatedUpgradable )
        {
        iPairedDevices.Remove( i );
        TRAP_IGNORE(
                HandleRegistryBondingL( 
                        aDevice.Device().AsNamelessDevice() ) );                
        }
    }

// ---------------------------------------------------------------------------
// From class MBtDevRepositoryObserver.
// This class is not interested in such events.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::ServiceConnectionChanged(
        const CBtDevExtension& aDevice, TBool aConnected )
    {
    (void) aDevice;
    (void) aConnected;
    }

// ---------------------------------------------------------------------------
// Activate or deactivate a pairing handler
// ---------------------------------------------------------------------------
//
TInt CBTNotifSecurityManager::SetPairObserver(const TBTDevAddr& aAddr, TBool aActivate)
    {
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"%d", aActivate);
    BtTraceBtAddr0(TRACE_DEBUG,DUMMY_DEVLIST, aAddr );
    TInt err( KErrNone );
    if ( !aActivate )
        {
        if ( iPairingHandler )
            {
            iPairingHandler->StopPairHandling( aAddr );
            }
        return err;
        }
    
    if ( !iPairingHandler)
        {
        // This is an incoming pair, unpair it from registry and 
        // create the handler:
        UnpairDevice( aAddr );
        TRAP( err, iPairingHandler = CBTNotifIncomingPairingHandler::NewL( *this, aAddr ));     
        }
    if ( iPairingHandler)
        {
        // let the handler decide what to do:
        err = iPairingHandler->ObserveIncomingPair( aAddr );
        }
    return err;
    }

// ---------------------------------------------------------------------------
// Delegates the request to current pair handler
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::PairDeviceL( const TBTDevAddr& aAddr, TUint32 aCod )
    {
    if ( !iPairingHandler)
        {
        // no existing pair handling, create one:
        iPairingHandler = CBTNotifOutgoingPairingHandler::NewL( *this, aAddr );
        }
    // let pair handler decide what to do:
    iPairingHandler->HandleOutgoingPairL( aAddr, aCod );
    }

// ---------------------------------------------------------------------------
// cancel Subscribings to simple pairing result and authentication result from
// Pairing Server
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::CancelSubscribePairingAuthenticate()
    {
    if( iSSPResultActive )
        {
        // Cancel listening Simple pairing result
        iSSPResultActive->Cancel();
        }
    if( iAuthenResultActive )
        {
        iAuthenResultActive->Cancel();
        }
    }

// ---------------------------------------------------------------------------
// Subscribes to simple pairing result from Pairing Server (if not already 
// subscribed).
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::SubscribeSspPairingResult()
    {
    if ( !iSSPResultActive->IsActive() )
        {
        iPairingResult.SimplePairingResult( iSimplePairingRemote, iSSPResultActive->RequestStatus() );
        iSSPResultActive->GoActive();
        }
    }

// ---------------------------------------------------------------------------
// Subscribes to authentication result from Pairing Server (if not already
// subscribed).
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::SubscribeAuthenticateResult()
    {
    if ( !iAuthenResultActive->IsActive() )
        {
        // Subscribe authentication result (which requires pairing for unpaired devices)
        iAuthenResult.AuthenticationResult( iAuthenticateRemote, iAuthenResultActive->RequestStatus() );
        iAuthenResultActive->GoActive();
        }
    }

// ---------------------------------------------------------------------------
// Handle a pairing result from the pairing server.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::HandlePairingResultL( const TBTDevAddr& aAddr, TInt aResult )
    {
    BOstrace1(TRACE_DEBUG,DUMMY_DEVLIST,"result %d", aResult);
    BtTraceBtAddr0(TRACE_DEBUG,DUMMY_DEVLIST, aAddr );
 
    if ( !iPairingHandler && ( aResult == KErrNone || aResult == KHCIErrorBase ) )
        {
        // we only create new handler if incoming pairing succeeds.
        // Pairing failure could be caused by user local cancellation, as the  
        // result, the handler was destroyed by notifier. We shall not
        // instantiate the handler again.
        // If a pairing failed due to other reasons than user local cancelling,
        // it will be catched by the already started handler 
        // (except Just Works pairing - no handler for it at all until we receive
        // registry change event. Thus if incoming JWs pairing failed, no user
        // notification will be shown.)
        iPairedDevices.Find( aAddr, MatchDeviceAddress);
        TInt index = iPairedDevices.Find( aAddr, MatchDeviceAddress);
        // If the device is not found in the old paired device list, it is a new
        // paired device:
        if ( index == KErrNotFound)
            {
            // No handler yet, create incoming pairing handler:
            iPairingHandler = CBTNotifIncomingPairingHandler::NewL( *this, aAddr );
            }
        }
    if ( iPairingHandler )
        {
        iPairingHandler->HandlePairServerResult( aAddr, aResult );
        }
    }

// ---------------------------------------------------------------------------
// copy the nameless devices to local array
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::UpdatePairedDeviceListL()
    {
    iPairedDevices.Reset();
    const RDevExtensionArray& alldevs = iDevRepository.AllDevices();
    for ( TInt i = 0; i < alldevs.Count(); i++ )
        {
        if ( CBtDevExtension::IsBonded( alldevs[i]->Device().AsNamelessDevice() ) )
            {
            iPairedDevices.AppendL( alldevs[i]->Device().AsNamelessDevice() );
            }
        }
    }

// ---------------------------------------------------------------------------
// Create incoming pairing handler if no one exists yet.
// ---------------------------------------------------------------------------
//
void CBTNotifSecurityManager::HandleRegistryBondingL(
        const TBTNamelessDevice& aNameless)
    {
    TInt err = iPairedDevices.Append( aNameless );
    if ( !err && !iPairingHandler)
        {
        // New paired device, but no pairing handler yet.
        // this means an incoming pairing has occured:
        TRAP( err, iPairingHandler = 
                CBTNotifIncomingPairingHandler::NewL( *this, aNameless.Address() ) );
        }
    if ( !err )
        {
        // We have a pairing handler now.
        // Ask pair handler to decide what to do:
        iPairingHandler->HandleRegistryNewPairedEvent( 
                aNameless );
        }
    else if ( iPairingHandler )
        {
        // error could occur due to no memory, 
        // let us try aborting pairing handling
        iPairingHandler->StopPairHandling( aNameless.Address() );
        }
    }

void CBTNotifSecurityManager::TrustDevice( const TBTDevAddr& aAddr )
    {
    TIdentityRelation<TBTNamelessDevice> addrComp( CompareDeviceByAddress );
    TBTNamelessDevice dev;
    dev.SetAddress( aAddr );
    TRequestStatus status( KRequestPending );
    
    iRegistry.GetDevice(dev,status);
    User::WaitForRequest( status ); 
    if(status == KErrNone)
        {
        TBTDeviceSecurity security = dev.GlobalSecurity();
        security.SetNoAuthorise(ETrue);
        security.SetBanned(EFalse);
        dev.SetGlobalSecurity(security);
        (void)UpdateRegDevice(dev);
        }
    }

