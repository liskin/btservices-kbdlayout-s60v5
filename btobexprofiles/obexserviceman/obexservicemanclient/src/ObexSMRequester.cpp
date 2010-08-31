/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  obexservicemanager client request class implementation
*
*/



#include "ObexSMPlugin.h"
#include "ObexSMRequester.h"
#include "obexsmclient.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CObexSMRequester* CObexSMRequester::NewL(MObexSMRequestObserver* aObserver, TLocodBearer aBearer, TBool aStatus)
    {
    TRACE_FUNC    
    CObexSMRequester* self = CObexSMRequester::NewLC(aObserver, aBearer,aStatus);
    CleanupStack::Pop( self );
    return self;    
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CObexSMRequester* CObexSMRequester::NewLC(MObexSMRequestObserver* aObserver, TLocodBearer aBearer, TBool aStatus)
    {
    TRACE_FUNC
    CObexSMRequester* self = new( ELeave ) CObexSMRequester(aObserver,aBearer,aStatus);
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;    
    }

// ---------------------------------------------------------------------------
// destructor
// ---------------------------------------------------------------------------
//
CObexSMRequester::~CObexSMRequester()
    {
    TRACE_FUNC    
    Cancel();    
    }


// ---------------------------------------------------------------------------
// DoCancel
// ---------------------------------------------------------------------------
//
void CObexSMRequester::DoCancel()
    {
    
    }
// ---------------------------------------------------------------------------
// RunL
// ---------------------------------------------------------------------------
//    
void CObexSMRequester::RunL()
    {	
    TRACE_FUNC    
    iObserver->RequestComplete(this,iStatus.Int());    
    }
// ---------------------------------------------------------------------------
// RunError
// ---------------------------------------------------------------------------
//    
TInt CObexSMRequester::RunError( TInt aError )
    {
    TRACE_FUNC	
    iObserver->RequestComplete(this,aError);   
    return KErrNone; 
    }
// ---------------------------------------------------------------------------
// ActivateRequest
// ---------------------------------------------------------------------------
//
void CObexSMRequester::ActivateRequest()
    {
    TRACE_FUNC_ENTRY    
    TInt err;
    RObexSMServer smServer;
    if(iObserver)
        {        
        smServer=iObserver->GetServer();    
        iStatus = KRequestPending;
        err=smServer.ManageServices(iBearer, iBearStatus, iStatus );
        if(err==KErrNone)
            {
            SetActive();    
            }
        else
            {
            iObserver->RequestComplete(this,err);        
            }    
        }    
    TRACE_FUNC_EXIT    
    }
// ---------------------------------------------------------------------------
// CObexSMRequester
// ---------------------------------------------------------------------------
//
CObexSMRequester::CObexSMRequester(MObexSMRequestObserver* aObserver,
                                TLocodBearer aBearer, TBool aBearStatus)
                               : CActive(EPriorityStandard),
                               iObserver(aObserver),
                               iBearer(aBearer), 
                               iBearStatus(aBearStatus)
    {
    TRACE_FUNC    
    CActiveScheduler::Add( this );    
    }


// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CObexSMRequester::ConstructL()
    {
    TRACE_FUNC    	
    }

// ---------------------------------------------------------------------------
// GetStatus
// ---------------------------------------------------------------------------
//
TBool CObexSMRequester::GetStatus()
    {
    return iBearStatus;
    }

// ---------------------------------------------------------------------------
// GetBearer
// ---------------------------------------------------------------------------
//    
TLocodBearer CObexSMRequester::GetBearer()
    {
    return iBearer;    
    }
//End of file

    

