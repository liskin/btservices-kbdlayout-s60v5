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
* Description:  Active object helper class.
*
*/



#include <e32base.h>

#include "btengactive.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngActive::CBTEngActive(  MBTEngActiveObserver& aObserver, TInt aId, 
    TInt aPriority )
:   CActive( aPriority ),
    iRequestId( aId ),
    iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngActive::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
CBTEngActive* CBTEngActive::NewL( MBTEngActiveObserver& aObserver, 
    TInt aId, TInt aPriority )
    {
    CBTEngActive* self = new( ELeave ) CBTEngActive( aObserver, aId, aPriority );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngActive::~CBTEngActive()
    {
    Cancel();
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
void CBTEngActive::DoCancel()
    {
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
void CBTEngActive::RunL()
    {
    iObserver.RequestCompletedL( this, iRequestId, iStatus.Int() );
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
TInt CBTEngActive::RunError( TInt aError )
    {
    iObserver.HandleError( this, iRequestId, aError );
    return KErrNone;
    }
