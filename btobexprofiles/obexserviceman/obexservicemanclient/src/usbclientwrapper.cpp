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
* Description:  Wrapper class for usb client
*
*/



#include "usbclientwrapper.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CUSBClientWrapper::CUSBClientWrapper()
    {    
    TRACE_FUNC              
    }


// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CUSBClientWrapper::ConstructL()
    {    
    TRACE_FUNC
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CUSBClientWrapper* CUSBClientWrapper::NewL()
    {
    CUSBClientWrapper* self = new (ELeave) CUSBClientWrapper();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop( self );    
    return self;
    }

// ---------------------------------------------------------------------------
// destructor
// ---------------------------------------------------------------------------
//
CUSBClientWrapper::~CUSBClientWrapper()
    {
    TRACE_FUNC_ENTRY          
    if( iServer.Handle() )
        {                 
        iServer.Close();       
        }    
    TRACE_FUNC_EXIT        
    }
    
// ---------------------------------------------------------------------------
// ManageUSBServices
// ---------------------------------------------------------------------------
//
void  CUSBClientWrapper::ManageUSBServices(TBool aUSBStatus, 
                                           TRequestStatus &aStatus)
    {
    TRACE_FUNC_ENTRY        
    TInt err = KErrNone;         
    TRequestStatus* status = &aStatus;
    aStatus = KRequestPending;    
    if( !iServer.Handle() )
    	{    	
       	err = iServer.Connect();              	       	
    	}    
    if( err != KErrNone )
        {                
        User::RequestComplete( status, err );            
        return;
        }       
    err=iServer.ManageServices( ELocodBearerUSB, aUSBStatus, aStatus );        
    if ( err != KErrNone )
        {
        User::RequestComplete( status, err );
        }    
    TRACE_FUNC_EXIT        
    }

// ---------------------------------------------------------------------------
// CancelManageUSBServices
// ---------------------------------------------------------------------------
//
void  CUSBClientWrapper::CancelManageUSBServices()
    {
    if ( iServer.Handle() )
        {
        iServer.CancelRequest();     
        }        
    }

