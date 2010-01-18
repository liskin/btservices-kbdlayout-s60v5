/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the btpbap bteng ecom plugin class implementation.
*
*/

#include "Pbapplugin.h"
#include "DisconnectHelper.h"
#include "debug.h"

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
CPBAPplugin* CPBAPplugin::NewL()
    {
    TRACE_FUNC
    CPBAPplugin* self = new ( ELeave ) CPBAPplugin();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CPBAPplugin::~CPBAPplugin()
    {
    TRACE_FUNC_ENTRY         
    if( iBPAP.Handle() )
      {
      iBPAP.Close();
      }
    delete iDisconnectHelper;  
    iDisconnectHelper = NULL;
    TRACE_FUNC_EXIT
    }   
    
// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CPBAPplugin::CPBAPplugin() : iDisconnectHelper( NULL )
    {               
    }
    
// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CPBAPplugin::ConstructL()
    {    
    TRACE_FUNC_ENTRY        
    TInt error;    
    iDisconnectHelper=CDisconnectHelper::NewL( this );
    error=iBPAP.Connect();
    if ( error == KErrNone )
        {
        error=iBPAP.Start();
        if( error )
            {
            TRACE_ERROR(( _L("[PBAP]\tPBAP\tCPBAPplugin::ConstructL() start failed %d"), error) );
            User::Leave( error );
            }
        }
    else
        {
        TRACE_ERROR(( _L("[PBAP]\tPBAP\tCPBAPplugin::ConstructL() connect failed %d"), error ) );    
        User::Leave( error );
        }
    TRACE_FUNC_EXIT    
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::SetObserver
// ---------------------------------------------------------
//
void CPBAPplugin::SetObserver( MBTEngPluginObserver* aObserver )
    {
    iObserver = aObserver;    
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::GetSupportedProfiles
// ---------------------------------------------------------
//
void CPBAPplugin::GetSupportedProfiles( RProfileArray& aProfiles )
    {
    aProfiles.Append( EBTProfilePBAP );
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::IsProfileSupported
// ---------------------------------------------------------
//
TBool CPBAPplugin::IsProfileSupported( const TBTProfile aProfile ) const
    {
    if ( aProfile == EBTProfilePBAP )
        {
        return ETrue;    
        }
    return EFalse;        
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::Connect
// ---------------------------------------------------------
//
TInt CPBAPplugin::Connect( const TBTDevAddr& /*aAddr*/ )
    {  
    return KErrNotSupported;
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::CancelConnect
// ---------------------------------------------------------
//
void CPBAPplugin::CancelConnect( const TBTDevAddr& /*aAddr*/ )
    {    
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::Disconnect
// ---------------------------------------------------------
//
TInt CPBAPplugin::Disconnect( const TBTDevAddr& aAddr, TBTDisconnectType /*aDiscType*/ )
    {   
    if ( iObserver )   
        {
        return KErrGeneral;    
        }
    iAddr = aAddr;
    return iDisconnectHelper->Activate();
    }

// ---------------------------------------------------------
// From class CBTEngPlugin
// CPBAPplugin::GetConnections
// ---------------------------------------------------------
//    
void CPBAPplugin::GetConnections( RBTDevAddrArray& /*aAddrArray*/, TBTProfile /*aConnectedProfile*/ )
    {    
    }
    
// ---------------------------------------------------------
// CPBAPplugin::IsConnected, from CBTEngPlugin
// ---------------------------------------------------------
//
TBTEngConnectionStatus CPBAPplugin::IsConnected( const TBTDevAddr& /*aAddr*/ ) 
    {    
    return EBTEngNotConnected;  
    }    
    
// ---------------------------------------------------------
// From class MDisconnectionCallback
// CPBAPplugin::CompleteDisconnection
// ---------------------------------------------------------
//
void CPBAPplugin::CompleteDisconnection()
    {     
    
    iBPAP.Stop();
    TInt err = iBPAP.Start();     
    TRACE_INFO((_L("Failed to start RPbapSession err = %d"), err));
    static_cast<void>(err); 
    if ( iObserver )
        {
        iObserver->DisconnectComplete( iAddr, EBTProfilePBAP, KErrNone );    
        }      
    }
