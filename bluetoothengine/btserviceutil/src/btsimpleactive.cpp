/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <btservices/btsimpleactive.h>

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBtSimpleActive::CBtSimpleActive(  MBtSimpleActiveObserver& aObserver, TInt aId, 
    TInt aPriority )
:   CActive( aPriority ),
    iRequestId( aId ),
    iObserver( aObserver )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBtSimpleActive* CBtSimpleActive::NewL( MBtSimpleActiveObserver& aObserver, 
    TInt aId, TInt aPriority )
    {
    CBtSimpleActive* self = new( ELeave ) CBtSimpleActive( aObserver, aId, aPriority );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CBtSimpleActive::~CBtSimpleActive()
    {
    Cancel();
    }

// -----------------------------------------------------------------------------
// Get the identifier of this instance.
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CBtSimpleActive::RequestId()
    {
    return iRequestId;
    }

// -----------------------------------------------------------------------------
// Set the identifier of this instance.
// -----------------------------------------------------------------------------
//
EXPORT_C void CBtSimpleActive::SetRequestId( TInt aId )
    {
    iRequestId = aId;
    }

// -----------------------------------------------------------------------------
// Activate the active object.
// -----------------------------------------------------------------------------
//
EXPORT_C void CBtSimpleActive::GoActive()
    {
    SetActive();
    }

// -----------------------------------------------------------------------------
// Get a reference to the active object request status.
// -----------------------------------------------------------------------------
//
EXPORT_C TRequestStatus& CBtSimpleActive::RequestStatus()
    {
    return iStatus;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been cancelled.
// ---------------------------------------------------------------------------
//
void CBtSimpleActive::DoCancel()
    {
    iObserver.CancelRequest( iRequestId );
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the request has been completed.
// ---------------------------------------------------------------------------
//
void CBtSimpleActive::RunL()
    {
    iObserver.RequestCompletedL( this, iStatus.Int() );
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when an error in RunL has occurred.
// ---------------------------------------------------------------------------
//
TInt CBtSimpleActive::RunError( TInt aError )
    {
    iObserver.HandleError( this, aError );
    return KErrNone;
    }
