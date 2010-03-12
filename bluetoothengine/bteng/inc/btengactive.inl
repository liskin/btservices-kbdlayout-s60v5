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
* Description:  Active object helper class inline function definitions.
*
*/


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
inline CBTEngActive::CBTEngActive(  MBTEngActiveObserver& aObserver, TInt aId, 
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
inline void CBTEngActive::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
inline CBTEngActive* CBTEngActive::NewL( MBTEngActiveObserver& aObserver, 
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
inline CBTEngActive::~CBTEngActive()
    {
    Cancel();
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
inline void CBTEngActive::DoCancel()
    {
    }



// -----------------------------------------------------------------------------
// Get the identifier of this instance.
// -----------------------------------------------------------------------------
//
inline TInt CBTEngActive::RequestId()
    {
    return iRequestId;
    }


// -----------------------------------------------------------------------------
// Set the identifier of this instance.
// -----------------------------------------------------------------------------
//
inline void CBTEngActive::SetRequestId( TInt aId )
    {
    iRequestId = aId;
    }


// -----------------------------------------------------------------------------
// Activate the active object.
// -----------------------------------------------------------------------------
//
inline void CBTEngActive::GoActive()
    {
    SetActive();
    }


// -----------------------------------------------------------------------------
// Cancel an outstanding request.
// -----------------------------------------------------------------------------
//
inline void CBTEngActive::CancelRequest()
    {
    Cancel();
    }


// -----------------------------------------------------------------------------
// Get a reference to the active object request status.
// -----------------------------------------------------------------------------
//
inline TRequestStatus& CBTEngActive::RequestStatus()
    {
    return iStatus;
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
inline void CBTEngActive::RunL()
    {
    iObserver.RequestCompletedL( this, iRequestId, iStatus.Int() );
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
inline TInt CBTEngActive::RunError( TInt aError )
    {
    iObserver.HandleError( this, iRequestId, aError );
    return KErrNone;
    }
