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
        SubscribeSspPairingResult();
        SubscribeAuthenticateResult();
        }

    // connect to registry
    User::LeaveIfError( iBTRegistry.Open( BTRegServ() ) );    
    iRegistryActive = CBTEngActive::NewL( *this, ERegistryInitiatePairedDevicesView, CActive::EPriorityStandard );
    // Start to get the list of all paired devices.
    CreatePairedDevicesView( ERegistryInitiatePairedDevicesView );
    iPairedDevices = new (ELeave) RArray<TBTNamelessDevice>;
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
    TRACE_FUNC_EXIT
    }

TInt CBTEngPairMan::AddUiCookieJustWorksPaired( const TBTNamelessDevice& aDev )
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
        TRACE_INFO( ( _L( "[BTENG] CBTEngOtgPair write Ui cookie ret %d"), err ) );
        }
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
        case ERegistryGetPairedDevices:    
            {
            HandleGetPairedDevicesCompletedL( aStatus, aId );
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
// Subscribes to simple pairing result from Pairing Server
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::SubscribeSspPairingResult()
    {
    TRACE_FUNC_ENTRY
    iPairingResult.SimplePairingResult( iSimplePairingRemote, iSSPResultActive->RequestStatus() );
    iSSPResultActive->GoActive();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Subscribes to authentication result from Pairing Server
// ---------------------------------------------------------------------------
//
void CBTEngPairMan::SubscribeAuthenticateResult()
    {
    TRACE_FUNC_ENTRY
    // Subscribe authentication result (which requires pairing for unpaired devices)
    iAuthenResult.AuthenticationResult( iAuthenticateRemote, iAuthenResultActive->RequestStatus() );
    iAuthenResultActive->GoActive();
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
    iNotHandledRegEventCounter = 0;
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
        {// Initialization phase, list paired devices if there are.
        if ( aStatus > KErrNone )
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
        if (iNotHandledRegEventCounter)
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
        // We completed the initialization of paired device list, 
        // move all paired devices to the array:
        UpdatePairedDeviceListL();
        }
    
    if (iNotHandledRegEventCounter)
        { // more registry change detected, create paired device view again:
        CreatePairedDevicesView( ERegistryPairedDevicesNewView );
        }
    else if ( aReqId == ERegistryGetPairedDevices)
        {
        // no more registry change detected, find new pairings:
        CheckPairEventL();
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
            iPairer = CBTEngIncPair::NewL( *this, dev.Address() );
            }
        if ( iPairer )
            {
            // Ask pair handler to decide what to do:
            iPairer->HandleRegistryNewPairedEvent( dev );
            }
        }
    // Free old paired device list resource:
    CleanupStack::PopAndDestroy( 2 );
    TRACE_FUNC_EXIT
    }

