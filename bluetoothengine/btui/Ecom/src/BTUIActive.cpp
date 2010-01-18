/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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


#include "BTUIActive.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTUIActive::CBTUIActive(  MBTUIActiveObserver* aObserver, TInt aId, 
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
void CBTUIActive::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTUIActive* CBTUIActive::NewL( MBTUIActiveObserver* aObserver, 
    TInt aId, TInt aPriority )
    {
    CBTUIActive* self = new( ELeave ) CBTUIActive( aObserver, aId, aPriority );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTUIActive::~CBTUIActive()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
// RequestId()
// Return the request ID of this active object.
// ---------------------------------------------------------------------------
//
TInt CBTUIActive::RequestId()	
    	{
    	return iRequestId;
    	}

// ---------------------------------------------------------------------------
// SetRequestId()
// Set a new request ID for this active object.
// ---------------------------------------------------------------------------
//
void CBTUIActive::SetRequestId( TInt aId )
    	{
    	iRequestId = aId;
    	}    	
    	
// ---------------------------------------------------------------------------
// GoActive()
// Set active.
// ---------------------------------------------------------------------------
//
void CBTUIActive::GoActive()
	    {
	    SetActive();
	    }
	    
// ---------------------------------------------------------------------------
// CancelRequest()
// Cancel an outstanding request.
// ---------------------------------------------------------------------------
//    	
void CBTUIActive::CancelRequest()
	    {
	    Cancel();
	    }
	    
// ---------------------------------------------------------------------------
// RequestStatus()
// Get a reference to the active object request status.
// ---------------------------------------------------------------------------
//    
TRequestStatus& CBTUIActive::RequestStatus()
	    {
	    return iStatus;
	    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
void CBTUIActive::DoCancel()
    {
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
void CBTUIActive::RunL()
    {
    iObserver->RequestCompletedL( this, iRequestId, iStatus.Int() );
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
TInt CBTUIActive::RunError( TInt aError )
    {
    iObserver->HandleError( this, iRequestId, aError );
    return KErrNone;
    }
