/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies).
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
*     BT SAP Plugin header definition
*
*
*/


// INCLUDE FILES
#include <ecom/ecom.h>
#include <ecom/implementationproxy.h>
#include <btmanclient.h>
#include <bt_sock.h>
#include "BTSapPlugin.h"
#include "BTSapServerState.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTSapStatusObserver::CBTSapStatusObserver()
//----------------------------------------------------------
//
CBTSapAsyncHelper::CBTSapAsyncHelper(): CActive(CActive::EPriorityStandard)
    {                                
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CBTSapStatusObserver::NewL()
//----------------------------------------------------------
// 
CBTSapAsyncHelper* CBTSapAsyncHelper::NewL(CBTSapServerState* aServerState, MSapAsyncObserver* aObserver) 
    {
    CBTSapAsyncHelper* self = new(ELeave) CBTSapAsyncHelper();
    CleanupStack::PushL( self );
    self->ConstructL(aServerState, aObserver);
    CleanupStack::Pop();
    return self;
    }
 
// ---------------------------------------------------------
// CBTSapAsyncHelper::ConstructL()
//----------------------------------------------------------
//   
void CBTSapAsyncHelper::ConstructL(CBTSapServerState* aServerState, MSapAsyncObserver* aObserver)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapAsyncHelper::ConstructL")));
    
    iServerState = aServerState;
    iObserver = aObserver;
    iOperation = MSapAsyncObserver::EConnect;
    iDiscType = EDisconnectImmediate;
    }

// ---------------------------------------------------------
// CBTSapAsyncHelper::~CBTSapAsyncHelper
//----------------------------------------------------------
//
CBTSapAsyncHelper::~CBTSapAsyncHelper()
    {
    Cancel();
    }
    
// ---------------------------------------------------------
// CBTSapAsyncHelper::AsyncConnect()
//----------------------------------------------------------
//
/*
void CBTSapAsyncHelper::AsyncConnect(const TBTDevAddr& aAddr)
    {
    // Not supported at the moment
    }
*/
  
// ---------------------------------------------------------
// CBTSapAsyncHelper::AsyncCancelConnect()
//----------------------------------------------------------
//
/*
void CBTSapAsyncHelper::AsyncCancelConnect(const TBTDevAddr& aAddr)
    {
    // Not supported at the moment
    }
*/

// ---------------------------------------------------------
// CBTSapAsyncHelper::AsyncDissconnect()
//----------------------------------------------------------
//    
void CBTSapAsyncHelper::AsyncDisconnect(const TBTDevAddr& /*aAddr*/, TBTSapDisconnectType aDiscType)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapAsyncHelper::AsyncDisconnect")));
    
    if (IsActive())
        {
        return;
        }
    iOperation = MSapAsyncObserver::EDisconnect;
    iDiscType = aDiscType;
    TRequestStatus* ownStatus;
    ownStatus = &iStatus;
    *ownStatus = KRequestPending;
    SetActive();
    User::RequestComplete(ownStatus, KErrNone);;
    }
    

// ---------------------------------------------------------
// CBTSapAsyncHelper::DoCancel()
// ---------------------------------------------------------
//
void CBTSapAsyncHelper::DoCancel()
    {
    }

// ---------------------------------------------------------
// CBTSapAsyncHelper::RunL()
// ---------------------------------------------------------
//
void CBTSapAsyncHelper::RunL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapAsyncHelper::RunL")));
    
    TInt r = KErrNone;
    
    switch(iOperation)
        {
        case MSapAsyncObserver::EDisconnect:
            {
            r = iServerState->DisconnectSapConnection(iDiscType);
            return; // Disconnect callback will happen after a socket is closed, not yet here
            }
        default:
            {
            // Nothing
            }
        }
    iObserver->OperationCompletedL(iOperation, r);
    }

// ---------------------------------------------------------
// CBTSapPlugin::CBTSapPlugin()
// ---------------------------------------------------------
//
CBTSapPlugin::CBTSapPlugin()
    : iServiceState(EServiceOff)
    {
    }

// ---------------------------------------------------------
// CBTSapPlugin::~CBTSapPlugin()
// ---------------------------------------------------------
//
CBTSapPlugin::~CBTSapPlugin()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  ~CBTSapPlugin")));
    
    if(iBTSapServerState)
        {
        iBTSapServerState->DisconnectSapConnection(EDisconnectImmediate);
        }

    if(iAsyncHelper)
        {
        iAsyncHelper->Cancel();
        delete iAsyncHelper;
        }
    
    delete iBTSapServerState;
    iBTSapServerState = NULL;
    }

// ---------------------------------------------------------
// CBTSapPlugin::NewL
// ---------------------------------------------------------
//
CBTSapPlugin* CBTSapPlugin::NewL()
    {
    CBTSapPlugin* self=new(ELeave) CBTSapPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CBTSapPlugin::ConstructL
// ---------------------------------------------------------
//
void CBTSapPlugin::ConstructL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::ConstructL")));
    iBTSapServerState = CBTSapServerState::NewL(*this);
    iAsyncHelper = CBTSapAsyncHelper::NewL(iBTSapServerState, this);
    StartBTSapServiceL(); // Creates iBTSapServerState
    }

// ---------------------------------------------------------
// CBTSapPlugin::SetObserver, from CBTEngPlugin
// ---------------------------------------------------------
//
void CBTSapPlugin::SetObserver( MBTEngPluginObserver* aObserver )
    {
    iObserver = aObserver;
    }

// ---------------------------------------------------------
// CBTSapPlugin::GetSupportedProfiles, from CBTEngPlugin
// ---------------------------------------------------------
//
void CBTSapPlugin::GetSupportedProfiles( RProfileArray& aProfiles )
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::GetSupportedProfiles")));
    
    aProfiles.Append(EBTProfileSAP);
    }

// ---------------------------------------------------------
// CBTSapPlugin::IsProfileSupported, from CBTEngPlugin
// ---------------------------------------------------------
//
TBool CBTSapPlugin::IsProfileSupported( const TBTProfile aProfile ) const
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::IsProfileSupported")));
    
    return (aProfile == EBTProfileSAP) ? ETrue : EFalse;
    }

// ---------------------------------------------------------
// CBTSapPlugin::Connect, from CBTEngPlugin
// ---------------------------------------------------------
//
TInt CBTSapPlugin::Connect( const TBTDevAddr& /*aAddr*/ )
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::Connect")));
    
    // A SAP connection can only be created by a remote device
    return KErrNotSupported;
    }

// ---------------------------------------------------------
// CBTSapPlugin::CancelConnect, from CBTEngPlugin
// ---------------------------------------------------------
//
void CBTSapPlugin::CancelConnect( const TBTDevAddr& /*aAddr*/ )
    {
    // A SAP connection can only be created by a remote device
    }

// ---------------------------------------------------------
// CBTSapPlugin::Disconnect, from CBTEngPlugin
// ---------------------------------------------------------
//
TInt CBTSapPlugin::Disconnect( const TBTDevAddr& aAddr, TBTDisconnectType aDiscType )
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::Disconnect")));
    
    TInt r = KErrNotSupported;
    TBTDevAddr btDevAddr;
    iAddr = aAddr;

    r = GetRemoteBTAddress(btDevAddr);

    if (!r)
        {
        if (btDevAddr != aAddr)
            {
            return KErrNone;
            }
        }
    else
        {
        return r;
        }
    
    if (aDiscType == EBTDiscImmediate)
        {
        r = DisconnectSapConnection(EDisconnectImmediate);
        }
    else if (aDiscType == EBTDiscGraceful)
        {
        r = DisconnectSapConnection(EDisconnectGraceful);
        }
    return r;
    }

// ---------------------------------------------------------
// CBTSapPlugin::GetConnections, from CBTEngPlugin
// ---------------------------------------------------------
//    
void CBTSapPlugin::GetConnections( RBTDevAddrArray& aAddrArray, TBTProfile aConnectedProfile )
    {
    TBTDevAddr btDevAddr;
    
    if(aConnectedProfile == EBTProfileSAP)
        {
        if (GetRemoteBTAddress(btDevAddr) == KErrNone)
            {
            aAddrArray.Append(btDevAddr);
            }
        }
    }
    
// ---------------------------------------------------------
// CBTSapPlugin::IsConnected, from CBTEngPlugin
// ---------------------------------------------------------
//
TBTEngConnectionStatus CBTSapPlugin::IsConnected( const TBTDevAddr& aAddr )
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::IsConnected")));
    
    TBTDevAddr addr;
    TInt r = KErrNotSupported;
    
    if(iServiceState)
        {
        // Only check connection state if the service is enabled
        if (iBTSapServerState->IsSapConnected())
            {
            r = GetRemoteBTAddress( addr );
            if( !r && addr == aAddr )
                {
                return EBTEngConnected;
                }
            }
        }
        
    // Otherwise there is no connection, for sure
    return EBTEngNotConnected;
    }


// ---------------------------------------------------------
// CBTSapPlugin::StartBTSapServiceL
// ---------------------------------------------------------
//
void CBTSapPlugin::StartBTSapServiceL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::StartBTSapServiceL")));
    
    if(iServiceState == EServiceOff)
        {
        iBTSapServerState->StartL();
        iServiceState = EServiceOn;
        }
    }

// ---------------------------------------------------------
// CBTSapPlugin::AcceptSapConnection
// ---------------------------------------------------------
//
TInt CBTSapPlugin::AcceptSapConnection()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::AcceptSapConnection")));
    
    if(iServiceState)
        {
        // Only accept connection if the service is enabled
        return iBTSapServerState->AcceptSapConnection();
        }
    else
        {
        // Otherwise the state-machine is not ready yet
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR,
                        BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::AcceptSapConnection: Can't accept when service is disabled")));
        return KErrNotReady;
        }
    }

// ---------------------------------------------------------
// CBTSapPlugin::RejectSapConnection
// ---------------------------------------------------------
//
TInt CBTSapPlugin::RejectSapConnection(TBTSapRejectReason aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::RejectSapConnection")));
    
    if(iServiceState)
        {
        // Only reject connection if the service is enabled
        return iBTSapServerState->RejectSapConnection(aReason);
        }
    else
        {
        // Otherwise the state-machine is not ready yet
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR,
                        BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::RejectSapConnection: Can't reject when service is disabled")));
        return KErrNotReady;
        }
    }

// ---------------------------------------------------------
// CBTSapPlugin::DisconnectSapConnection
// ---------------------------------------------------------
//
TInt CBTSapPlugin::DisconnectSapConnection(TBTSapDisconnectType aType)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::DisconnectSapConnection")));
    
    if(iServiceState)
        {
        TBTDevAddr addr;
        // Only disconnect SAP if the service is enabled
        iAsyncHelper->AsyncDisconnect(addr, aType);
        return KErrNone;
        }
    else
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR,
                        BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::DisconnectSapConnection: Can't disconnect when service is disabled")));
        // Otherwise the state-machine is not ready yet
        return KErrNotReady;
        }
    }

// ---------------------------------------------------------
// CBTSapPlugin::IsSapConnected
// ---------------------------------------------------------
//
TBool CBTSapPlugin::IsSapConnected()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::IsSapConnected")));
    
    if(iServiceState)
        {
        // Only check connection state if the service is enabled
        return iBTSapServerState->IsSapConnected();
        }
        
    // Otherwise there is no connection, for sure
    return EFalse;
    }

// ---------------------------------------------------------
// CBTSapPlugin::GetRemoteBTAddress
// ---------------------------------------------------------
//
TInt CBTSapPlugin::GetRemoteBTAddress(TBTDevAddr& aBTDevAddr)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::GetRemoteBTAddress")));
    
    if(iServiceState)
        {
        // Only get remote address if the service is enabled
        return iBTSapServerState->GetRemoteBTAddress(aBTDevAddr);
        }
    else
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR,
                        BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::GetRemoteBTAddress: Can't get remote address when service is disabled")));
        // Otherwise the state-machine is not ready yet
        return KErrNotReady;
        }
    }

// ---------------------------------------------------------
// CBTSapPlugin::OperationCompletedL
// ---------------------------------------------------------
//    
void CBTSapPlugin::OperationCompletedL(MSapAsyncObserver::TOperation aOperation, TInt aError)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapPlugin::OperationCompletedL")));
    
    TBTProfile profile = EBTProfileSAP;
    
    switch (aOperation)
        {
        case MSapAsyncObserver::EDisconnect:
            {
            iObserver->DisconnectComplete(iAddr, profile, aError);
            }
        }
    }
    
// ---------------------------------------------------------
// CBTSapPlugin::ConnectComplete
// ---------------------------------------------------------
//    
void CBTSapPlugin::ConnectComplete()
    {
    // Used just for incoming SAP connectios, outgoing SAP connections are not supported.
    // And used only when connection is created successully.
    if ( iObserver )
        {
        TBTDevAddr addr;
        if ( !GetRemoteBTAddress( addr ) )
            {
            iObserver->ConnectComplete( addr, EBTProfileSAP, KErrNone );
            }
        }
    }
    
// -----------------------------------------------------------------------------
// Implementation table is required by ECom. Allows alternative
// New methods to be specified.
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
	{
    IMPLEMENTATION_PROXY_ENTRY(0x101FFE46,CBTSapPlugin::NewL)
	}; 
    
// -----------------------------------------------------------------------------
// ImplementationGroupProxy: Lookup method required by ECom
// Returns the ImplementationTable to the  ECom framework
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
	return ImplementationTable;
	}

// End of file
