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
* Description:  This is the bteng ecom plugin class implementation.
*
*/


#include "DisconnectHelper.h"
#include "debug.h"


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
CDisconnectHelper* CDisconnectHelper::NewL( MDisconnectionCallback* aObserver )    
    {
    TRACE_FUNC
    CDisconnectHelper* self = new (ELeave) CDisconnectHelper( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CDisconnectHelper::~CDisconnectHelper()
    {
    TRACE_FUNC_ENTRY                 
    Cancel();    
    TRACE_FUNC_EXIT
    }
       
// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CDisconnectHelper::CDisconnectHelper( MDisconnectionCallback* aObserver ):    
                                     CActive( CActive::EPriorityStandard ),
                                     iObserver( aObserver )                                     
    {           
    }
    
// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CDisconnectHelper::ConstructL()
    {    
    TRACE_FUNC      
    if ( !iObserver )          
        {
        User::Leave(KErrArgument);    
        }
    CActiveScheduler::Add( this );             
    }
        
// ---------------------------------------------------------
// CPBAPplugin::Activate
// ---------------------------------------------------------
//
TInt CDisconnectHelper::Activate() 
    {        
    if ( IsActive() )
        {
        return KErrInUse; 
        }
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );    
    return KErrNone;
    }    
    
// ---------------------------------------------------------
// From class CActive
// CPBAPplugin::RunL
// ---------------------------------------------------------
//
void CDisconnectHelper::RunL( ) 
    {        
    if ( iObserver )
        {
        iObserver->CompleteDisconnection();    
        }  
    }      
    
// ---------------------------------------------------------
// From class CActive
// CPBAPplugin::DoCancel
// ---------------------------------------------------------
//
void CDisconnectHelper::DoCancel()
    {            
    }         
