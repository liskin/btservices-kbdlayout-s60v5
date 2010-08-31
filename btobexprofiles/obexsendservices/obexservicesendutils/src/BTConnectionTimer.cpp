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
#include "BTConnectionTimer.h"
#include "BTServiceUtils.h"
#include "BTSUDebug.h"

// CONSTANTS

                                    

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSBPPServerWait::CBTSBPPServerWait
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTConnectionTimer::CBTConnectionTimer( MBTConTimeObserver* aObserver ) : 
    CTimer( EPriorityStandard ), iObserver( aObserver )
                                  
    {
    
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBTConnectionTimer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTConnectionTimer::ConstructL()
    {
    FLOG(_L("[BTSU]\t CBTConnectionTimer::ConstructL()"));

    CTimer::ConstructL();
    

    FLOG(_L("[BTSU]\t CBTConnectionTimer::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTConnectionTimer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTConnectionTimer* CBTConnectionTimer::NewL( MBTConTimeObserver* aObserver )
    {
    CBTConnectionTimer* self = new( ELeave ) CBTConnectionTimer( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

    
// Destructor
CBTConnectionTimer::~CBTConnectionTimer()
    {
    Cancel();
    }
    
// -----------------------------------------------------------------------------
// CBTConnectionTimer::SetTimeOut
// -----------------------------------------------------------------------------
//
void CBTConnectionTimer::SetTimeOut(TTimeIntervalMicroSeconds32 aTimeOutValue)
    {
    iTimeOutValue = aTimeOutValue;
    }
// -----------------------------------------------------------------------------
// CBTConnectionTimer::Start
// -----------------------------------------------------------------------------
//
void CBTConnectionTimer::Start()
    {    
    After( iTimeOutValue );
    }
        
// -----------------------------------------------------------------------------
// CBTConnectionTimer::RunL
// -----------------------------------------------------------------------------
//
void CBTConnectionTimer::RunL()
    {
    FTRACE(FPrint(_L("[BTSU]\t CBTConnectionTimer::RunL() status %d"), iStatus.Int() ) );
    
    if ( iObserver )
        {
        iObserver->ConnectionTimedOut();
        }

    FLOG(_L("[BTSU]\t CBTConnectionTimer::RunL() completed"));
    }

//  End of File  
