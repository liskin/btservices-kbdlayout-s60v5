/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Active object helper class.
*
*/

#include "btnotifactive.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTNotifActive::CBTNotifActive( MBTNotifActiveObserver* aObserver, 
        TInt aId, TInt aPriority )
: CActive( aPriority ), iRequestId( aId ), iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTNotifActive::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTNotifActive* CBTNotifActive::NewL( MBTNotifActiveObserver* aObserver, 
        TInt aId, TInt aPriority )
    {
    CBTNotifActive* self = new (ELeave) CBTNotifActive( aObserver, aId, aPriority );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTNotifActive::~CBTNotifActive()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
void CBTNotifActive::DoCancel()
    {
    iObserver->DoCancelRequest( this, iRequestId );
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
void CBTNotifActive::RunL()
    {
    iObserver->RequestCompletedL( this, iRequestId, iStatus.Int() );
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
TInt CBTNotifActive::RunError( TInt aError )
    {
    iObserver->HandleError( this, iRequestId, aError );
    return KErrNone;
    }
