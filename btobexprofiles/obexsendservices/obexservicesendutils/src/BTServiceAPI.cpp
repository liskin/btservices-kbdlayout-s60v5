/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Btserviceutils API implementation
*
*/



// INCLUDE FILES
#include "BTServiceAPI.h"
#include "BTServiceStarter.h"
#include "BTSUDebug.h"

// CONSTANTS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTServiceAPI::CBTServiceAPI
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTServiceAPI::CBTServiceAPI()
    {
    }

// -----------------------------------------------------------------------------
// CBTServiceAPI::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTServiceAPI::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTServiceAPI::ConstructL()"));
    iStarter = CBTServiceStarter::NewL();
    }

// -----------------------------------------------------------------------------
// CBTServiceAPI::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CBTServiceAPI* CBTServiceAPI::NewL()
    {
    CBTServiceAPI* self = new( ELeave ) CBTServiceAPI();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
    
// Destructor
CBTServiceAPI::~CBTServiceAPI()
    {
    FLOG(_L("[BTSU]\t CBTServiceAPI::Destructor()"));
    if ( iStarter )
    	{    	
    	delete iStarter;
    	}
    FLOG(_L("[BTSU]\t CBTServiceAPI::Destructor() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceAPI::StartServiceL
// -----------------------------------------------------------------------------
//
EXPORT_C void CBTServiceAPI::StartServiceL( TBTServiceType aService, 
                                   CBTServiceParameterList* aList )
    {
    FLOG(_L("[BTSU]\t CBTServiceAPI::StartServiceL()"));
    
    iStarter->StartServiceL( aService, aList );    

    FLOG(_L("[BTSU]\t CBTServiceAPI::StartServiceL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceAPI::StartSynchronousServiceL
// -----------------------------------------------------------------------------
//
EXPORT_C void CBTServiceAPI::StartSynchronousServiceL( TBTServiceType aService, 
                                   CBTServiceParameterList* aList )
    {
    FLOG(_L("[BTSU]\t CBTServiceAPI::StartSynchronousServiceL()"));

    iStarter->StartServiceL( aService, aList, &iSyncWaiter);

    
    FLOG(_L("[BTSU]\t CBTServiceAPI::StartSynchronousServiceL() Wait for completion"))
    iSyncWaiter.Start();
    
    FLOG(_L("[BTSU]\t CBTServiceAPI::StartSynchronousServiceL() completed"));
    }

//  End of File  
