/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of obexservicemanager core.
*
*/


// INCLUDE FILES

#include "obexserviceman.h"
#include "obexsm.h"
#include "SrcsSession.h"
#include "SrcsServiceManager.h"
#include "debug.h"
#include "SrcsSecurityPolicy.h"
#include "obexutilsmessagehandler.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CObexSM::CObexSM(TInt aPriority) 
	// aPriority: priority of AO
	// KSrcsPolicy: TPolicy structure of Faxmodem
	:CPolicyServer( aPriority, KSrcsPolicy )
    {
    }
// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CObexSM::~CObexSM()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcs: Destructor") );

    delete iServiceHandler;

   }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CObexSM* CObexSM::NewL()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcs: NewL") );
    CObexSM* self=new(ELeave) CObexSM( EPriorityBackground );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CObexSM::ConstructL()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcs: ConstrucL") );
    // and class for service handling
    iServiceHandler = CSrcsServiceManager::NewL();
    // Then we are ready to start server
    StartL(KSrcsName);
    // Do server initialisation
    InitialiseServerL();
    }

// ---------------------------------------------------------
// NewSessionL
// Create session(s) to client(s)
//
// ---------------------------------------------------------
//
CSession2* CObexSM::NewSessionL( const TVersion& aVersion, const RMessage2& ) const
    {
    FLOG(_L("[SRCS]\tserver\tCSrcs NewSessionL"));

    // check we're the right version
    TVersion srvVersion(KSrcsMajorVersionNumber,
                        KSrcsMinorVersionNumber,
                        KSrcsBuildVersionNumber);

    if (!User::QueryVersionSupported(srvVersion,aVersion))
        {
        User::Leave(KErrNotSupported);
        }

    // make new session
        
    return ( CSrcsSession::NewL(const_cast<CObexSM*>(this)) );
    }

// ---------------------------------------------------------
// InitialiseServerL
// Start Media Watchers at this point.
// ---------------------------------------------------------
//
void CObexSM::InitialiseServerL()
    {
    FLOG(_L("[SRCS]\tserver\tSrcs Initialise Server"));
    // If phone battery is removed during file receiving, 
    // There will be two temp file objects left in Received Folders.
    // We clean those temp files here. 
    TFileName filename;
    TObexUtilsMessageHandler::RemoveTemporaryRFileL (filename);  
    FLOG(_L("[SRCS]\tserver\tSrcs Initialize server complete."));
    }

// ---------------------------------------------------------
// ManageServices
// Initialise services according to parameter value.
// ---------------------------------------------------------
//
TInt CObexSM::ManageServices(TSrcsTransport aTransport, TBool aState, 
    MObexSMRequestObserver* aObserver, const RMessage2& aMessage)
    {
    FLOG(_L("[SRCS]\tserver\tSrcs ManageServicesL"));
    // Simply forward call
    return iServiceHandler->ManageServices( aTransport, aState, aObserver, aMessage);
    }
// ---------------------------------------------------------
// IncSessionCount
// Increase session count.
// ---------------------------------------------------------
//
void CObexSM::IncSessionCount()
    {
    FLOG(_L("[SRCS]\tserver\tSrcs IncSessionCount"));                    
    iSessionCount++;    
    }
// ---------------------------------------------------------
// DecSessionCount
// Decrease session count
// ---------------------------------------------------------
//    
void CObexSM::DecSessionCount()
    {
    FLOG(_L("[SRCS]\tserver\tSrcs DecSessionCount"));                
    if ( iSessionCount>0 )
        {    
        iSessionCount--;    
        if (iSessionCount == 0)
            {
            FLOG(_L("[SRCS]\tserver\tSrcs StoppingServer"));            
            CActiveScheduler::Stop();    
            }        
        }    
    }
// End of file
