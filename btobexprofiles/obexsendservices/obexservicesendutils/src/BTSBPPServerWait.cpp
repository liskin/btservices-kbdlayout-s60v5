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
* Description:  Waiter class implementation
*
*/



// INCLUDE FILES
#include "BTSBPPServerWait.h"
#include "BTServiceUtils.h"
#include "BTSUDebug.h"

// CONSTANTS
const TInt KBTSBPPWaitTime = 120000000; // 2 minutes


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBPPServerWait::CBTSBPPServerWait
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSBPPServerWait::CBTSBPPServerWait( MBTSBPPServerWaitObserver* aObserver ) : 
    CTimer( EPriorityStandard ), iObserver( aObserver )
                                  
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBTSBPPServerWait::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSBPPServerWait::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTSBPPServerWait::ConstructL()"));

    CTimer::ConstructL();
    After( TTimeIntervalMicroSeconds32( KBTSBPPWaitTime ) );

    FLOG(_L("[BTSU]\t CBTSBPPServerWait::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTSBPPServerWait::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSBPPServerWait* CBTSBPPServerWait::NewL( MBTSBPPServerWaitObserver* aObserver )
    {
    CBTSBPPServerWait* self = new( ELeave ) CBTSBPPServerWait( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

    
// Destructor
CBTSBPPServerWait::~CBTSBPPServerWait()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CBTSBPPServerWait::RunL
// -----------------------------------------------------------------------------
//
void CBTSBPPServerWait::RunL()
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTSBPPServerWait::RunL() status %d"), iStatus.Int() ) );
    __ASSERT_DEBUG( iObserver != NULL, BTSUPanic( EBTSUPanicNullPointer ) );

    iObserver->WaitComplete();

    FLOG(_L("[BTSU]\t CBTSBPPServerWait::RunL() completed"));
    }

//  End of File  
