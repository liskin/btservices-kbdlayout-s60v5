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
* Description:  This is the ObexServiceManager ecom plugin class implementation.
*
*/


#include "ObexSMPlugin.h"
#include "ObexSMRequester.h"
#include "debug.h"

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
CObexSMPlugin* CObexSMPlugin::NewL(TLocodServicePluginParams& aParams)
    {
    TRACE_FUNC
    CObexSMPlugin* self = new (ELeave) CObexSMPlugin(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CObexSMPlugin::~CObexSMPlugin()
    {
    TRACE_FUNC_ENTRY      
    iRequests.ResetAndDestroy();
    iRequests.Close();    
    if(iObexSMServer.Handle())
        {              
        iObexSMServer.Close();       
        }    
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ManageService
// ---------------------------------------------------------------------------
//
void CObexSMPlugin::ManageService(TLocodBearer aBearer, TBool aStatus)
    {
    TRACE_FUNC_ENTRY    
    TInt err = KErrNone; 
    CObexSMRequester* request=NULL;
    
    if(!iObexSMServer.Handle())
    	{
       	err = iObexSMServer.Connect();              	
    	}    
    if(err == KErrNone &&  aBearer != ELocodBearerUSB)
    	{    	
    	TRAP(err, request = CObexSMRequester::NewL(this, aBearer, aStatus)); 
    	if(err==KErrNone)
    	    {
            err=iRequests.Append( request );     
            if(err==KErrNone)
                {
                request->ActivateRequest();               
                }
            }        
    	}    
    else if(err == KErrNone &&  aBearer == ELocodBearerUSB) 	
        {
        Observer().ManageServiceCompleted(aBearer,aStatus,ImplementationUid(), KErrNone); 	           
        }
        
    if(err!=KErrNone)
        {
        Observer().ManageServiceCompleted(aBearer,aStatus,ImplementationUid(), err); 	       
        }
    TRACE_FUNC_EXIT    
    }
   
// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//

CObexSMPlugin::CObexSMPlugin(TLocodServicePluginParams& aParams)
    : CLocodServicePlugin(aParams)
    {	        
	}

// ---------------------------------------------------------------------------
// ConstructL
// ---------------------------------------------------------------------------
//
void CObexSMPlugin::ConstructL()
    {    
    TRACE_FUNC    
    }

// ---------------------------------------------------------------------------
// RequestComplete
// ---------------------------------------------------------------------------
//
void CObexSMPlugin::RequestComplete(CObexSMRequester* aRequest, TInt aError)
    {   
    TRACE_FUNC_ENTRY    
    
    TLocodBearer bearer= aRequest->GetBearer();
    TBool status = aRequest->GetStatus();       
    for(TInt index=0; index<iRequests.Count();index++)
        {        
        if(aRequest==iRequests[index])
            {
            delete iRequests[index];    
            iRequests.Remove(index);
            iRequests.Compress();
            break;
            }
        }         
    Observer().ManageServiceCompleted(bearer,status,ImplementationUid(), aError); 	   
    
    TRACE_FUNC_EXIT
    }      


// ---------------------------------------------------------------------------
// GetServer
// ---------------------------------------------------------------------------
//
RObexSMServer&  CObexSMPlugin::GetServer()
    {
    TRACE_FUNC    
    return  iObexSMServer;
    }
    
// End of file
