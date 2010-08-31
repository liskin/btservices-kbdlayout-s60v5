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
* Description:  ?Description
*
*/




#include "BTSBIPController.h"
#include "BTServiceClient.h"
#include "BTSUDebug.h"


// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CBTSController::CBTSController()
    {    
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTSController::~CBTSController()
    {    
    FLOG(_L("[BTSU]\t CBTSController::Destructor()"));
    delete iClient;
    }

// ---------------------------------------------------------------------------
// CBTSController::Abort
// ---------------------------------------------------------------------------
//
void CBTSController::Abort()
    {
    FLOG(_L("[BTSU]\t CBTSController::Abort"));        
    if ( iClient )
        {
        iClient->Abort();
        }
    FLOG(_L("[BTSU]\t CBTSController::Abort"));            
    }

void CBTSController::SendUnSupportedFiles()
    {
    
    }

// ---------------------------------------------------------------------------
// CBTSController::Abort
// ---------------------------------------------------------------------------
//
void CBTSController::CreateClientL(MBTServiceClientObserver* aObserver,
                                        const TBTDevAddr& aRemoteDevice,
                                        const TUint aRemotePort,
                                        RArray<CObexHeader*> aHeaderList )                                        
    {
    FLOG(_L("[BTSU]\t CBTSController::CreateClient"));        
    iClient = CBTServiceClient::NewL( aObserver, aRemoteDevice, aRemotePort, aHeaderList );
    FLOG(_L("[BTSU]\t CBTSController::Abort"));            
    }

