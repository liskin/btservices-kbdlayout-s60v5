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
* Description:  
*
*/


// INCLUDE FILES
#include "obexutilsdialogtimer.h"
#include "obexutilsdebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CObexUtilsDialogTimer::CObexUtilsDialogTimer( CObexUtilsDialog* aParent)
                               : CTimer( EPriorityLow ), 
                                 iParent( aParent )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::ConstructL
// Symbian OS default constructor can leave.
// -----------------------------------------------------------------------------
//
void CObexUtilsDialogTimer::ConstructL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialogTimer::ConstructL()"));

    CTimer::ConstructL();

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialogTimer::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::NewL
// -----------------------------------------------------------------------------
CObexUtilsDialogTimer* CObexUtilsDialogTimer::NewL( CObexUtilsDialog* aParent)
    {
    CObexUtilsDialogTimer* self = 
        new( ELeave ) CObexUtilsDialogTimer( aParent );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CObexUtilsDialogTimer::~CObexUtilsDialogTimer()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::Tickle
// -----------------------------------------------------------------------------
//
void CObexUtilsDialogTimer::Tickle()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialogTimer::Tickle()"));

    Cancel();
    After( iTimeout );

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialogTimer::Tickle() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::RunL
// -----------------------------------------------------------------------------
//
void CObexUtilsDialogTimer::RunL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialogTimer::RunL()"));
    if (iParent)
        {
        iParent->UpdateProgressDialog();
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialogTimer::RunL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialogTimer::SetTimeout
// -----------------------------------------------------------------------------
//
void CObexUtilsDialogTimer::SetTimeout( TTimeIntervalMicroSeconds32 aTimeout )
    {
    iTimeout = aTimeout;
    }

//  End of File  
