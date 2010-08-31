/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The base active class definition 
*
*/


#include "BTServiceDelayedDestroyer.h"
#include "BTSUDebug.h"

// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::NewL()
// -----------------------------------------------------------------------------
//  
CBTServiceDelayedDestroyer* CBTServiceDelayedDestroyer::NewL(CActive::TPriority aPriority)
    {
    CBTServiceDelayedDestroyer* self = CBTServiceDelayedDestroyer::NewLC(aPriority);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::NewLC()
// -----------------------------------------------------------------------------
//  
CBTServiceDelayedDestroyer* CBTServiceDelayedDestroyer::NewLC(CActive::TPriority aPriority)
    {
    CBTServiceDelayedDestroyer* self = new (ELeave) CBTServiceDelayedDestroyer(aPriority);
    CleanupStack::PushL(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::CBTServiceDelayedDestroyer()
// -----------------------------------------------------------------------------
//  
CBTServiceDelayedDestroyer::CBTServiceDelayedDestroyer(CActive::TPriority aPriority)
    : CActive(aPriority), iPtr(NULL)
    {
    FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::CBTServiceDelayedDestroyer()"));
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::~CBTServiceDelayedDestroyer()
// -----------------------------------------------------------------------------
//  
CBTServiceDelayedDestroyer::~CBTServiceDelayedDestroyer()
    {
    FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::Destructor()"));
    if( iPtr )
        {
        delete iPtr;
        iPtr = NULL;
        }
    Cancel();
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::GoActive()
// -----------------------------------------------------------------------------
//  
void CBTServiceDelayedDestroyer::GoActive()
    {
    FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::GoActive()"));
    if( !IsActive() )
        {
        TRequestStatus* status = &iStatus;
        iStatus = KRequestPending;
        User::RequestComplete(status, KErrNone);
    	SetActive();
        }
    else
    	{
        FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::GoActive(), Already active"));
    	}
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::SetDestructPointer()
// Must be called with GoActive, they are pairs, first call SetDestructPointer
// and then GoActive.
// -----------------------------------------------------------------------------
//  
void CBTServiceDelayedDestroyer::SetDestructPointer(CBTEngDiscovery* aPtr)
    {
    FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::SetDestructPointer()"));
    if( iPtr )
        {
        // If previous is not deleted yet, meaning, RunL is not called yet,
        // we can safely delete previous here.
        delete iPtr;
        }
    iPtr = aPtr;
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::DoCancel()
// -----------------------------------------------------------------------------
//  
void CBTServiceDelayedDestroyer::DoCancel()
    {
    FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::DoCancel()"));
    }
    
// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::RunL()
// -----------------------------------------------------------------------------
//  
void CBTServiceDelayedDestroyer::RunL()
    {
    FLOG(_L("[BTSU]\t CBTServiceDelayedDestroyer::RunL()"));
    delete iPtr;
    iPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CBTServiceDelayedDestroyer::RunError()
// -----------------------------------------------------------------------------
//      
TInt CBTServiceDelayedDestroyer::RunError(TInt aError)
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTServiceStarter::RunError() aError = %d"), aError) );
    (void) aError;
    return KErrNone;
    } 
