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
* Description:  USB obexservicemanager client implementation
*
*/


#include "usbobex.h"
#include "debug.h"
#include "obexsmclient.h"
#include "usbclientwrapper.h"

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
EXPORT_C CObexUSB* CObexUSB::NewL()
    {
    TRACE_FUNC
    CObexUSB* self = new (ELeave) CObexUSB();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CObexUSB::~CObexUSB()
    {
    delete iUSBWrapper;
    iUSBWrapper = NULL;
    }
    
// ---------------------------------------------------------------------------
// ManageUSBServices
// ---------------------------------------------------------------------------
//
EXPORT_C void CObexUSB::ManageUSBServices(TBool aUSBStatus, TRequestStatus &aStatus)
    {
    iUSBWrapper->ManageUSBServices( aUSBStatus, aStatus );
    }
    
// ---------------------------------------------------------------------------
// CancelManageUSBServices
// ---------------------------------------------------------------------------
//
EXPORT_C void  CObexUSB::CancelManageUSBServices()
    {    
    iUSBWrapper->CancelManageUSBServices();
    }   
    
// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CObexUSB::CObexUSB()    
    {	        
	}
	
// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CObexUSB::ConstructL()
    {    
    TRACE_FUNC    
    iUSBWrapper = CUSBClientWrapper::NewL();
    }  

