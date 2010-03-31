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
* Description:  Pairing result receiver in Bluetooth engine subsystem
*
*/

#include "btengpairman.h"
#include "btengserver.h"
#include "btengsrvsession.h"
#include "btengotgpair.h"
#include "btengincpair.h"
#include "btengclientserver.h"
#include "debug.h"
#include <e32property.h>

/**  Identification for active object */
enum TPairManActiveRequestId
    {
    ESimplePairingResult,
    EAuthenticationResult,
    ERegistryInitiatePairedDevicesView,
    ERegistryPairedDevicesNewView,
    ERegistryInitiatePairedDevicesList,
    ERegistryGetPairedDevices,
    ERegistryGetLocalAddress,
    };

/**  The message argument which holds the Bluetooth address. */
const TInt KBTEngAddrSlot = 0;

// ---------------------------------------------------------------------------
// Tells if two TBTNamelessDevice instances are for the same remote device
// ---------------------------------------------------------------------------
//
TBool CompareDeviceByAddress( const TBTNamelessDevice& aDevA, const TBTNamelessDevice& aDevB )
    {
    return aDevA.Address() == aDevB.Address();
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngPairMan::CBTEngPairMan( CBTEngServer& aServer )
    : iServer( aServer )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::ConstructL()
    {
    TRACE_FUNC_ENTRY
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
        iSSPResultActive = CBTEngActive::NewL( *this, ESimplePairingResult, CActive::EPriorityStandard );
        iAuthenResultActive = CBTEngActive::NewL( *this, EAuthenticationResult, CActive::EPriorityStandard );        
        }

    // RProperty for accessing the local device address
    User::LeaveIfError( iPropertyLocalAddr.Attach(KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetLocalDeviceAddress) );

    // connect to registry
    User::LeaveIfError( iBTRegistry.Open( BTRegServ() ) );    
    iRegistryActive = CBTEngActive::NewL( *this, ERegistryInitiatePairedDevicesView, CActive::EPriorityStandard );
    iPairedDevices = new (ELeave) RArray<TBTNamelessDevice>; 

    // Initialise paired devices list
    iLocalAddrActive = CBTEngActive::NewL( *this, ERegistryGetLocalAddress, CActive::EPriorityStandard );
    InitPairedDevicesList();
 
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngPairMan* CBTEngPairMan::NewL( CBTEngServer& aServer )
    {
    CBTEngPairMan* self = NULL;
    self = new  CBTEngPairMan( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngPairMan::~CBTEngPairMan()
    {
    TRACE_FUNC_ENTRY
    CancelSubscribe();
    delete iSSPResultActive;
    delete iAuthenResultActive;
    delete iRegistryActive;
    delete iPairedDevicesResp;
    delete iPairer;
    if ( iPairedDevices )
        {
        iPairedDevices->Close();
        delete iPairedDevices;
        }
    iBTRegistry.Close();
    iPairingResult.Close();
    iAuthenResult.Close();
    if ( iPairingServ )
        {
        iPairingServ->Close();
        delete iPairingServ;
        }
    if ( !iMessage.IsNull() )
        {
        iMessage.Complete( KErrCancel );
        }
    iPropertyLocalAddr.Cancel();
    iPropertyLocalAddr.Close();
    delete iLocalAddrActive;
    TRACE_FUNC_EXIT
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
void CBTEngPairMan::InitPairedDevicesList()
    {
    TRACE_FUNC_ENTRY

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
                // Non-zero local address is available.
                iPropertyLocalAddr.Cancel();
                iLocalAddrActive->CancelRequest();
                }
            }
        }

    // Perform initialisation of the paired devices list.
    DoInitPairedDevicesList();

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Initialises the paired devices list (second stage)
// This method performs the actual initialisation, now that the local BT H/W
// address had been made available.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::DoInitPairedDevicesList()
    {
    TRACE_FUNC_ENTRY

    if ( !iRegistryActive->IsActive() )
        {
        // Start to get the list of all paired devices.
        CreatePairedDevicesView( ERegistryInitiatePairedDevicesView );
        }
    else
        {
        iNotHandledInitEventCounter++;
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Handles pairing related commands from BTEng clients.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::ProcessCommandL( const RMessage2& aMessage )
    {
    TRACE_FUNC_ENTRY
    TInt opcode = aMessage.Function();
    TBTDevAddrPckgBuf addrPkg;
    switch( opcode )
        {
        case EBTEngSetPairingObserver:
            {
            aMessage.ReadL( KBTEngAddrSlot, addrPkg );
            SetPairObserver( addrPkg(), aMessage.Int1() );
            break;
            }
        case EBTEngPairDevice:
            {
            if ( !iMessage.IsNull() )
                {
                User::Leave( KErrServerBusy );
                }
            TBTDevAddrPckgBuf addrPkg;
            aMessage.ReadL( KBTEngAddrSlot, addrPkg );
            PairDeviceL( addrPkg(), aMessage.Int1() );
            iMessage = RMessage2( aMessage );
            break;
            }
        case EBTEngCancelPairDevice:
            {
            // Only the client who requested pairing can cancel it:
            if ( !iMessage.IsNull() && aMessage.Session() == iMessage.Session() )
                {
                iPairer->CancelOutgoingPair();
                iMessage.Complete( KErrCancel );
                }
            break;
            }
        default:
            {
            TRACE_INFO( ( _L( "CBTEngPairMan ProcessCommandL: bad request (%d)" ), 
                           aMessage.Function() ) )
            User::Leave( KErrArgument );
            }
        }
    TRACE_FUNC_EXIT    
    }

// ---------------------------------------------------------------------------
// Handle a change in BTRegistry remote devices table.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::RemoteRegistryChangeDetected()
    {
    if ( !iRegistryActive->IsActive() )
        {
        CreatePairedDevicesView( ERegistryPairedDevicesNewView );
        }
    else
        {
        iNotHandledRegEventCounter++;
        }
    }

// ---------------------------------------------------------------------------
// Returns the RBluetoothPairingServer instance.
// ---------------------------------------------------------------------------
//
RBluetoothPairingServer* CBTEngPairMan::PairingServer()
    {
    return iPairingServ;
    }

// ---------------------------------------------------------------------------
// Access the reference of RSockServ
// ---------------------------------------------------------------------------
//
RSocketServ& CBTEngPairMan::SocketServ()
    {
    return iServer.SocketServer();
    }

// ---------------------------------------------------------------------------
// Access the reference of RBTRegSrv
// ---------------------------------------------------------------------------
//
RBTRegServ& CBTEngPairMan::BTRegServ()
    {
    return iServer.RegistrServer();
    }

// ---------------------------------------------------------------------------
// Deletes the current pairing handler and transfer the responsibility
// to the specified.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::RenewPairer( CBTEngPairBase* aPairer )
    {
    delete iPairer;
    iPairer = aPairer;
    }

// ---------------------------------------------------------------------------
// Find the session who requested this and completes its request.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::OutgoingPairCompleted( TInt aErr )
    {
    TRACE_FUNC_ENTRY
    // the meaning of KHCIErrorBase equals KErrNone. Hide this specific BT stack
	// detail from clients:
    if ( aErr == KHCIErrorBase )
        {
        aErr = KErrNone;
        }
    // we must complete client's pairing request:
    if ( !iMessage.IsNull()  )
        {
        iMessage.Complete( aErr );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// A session will be ended, completes the pending request for this session.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::SessionClosed( CSession2* aSession )
    {
    TRACE_FUNC_ARG( ( _L( " session %x"), aSession ) )
    if ( !iMessage.IsNull() && iMessage.Session() == aSession )
        {
        iMessage.Complete( KErrCancel );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Unpair the device from registry
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::UnpairDevice( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    TIdentityRelation<TBTNamelessDevice> addrComp( CompareDeviceByAddress );
    TBTNamelessDevice dev;
    dev.SetAddress( aAddr );
    // only do unpairing if the we have a link key with it.
    TInt index = iPairedDevices->Find( dev, addrComp );
    if ( index > KErrNotFound )
        {
        dev = (*iPairedDevices)[index];
        
        TRequestStatus status( KRequestPending );
        // Unpair the device in registry (synchronously)
        iBTRegistry.UnpairDevice( dev.Address(), status );
        User::WaitForRequest( status );
        TRACE_INFO( ( _L( "Delete link key, res %d"), status.Int() ) )
        
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
                TRACE_INFO( ( _L( "UI cookie %x cleared"), EBTUiCookieJustWorksPaired ) );
                }
            // modify the device in registry synchronously
            // status.Int() could be -1 if the device is not in registry 
            // which is totally fine for us.
            (void) UpdateRegDevice( dev );
            }
        }
    TRACE_FUNC_EXIT
    }

TInt CBTEngPairMan::AddUiCookieJustWorksPaired( const TBTNamelessDevice& aDev )
    {
	TRACE_FUNC_ENTRY
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
        TRACE_INFO( ( _L( "[BTENG] CBTEngOtgPair write Ui cookie ret %d"), err ) );
        }
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// update a nameless device in registry
// ---------------------------------------------------------------------------
//
TInt CBTEngPairMan::UpdateRegDevice( const TBTNamelessDevice& aDev )
    {
    TRequestStatus status( KRequestPending );
    // update the device in registry synchronously
    iBTRegistry.ModifyDevice( aDev, status );
    User::WaitForRequest( status );
    TRACE_INFO( ( _L( "UpdateRegDevice, ret %d"), status.Int() ) )
    return status.Int();
    }

// ---------------------------------------------------------------------------
// Ask server class the connection status of the specified device
// ---------------------------------------------------------------------------
//
TBTEngConnectionStatus CBTEngPairMan::IsDeviceConnected( const TBTDevAddr& aAddr )
    {
    return iServer.IsDeviceConnected( aAddr );
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Checks if there is an authentication result.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::RequestCompletedL( CBTEngActive* /*aActive*/, TInt aId, TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "aId: %d, aStatus: %d"), aId, aStatus ) )
        // Check which request completed.
    switch( aId )
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
        case ERegistryInitiatePairedDevicesView:
        case ERegistryPairedDevicesNewView:
            {
            HandleCreatePairedDevicesViewCompletedL( aStatus, aId );
            break;
            }
        case ERegistryInitiatePairedDevicesList:
            {			
			if (iSSPResultActive && iAuthenResultActive)
				{
				SubscribeSspPairingResult();
				SubscribeAuthenticateResult();
				}
            HandleGetPairedDevicesCompletedL( aStatus, aId );
            break;
            }
        case ERegistryGetPairedDevices:    
            {
            HandleGetPairedDevicesCompletedL( aStatus, aId );
            break;
            }
        case ERegistryGetLocalAddress:
            {
            // Refresh paired devices list to include any restored devices.
            DoInitPairedDevicesList();
            break;
            }
        default:
                // Should not be possible, but no need for handling.
            TRACE_INFO( (_L("[BTEng]: CBTEngPairMan::RequestCompletedL unhandled event!!") ) )
            break;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::HandleError( CBTEngActive* aActive, TInt aId, TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "request id: %d, error: %d" ), aId, aError ) )
    (void) aActive;
    (void) aError;
    if ( aId == ERegistryInitiatePairedDevicesList || 
         aId == ERegistryGetPairedDevices )
        {// leave happened in registry operation, delete registry response:
        delete iPairedDevicesResp;
        iPairedDevicesResp = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Activate or deactivate a pairing handler
// ---------------------------------------------------------------------------
//
TInt CBTEngPairMan::SetPairObserver(const TBTDevAddr& aAddr, TBool aActivate)
    {
    TRACE_FUNC_ARG( ( _L( "%d" ), aActivate ) )
    TRACE_BDADDR( aAddr )
    iPairingOperationAttempted = ETrue;
    TInt err( KErrNone );
    if ( !aActivate )
        {
        if ( iPairer )
            {
            iPairer->StopPairHandling( aAddr );
            }
        return err;
        }
    
    if ( !iPairer)
        {
        // This is an incoming pair, unpair it from registry and 
        // create the handler:
        UnpairDevice( aAddr );
        TRAP( err, iPairer = CBTEngIncPair::NewL( *this, aAddr ));     
        }
    if ( iPairer)
        {
        // let the handler decide what to do:
        err = iPairer->ObserveIncomingPair( aAddr );        
        }    
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Delegates the request to current pair handler
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::PairDeviceL( const TBTDevAddr& aAddr, TUint32 aCod )
    {
    iPairingOperationAttempted = ETrue;
    if ( !iPairer)
        {
        // no existing pair handling, create one:
        iPairer = CBTEngOtgPair::NewL( *this, aAddr );
        }
    // let pair handler decide what to do:
    iPairer->HandleOutgoingPairL( aAddr, aCod );
    }

// ---------------------------------------------------------------------------
// cancel Subscribings to simple pairing result and authentication result from
// Pairing Server
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::CancelSubscribe()
    {
    TRACE_FUNC_ENTRY
    if( iSSPResultActive && iSSPResultActive->IsActive() )
        {
            // Cancel listening Simple pairing result
        iPairingResult.CancelSimplePairingResult();
        iSSPResultActive->Cancel();
        }
    if( iAuthenResultActive && iAuthenResultActive->IsActive() )
        {
            // Cancel listening authentication result
        iAuthenResult.CancelAuthenticationResult();
        iAuthenResultActive->Cancel();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Subscribes to simple pairing result from Pairing Server (if not already 
// subscribed).
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::SubscribeSspPairingResult()
    {
    TRACE_FUNC_ENTRY
    if ( !iSSPResultActive->IsActive() )
        {
        iPairingResult.SimplePairingResult( iSimplePairingRemote, iSSPResultActive->RequestStatus() );
        iSSPResultActive->GoActive();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Subscribes to authentication result from Pairing Server (if not already
// subscribed).
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::SubscribeAuthenticateResult()
    {
    TRACE_FUNC_ENTRY
    if ( !iAuthenResultActive->IsActive() )
        {
        // Subscribe authentication result (which requires pairing for unpaired devices)
        iAuthenResult.AuthenticationResult( iAuthenticateRemote, iAuthenResultActive->RequestStatus() );
        iAuthenResultActive->GoActive();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Handle a pairing result from the pairing server.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::HandlePairingResultL( const TBTDevAddr& aAddr, TInt aResult )
    {
    TRACE_FUNC_ARG( (_L("result %d"), aResult ) )
    TRACE_BDADDR( aAddr );
    if ( !iPairer && ( aResult == KErrNone || aResult == KHCIErrorBase ) )
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
        TBTNamelessDevice dev;
        dev.SetAddress( aAddr );
        TIdentityRelation<TBTNamelessDevice> addrComp( CompareDeviceByAddress );
        TInt index = iPairedDevices->Find( dev, addrComp );
        
        // If the device is not found in the old paired device list, it is a new
        // paired device:
        if ( index == KErrNotFound)
            {
            // No handler yet, create incoming pairing handler:
            iPairer = CBTEngIncPair::NewL( *this, aAddr );
            }
        }
    if ( iPairer )
        {
        iPairer->HandlePairServerResult( aAddr, aResult );
        }  

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// issue creating a bonded devices view
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::CreatePairedDevicesView( TInt aReqId )
    {
    TRACE_FUNC_ENTRY
    if ( aReqId == ERegistryInitiatePairedDevicesView )
        {
        iNotHandledInitEventCounter = 0;
        }
    else
        {
        iNotHandledRegEventCounter = 0;
        }
    TBTRegistrySearch searchPattern;
    searchPattern.FindBonded();
    iRegistryActive->SetRequestId( aReqId );
    iBTRegistry.CreateView( searchPattern, iRegistryActive->iStatus );
    iRegistryActive->GoActive();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// gets the paired devices from the view created by CreatePairedDevicesView
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::GetPairedDevices( TInt aReqId )
    {
    TRACE_FUNC_ENTRY
    delete iPairedDevicesResp;
    iPairedDevicesResp = NULL;
    TRAP_IGNORE( iPairedDevicesResp = CBTRegistryResponse::NewL( iBTRegistry ) );
    if ( iPairedDevicesResp )
        {
        iRegistryActive->SetRequestId( aReqId );
        iPairedDevicesResp->Start( iRegistryActive->iStatus );
        iRegistryActive->GoActive();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// re-create a paired device view if registry was changed during the previous
// operation. otherwise if the view is not empty, get the paired devices.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::HandleCreatePairedDevicesViewCompletedL( TInt aStatus, TInt aReqId )
    {
    TRACE_FUNC_ENTRY

    if ( aReqId == ERegistryInitiatePairedDevicesView )
        {// Initialization phase, list paired devices if there are any.
        if ( iNotHandledInitEventCounter )
            {
            // Reinitialisaton detected, create paired device view again:
            (void) iBTRegistry.CloseView();
            CreatePairedDevicesView( ERegistryInitiatePairedDevicesView );
            }
        else if ( aStatus > KErrNone )
            {
            GetPairedDevices( ERegistryInitiatePairedDevicesList );
            }
        else
            {//no paired device, close the view.
            (void) iBTRegistry.CloseView();
            }
        }
    else
        {
        if ( iNotHandledInitEventCounter )
            {
            // We need to reinitialise but we may be pairing.
            // This situation is not expected to arise, as reinitialisation means
            // that the H/W was only just switched on.
            // If we have ever started to take part in a pairing, then prioritise that
            // pairing.
            (void) iBTRegistry.CloseView();
            if ( iPairingOperationAttempted )
                {
                iNotHandledInitEventCounter = 0;
                CreatePairedDevicesView( ERegistryPairedDevicesNewView );
                }
            else
                {
                CreatePairedDevicesView( ERegistryInitiatePairedDevicesView );
                }
            }
        else if (iNotHandledRegEventCounter)
            { // more registry change detected, create paired device view again:
            (void) iBTRegistry.CloseView();
            CreatePairedDevicesView( ERegistryPairedDevicesNewView );
            }          
        else if ( aStatus > KErrNone )
            { // paired device available, get them:
            GetPairedDevices( ERegistryGetPairedDevices );
            }
        else
            {
            // No paired devices in registry, empty local db:
            (void) iBTRegistry.CloseView();
            iPairedDevices->Reset();
            }
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// update paired device list. if registry was changed, create a new view.
// otherwise check for new pairing event.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::HandleGetPairedDevicesCompletedL( TInt /*aStatus*/, TInt aReqId )
    {
    TRACE_FUNC_ENTRY
    (void) iBTRegistry.CloseView();
    if ( aReqId == ERegistryInitiatePairedDevicesList )
        {
        if ( iNotHandledInitEventCounter )
            {
            // Reinitialisation required, create paired device view again:
            CreatePairedDevicesView( ERegistryInitiatePairedDevicesView );
            }
        else
            {
            // We completed the initialisation of paired device list, 
            // move all paired devices to the array:
            UpdatePairedDeviceListL();
            }
        }
    else
        {
        if (iNotHandledInitEventCounter)
            {
            // We need to reinitialise but we may be pairing.
            // This situation is not expected to arise, as reinitialisation means
            // that the H/W was only just switched on.
            // If we have ever started to take part in a pairing, then prioritise that
            // pairing.
            if ( iPairingOperationAttempted )
                {
                iNotHandledInitEventCounter = 0;
                CreatePairedDevicesView( ERegistryPairedDevicesNewView );
                }
            else
                {
                CreatePairedDevicesView( ERegistryInitiatePairedDevicesView );
                }		
            }     
        else if (iNotHandledRegEventCounter)
            { // more registry change detected, create paired device view again:
            CreatePairedDevicesView( ERegistryPairedDevicesNewView );
            }
        else if ( aReqId == ERegistryGetPairedDevices)
           {
            // no more registry change detected, find new pairings:
            CheckPairEventL();
           }
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// copy the nameless devices to local array
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::UpdatePairedDeviceListL()
    {
    TRACE_FUNC_ENTRY
    iPairedDevices->Reset();
    for ( TInt i = 0; i < iPairedDevicesResp->Results().Count(); i++ )
        {
        TRACE_BDADDR( iPairedDevicesResp->Results()[i]->BDAddr() );
        TRACE_INFO((_L("[BTENG]\t linkkeytype %d"), 
                iPairedDevicesResp->Results()[i]->LinkKeyType()))
        iPairedDevices->AppendL( iPairedDevicesResp->Results()[i]->AsNamelessDevice() );
        }
    delete iPairedDevicesResp;
    iPairedDevicesResp = NULL;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// find new paired devices. for each, delegate the information to
// current pair handler.
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::CheckPairEventL()
    {
    TRACE_FUNC_ENTRY
    RArray<TBTNamelessDevice>* pairedDevicesOld;
    pairedDevicesOld = iPairedDevices;
    CleanupStack::PushL( pairedDevicesOld );
    CleanupClosePushL( *pairedDevicesOld );
    iPairedDevices = NULL;
    iPairedDevices = new (ELeave) RArray<TBTNamelessDevice>;
    UpdatePairedDeviceListL();
    
    TIdentityRelation<TBTNamelessDevice> addrComp( CompareDeviceByAddress );
    for ( TInt i = 0; i < iPairedDevices->Count(); i++ )
        {
        TBTNamelessDevice& dev = (*iPairedDevices)[i];        
        TInt index = pairedDevicesOld->Find( dev, addrComp );
 
        // If the device is not found in the old paired device list or
        // the link key type has been changed from 
        // ELinkKeyUnauthenticatedUpgradable, the device is a new 
        // paired device:
        TBool newPaired = dev.LinkKeyType() != ELinkKeyUnauthenticatedUpgradable && 
             ( index == KErrNotFound  || 
                 ( index > KErrNotFound &&
                 dev.LinkKeyType() != (*pairedDevicesOld)[index].LinkKeyType() ) );
        TRACE_BDADDR( dev.Address() );
        if ( newPaired && !iPairer)
            {
            iPairingOperationAttempted = ETrue;
            iPairer = CBTEngIncPair::NewL( *this, dev.Address() );
            }
        if ( newPaired && iPairer )
            {
            // Ask pair handler to decide what to do:
            iPairer->HandleRegistryNewPairedEvent( dev );
            }
        }
    // Free old paired device list resource:
    CleanupStack::PopAndDestroy( 2 );
    TRACE_FUNC_EXIT
    }

