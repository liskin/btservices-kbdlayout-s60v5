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
* Description:  obexservicemanager client class implementation
*
*/


// INCLUDE FILES
#include "obexsmclient.h"
#include <e32std.h>
#include "debug.h"


// CONSTANTS
const TInt KServerConnectRetries = 2;       // How many times client tries to make connection to server

// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------
// StartThread
//
// Create the server thread/process depending on the framework.
// This function is exported from the DLL and called from the client
// RObexSMServer::Connect() -method.
// Returns: TInt: Error Code
//
// ---------------------------------------------------------

TInt StartThread()
    {
    TRACE_FUNC  
	
    TInt retVal = KErrNone;
    // create server - if one of this name does not already exist
    TFindServer findSrcs(KSrcsName);
    TFullName name;

    if ( findSrcs.Next( name ) != KErrNone ) // we don't exist already
        {
        TRequestStatus started;
        //TSrcsStart start( started );
        const TUidType serverUid( KNullUid,KNullUid,KSrcsUid );

        // Then we have to create the server
        // This depends on if we are in WINS or target HW environment.
	
		TRACE_INFO(_L("[SRCS]\tclient\tSrcs StartThread(): create the server"));
        // We are on target HW or EKA2 WINS.
        // New process has to be created for the SRCS		
        RProcess server;        
        retVal=server.Create(KSrcsName,             // Full path to SRCS
                             KNullDesC,     // Descriptor of parameters
                             serverUid);            // Triplet UID of executable

        // Check the return value of process creation
        if ( retVal != KErrNone )
            {
            // Loading failed.
			TRACE_ERROR((_L("[SRCS]\tclient\tSrcs StartThread(): process creation failed %d"), retVal));
            return retVal;
            }

		TRACE_INFO(_L("[SRCS]\tclient\tSrcs StartThread(): Process created successfully"));


        // Process/Thread has been created
        // Now logon to the server
        TRequestStatus stat;
        //server.Logon(died);
		server.Rendezvous(stat);

		if (stat!=KRequestPending)
			{
			server.Kill(0);		// abort startup
			//FTRACE(FPrint(_L("c\tclient\tSrcs abort startup.\n")));
			}
		else
			{
			server.Resume();	// logon OK - start the server
			//FTRACE(FPrint(_L("c\tclient\tSrcs Resumed.\n")));
			}

        // Then wait the start or death of the server.
        User::WaitForRequest(stat);

		// we can't use the 'exit reason' if the server panicked as this
		// is the panic 'reason' and may be '0' which cannot be distinguished
		// from KErrNone
		TRACE_INFO((_L("[SRCS]\tclient\tSrcs Server started, code %d (0=>success)\n"), stat.Int()));
		retVal=(server.ExitType()==EExitPanic) ? KErrGeneral : stat.Int();
		
		server.Close(); 
        TRACE_INFO(_L("[SRCS]\tclient\tSrcs server handle closed."));
        }

    return retVal;
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------------------------
// Default constructor
// ---------------------------------------------------------------------------
//
RObexSMServer::RObexSMServer()
    {
    }


// ---------------------------------------------------------
// Connect
// Handles connection to server( creates session )
// ---------------------------------------------------------
//
TInt RObexSMServer::Connect()
    {
    TRACE_FUNC 

    // IF there is allready a session handle,
    // no other session is created.
    if ( Handle() )
        {
        return KErrAlreadyExists;
        }

    // Else try to create a new session with server
    TInt retry= KServerConnectRetries;

    FOREVER
    {
        // Try to make session with server
        TInt retVal=CreateSession(KSrcsName,Version());
        TRACE_INFO((_L("[SRCS]\tclient\tRSrcs: create Session returned: %d"), retVal));
        if ( retVal != KErrNotFound && retVal != KErrServerTerminated )
        {
        // Error which can't be handled happened.
        return retVal;
        }
    // Decrease count
    --retry;
    if ( retry == 0 )
        {
        return retVal;
        }

    // Try to start the server
	TRACE_INFO(_L("[SRCS]\tclient\tRSrcs::Connect(): try to start server"));
    retVal=StartThread();
    TRACE_INFO((_L("[SRCS]\tclient\tRSrcs: StartThread returned: %d"), retVal));

    if ( retVal != KErrNone && retVal != KErrAlreadyExists )
        {
        // Error can't be handled.
        return retVal;
        }
	}	
	
    }

// ---------------------------------------------------------
// Version
// Defines server version number
// ---------------------------------------------------------
//
TVersion RObexSMServer::Version() const
    {
    TRACE_FUNC        
    return( TVersion( KSrcsMajorVersionNumber,
                      KSrcsMinorVersionNumber,
                      KSrcsBuildVersionNumber ));
    }

// ---------------------------------------------------------
// ManageServices
// Sends Manage services command to SRCS.
// ---------------------------------------------------------
//
TInt RObexSMServer::ManageServices(TLocodBearer aBearer, TBool aBearStatus,TRequestStatus &aStatus )
    {
    TRACE_FUNC  
    TInt retVal = KErrNone;

    TPckgBuf<TInt> pckg;
    TIpcArgs args( &pckg, NULL, NULL );
    TSrcsServRequest request;
    
    switch(aBearer)
        {
        case ELocodBearerBT:
            {
            if(aBearStatus)
                {
                request=ESrcsBTServicesON;    
                }
            else
                {
                request=ESrcsBTServicesOFF;    
                }                        
            }
            break;
        case ELocodBearerIR:
            {
            if(aBearStatus)
                {
                request=ESrcsIrDAServicesON;    
                }
            else
                {
                request=ESrcsIrDAServicesOFF;    
                }            
            }
            break;
        case ELocodBearerUSB:
            {
            if(aBearStatus)
                {
                request=ESrcsStartUSB;    
                }
            else
                {
                request=ESrcsStopUSB;    
                }                
            }
            break;
        default:
            {
            TRACE_ERROR(_L("[SRCS]\tclient\tRSrcs Manageservice: Bad bearer"));    
            return KErrArgument;    
            }               
        }    
    if ( Handle() )
        {                       
        SendReceive( request, args, aStatus );        
        }        
    else
        {
        TRACE_ERROR(_L("[SRCS]\tclient\tRSrcs ManageBTServices. No Handle"));
        retVal = KErrBadHandle;
        }                
    return retVal;
    }

// ---------------------------------------------------------
// CancelRequest
// ---------------------------------------------------------
//
void RObexSMServer::CancelRequest()
    {    
    TPckgBuf<TInt> pckg;    
    TIpcArgs args( &pckg, NULL, NULL );
    TSrcsServRequest request;
    request=ESrcsCancelRequest;    
    if ( Handle() )
        {               
        SendReceive( request, args );                     
        }    
    
    }    
//end of file

