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
* Description:  Helper class for handling Bluetooth Baseband-related 
*                connection management.
*
*/



#include <wlaninternalpskeys.h>
#include <featmgr.h>

#include "btengsrvbbconnectionmgr.h"
#include "btengserver.h"
#include "btengsrvsettingsmgr.h"
#include "debug.h"

/**  ?description */
const TInt KBTEngSrvBBConnId = 11;
/**  ?description */
const TInt KBTEngSrvWlanStatusId = 12;
/**  ?description */
const TInt KBTEngMaxAddrArraySize = 10;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSrvBBConnMgr::CBTEngSrvBBConnMgr(CBTEngServer* aServer, RSocketServ& aSockServ)
    : iSockServ(aSockServ), iServer(aServer)
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::ConstructL()
    {
    iLinkCountWatcher = CBTEngActive::NewL( *this, KBTEngSrvBBConnId, 
                                             CActive::EPriorityStandard );
        // Subscribe to the BT Baseband link count.
    User::LeaveIfError( iLinkCountProperty.Attach( KPropertyUidBluetoothCategory, 
                                                KPropertyKeyBluetoothGetPHYCount ) );
    // Check if we need to monitor WLAN
    FeatureManager::InitializeLibL();
    iWlanSupported = FeatureManager::FeatureSupported( KFeatureIdProtocolWlan );
    FeatureManager::UnInitializeLib();
    if( iWlanSupported )
        {
        iWlanWatcher = CBTEngActive::NewL( *this, KBTEngSrvWlanStatusId, 
                                                  CActive::EPriorityStandard );
        User::LeaveIfError( iWlanStatusProperty.Attach( KPSUidWlan, 
                                                         KPSWlanIndicator ) );
        }
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSrvBBConnMgr* CBTEngSrvBBConnMgr::NewL(CBTEngServer* aServer,
                                             RSocketServ& aSockServ)
    {
    CBTEngSrvBBConnMgr* self = new( ELeave ) CBTEngSrvBBConnMgr(aServer, aSockServ);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSrvBBConnMgr::~CBTEngSrvBBConnMgr()
    {
	delete iLinkCountWatcher;
	delete iWlanWatcher;    
	iLinkCountProperty.Close();
    iWlanStatusProperty.Close();
	delete iPhyLinks;
    }

// ---------------------------------------------------------------------------
// Start listening to the relevant properties.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::Subscribe()
    {
    if( !iLinkCountWatcher->IsActive() )
        {
        iLinkCountProperty.Subscribe( iLinkCountWatcher->RequestStatus() );
        iLinkCountWatcher->GoActive();
        }
    if( iWlanSupported && !iWlanWatcher->IsActive() )
        {
        iWlanStatusProperty.Subscribe( iWlanWatcher->RequestStatus() );
        iWlanWatcher->GoActive();
        }
    }


// ---------------------------------------------------------------------------
// Stop listening to the subscribed properties.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::Unsubscribe()
    {
    iLinkCountWatcher->Cancel();
    iWlanWatcher->Cancel();
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvBBConnMgr::ManageTopology( TBool aPrepDiscovery )
    {
    TInt linkCount = 0;
    TBool closeSock = ETrue;    // To check if we can close the handle again.
    TInt err = iLinkCountProperty.Get( linkCount );
    if( !err && !linkCount && iAutoSwitchOff )
        {
            // Inform server that there are no active connections anymore.
        iAutoSwitchOff = EFalse;
        iCallBack.CallBack();
        }
    RBTDevAddrArray addrArray;
    TInt minLinks = 1;  // We don't care about our role if we have only one link.
    if( aPrepDiscovery || GetWlanStatus() )
        {
            // If we are performing discovery shortly, or we have an active WLAN 
            // connection, then request master role on all links.
        minLinks = 0;
        }
    if( !err && linkCount > minLinks )
        {
        if( iPhyLinks )
            {
                // If we have an open handle with the socket server (because we
                // are disconnecting all links), then don't close the socket.
            closeSock = EFalse;
            }
            // There are existing links; get the addresses.
        TRAP( err, GetConnectedAddressesL( addrArray ) );
        }
    if( !err && addrArray.Count() > minLinks )
        {
            // Try to become master on all the links.
        RBTPhysicalLinkAdapter btLink;
        for( TInt i = 0; i < addrArray.Count(); i++ )
            {
                // At this point, the results of the operations do not matter 
                // too much. If an error is returned, then that only affects 
                // further operations on the same link, and they should not be 
                // passed back to the caller.
            TUint32 basebandState = 0;
            err = btLink.Open( iSockServ, addrArray[ i ] );
            if( !err )
                {
                    // Mostly for logging purposes, check the current role (the 
                    // request will anyway be ignored if we're already master).
                err = btLink.PhysicalLinkState( basebandState );
                TRACE_INFO( ( 
                    _L( "[BTEng]\t ManageTopology: Current role: %d; status: %d" ), 
                    (TInt) ( basebandState & ENotifyAnyRole ), err ) )
                }
            if( !err && ( basebandState & ENotifySlave ) )
                {
                    // Try to become master of this link. This request could 
                    // be issued without checking the current role, and would 
                    // just be ignored if we are already master. The return 
                    // value is also ignored later on, at this stage it is 
                    // not important if the call succeeds.
                err = btLink.RequestMasterRole();
                TRACE_INFO( ( 
                    _L( "[BTEng]\t Requesting master role; result: %d" ),  err ) )
                }
                // Reset the result, so that it is not passed up 
                // (in case this was the last one in the loop).
            err = KErrNone;
            btLink.Close();
            }
        }
    addrArray.Close();
    if( closeSock )
        {
        delete iPhyLinks;
        iPhyLinks = NULL;
        }
    return err;
    }


// ---------------------------------------------------------------------------
// Gets an array of addresses of remote devices for all Baseband connections.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::GetConnectedAddressesL( RBTDevAddrArray& aAddrArray )
    {
    TInt err = KErrNone;
    if( !iPhyLinks )
        {
        iPhyLinks = CBluetoothPhysicalLinks::NewL( *this, iSockServ );
        }
    if( !err )
        {
        err = iPhyLinks->Enumerate( aAddrArray, KBTEngMaxAddrArraySize );
        }
    User::LeaveIfError( err );
    }


// ---------------------------------------------------------------------------
// Request to disconnect all Bluetooth baseband connections.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::DisconnectAllLinksL( TCallBack& aCallBack )
    {
    TRACE_FUNC_ENTRY
    iCallBack = aCallBack;
    RBTDevAddrArray addrArray;
    GetConnectedAddressesL( addrArray );
    TInt err = KErrNone;
    if( addrArray.Count() > 0 )
        {
        err = iPhyLinks->DisconnectAll();
        }
    else
        {
        err = KErrNotFound;
        }
    addrArray.Close();
    if( err && err != KErrInUse )
        {
            // No connections, or something went wrong; just clean up 
            // and inform our client.
        HandleDisconnectAllCompleteL( err );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Request to disconnect all Bluetooth baseband connections with Power off reason code.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::DisconnectAllLinksForPowerOffL( TCallBack& aCallBack )
    {
    TRACE_FUNC_ENTRY
    iCallBack = aCallBack;
    RBTDevAddrArray addrArray;
    GetConnectedAddressesL( addrArray );
    TInt err = KErrNone;
    if( addrArray.Count() > 0 )
        {
        err = iPhyLinks->DisconnectAll();
        // @todo Once fix is in stack, call this API instead
        // err = iPhyLinks->DisconnectAllForPowerOff();
        }
    else
        {
        err = KErrNotFound;
        }
    addrArray.Close();
    if( err && err != KErrInUse )
        {
            // No connections, or something went wrong; just clean up 
            // and inform our client.
        HandleDisconnectAllCompleteL( err );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Request to disconnect all Bluetooth baseband connections.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::SetAutoSwitchOff( TBool aEnable, TCallBack& aCallBack )
    {
    iAutoSwitchOff = aEnable;
	iCallBack = aCallBack;
    if( aEnable )
        {
        TInt linkCount = 0;
        TInt err = iLinkCountProperty.Get( linkCount );
        if( !err && !linkCount )
            {
            iCallBack.CallBack();
            }
        }
    }


// ---------------------------------------------------------------------------
// From class MBluetoothPhysicalLinksNotifier.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::HandleCreateConnectionCompleteL( TInt aErr )
    {
    (void) aErr;
    }


// ---------------------------------------------------------------------------
// From class MBluetoothPhysicalLinksNotifier.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::HandleDisconnectCompleteL( TInt aErr )
    {
    (void) aErr;
    }


// ---------------------------------------------------------------------------
// From class MBluetoothPhysicalLinksNotifier.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::HandleDisconnectAllCompleteL( TInt aErr )
    {
    TRACE_FUNC_ARG( ( _L( "error: %d" ), aErr ) )
    delete iPhyLinks;
    iPhyLinks = NULL;
    iCallBack.CallBack();  // Inform our client.
    (void) aErr;    // There is no way to pass the error code; 
                    // anyway our client is not interested..
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::RequestCompletedL( CBTEngActive* aActive,
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "id: %d; status: %d" ), aActive->RequestId(), aStatus ) )
    ASSERT( aActive->RequestId() == KBTEngSrvBBConnId || aActive->RequestId() == KBTEngSrvWlanStatusId );
    if( aStatus != KErrPermissionDenied )
        {
        // Ignore any other errors.
        // First subscribe again, so that we don't miss any updates.
        Subscribe();
        }
    (void) ManageTopology( EFalse );    // Ignore result; nothing to do 
                                        // about it here.
    if( aActive->RequestId() == KBTEngSrvBBConnId )
        {
        TRACE_INFO( ( _L( "[BTENG] PHY count key changed, update UI connection status" ) ) )
        iServer->SettingsManager()->SetUiIndicatorsL();
        }    
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles cancelation of an outstanding request
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::CancelRequest( TInt aRequestId )
    {
    TRACE_FUNC_ARG( ( _L( "reqID %d" ), aRequestId ) );
    if ( aRequestId == KBTEngSrvBBConnId )
        {
        iLinkCountProperty.Cancel();
        }
    else if ( aRequestId == KBTEngSrvWlanStatusId )
        {
        iWlanStatusProperty.Cancel();
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles an error in RunL( i.e. RequestCompletedL). Does nothing, since our
// RunL cannot actually leave.
// ---------------------------------------------------------------------------
//
void CBTEngSrvBBConnMgr::HandleError( CBTEngActive* aActive,  
    TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "id: %d; status: %d" ), aActive->RequestId(), aError ) )
    (void) aActive;
    (void) aError;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TBool CBTEngSrvBBConnMgr::GetWlanStatus()
    {
    TInt wlanStatus = 0;
    TBool connected = EFalse;
    if( iWlanSupported )
        {
        TInt err = iWlanStatusProperty.Get( wlanStatus );
        if( err )
            {
            wlanStatus = 0; // Reset just to be sure.
            }
        }
    if( wlanStatus == EPSWlanIndicatorActive || 
        wlanStatus == EPSWlanIndicatorActiveSecure )
        {
        connected = ETrue;
        }
    return connected;
    }
