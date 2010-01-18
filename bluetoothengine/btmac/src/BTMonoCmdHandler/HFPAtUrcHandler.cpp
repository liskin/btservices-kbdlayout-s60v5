/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AT command URC handler
*
*/


#include <atext.h>
#include "HfpAtCmdHandler.h"
#include "HFPAtUrcHandler.h"
#include "debug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CHFPAtUrcHandler* CHFPAtUrcHandler::NewL(
    RATExt* aAtCmdExt,
    MATExtObserver& aObserver )
    {
    CHFPAtUrcHandler* self = NewLC( aAtCmdExt, aObserver );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CHFPAtUrcHandler* CHFPAtUrcHandler::NewLC(
    RATExt* aAtCmdExt,
    MATExtObserver& aObserver )
    {
    CHFPAtUrcHandler* self = new (ELeave) CHFPAtUrcHandler( aAtCmdExt,
                                                            aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CHFPAtUrcHandler::~CHFPAtUrcHandler()
    {
    TRACE_INFO( _L("CHFPAtUrcHandler::~CHFPAtUrcHandler()") );
    ResetData();
    TRACE_INFO( _L("CHFPAtUrcHandler::~CHFPAtUrcHandler() complete") );
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CHFPAtUrcHandler::ResetData()
    {
    TRACE_INFO( _L("CHFPAtUrcHandler::ResetData()") );
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Don't close iAtCmdExt here (it is done in CHFPAtCmdHandler)!
    // Internal
    Initialize();
    TRACE_INFO( _L("CHFPAtUrcHandler::ResetData() complete") );
    }

// ---------------------------------------------------------------------------
// Starts waiting for an incoming URC message
// ---------------------------------------------------------------------------
//
TInt CHFPAtUrcHandler::IssueRequest()
    {
    TRACE_INFO( _L("CHFPAtUrcHandler::IssueRequest()") );
    if ( iUrcHandleState != EHFPStateIdle )
        {
        TRACE_INFO( _L("CHFPAtUrcHandler::IssueRequest() (not ready) complete") );
        return KErrNotReady;
        }
    iStatus = KRequestPending;
    iAtCmdExt->ReceiveUnsolicitedResult( iStatus, iRecvBuffer, iOwnerUidPckg );
    SetActive();
    iUrcHandleState = EHFPStateAtUrcHandling;
    // Next mark ownership
    if ( !iStarted )
        {
        TInt retTemp = iAtCmdExt->MarkUrcHandlingOwnership( iOwnerUid );
        if ( retTemp != KErrNone )
            {
            TRACE_INFO( _L("CHFPAtUrcHandler::IssueRequest() (not owned) complete") );
            return KErrGeneral;
            }
        }
    iStarted = ETrue;
    TRACE_INFO( _L("CHFPAtUrcHandler::IssueRequest() complete") );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops waiting for an incoming URC message
// ---------------------------------------------------------------------------
//
TInt CHFPAtUrcHandler::Stop()
    {
    TRACE_INFO( _L("CHFPAtUrcHandler::Stop()") );
    if ( iUrcHandleState != EHFPStateAtUrcHandling )
        {
        TRACE_INFO( _L("CHFPAtUrcHandler::Stop() (not ready) complete" ) );
        return KErrNotReady;
        }
    iAtCmdExt->CancelReceiveUnsolicitedResult( iOwnerUid );
    Cancel();
    iUrcHandleState = EHFPStateIdle;
    TRACE_INFO( _L("CHFPAtUrcHandler::Stop() complete") );
    // Note: Don't mark iStarted to EFalse here as it is used to get the UID
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// UID of the owning plugin
// ---------------------------------------------------------------------------
//
TUid CHFPAtUrcHandler::OwnerUid()
    {
    return iOwnerUid;
    }

// ---------------------------------------------------------------------------
// CHFPAtUrcHandler::CHFPAtUrcHandler
// ---------------------------------------------------------------------------
//
CHFPAtUrcHandler::CHFPAtUrcHandler( RATExt* aAtCmdExt, MATExtObserver& aObserver ) :
    CActive( EPriorityHigh ),
    iAtCmdExt( aAtCmdExt ),
    iObserver( aObserver ),
    iOwnerUidPckg( iOwnerUid )

    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CHFPAtUrcHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CHFPAtUrcHandler::ConstructL() 
    {
    TRACE_INFO( _L("CHFPAtUrcHandler::ConstructL()") );
    if ( !iAtCmdExt )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    TRACE_INFO( _L("CHFPAtUrcHandler::ConstructL() complete") );
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CHFPAtUrcHandler::Initialize()
    {
    iUrcHandleState = EHFPStateIdle;
    iOwnerUid = TUid::Null();
    iStarted = EFalse;
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when URC command received
// ---------------------------------------------------------------------------
//
void CHFPAtUrcHandler::RunL()
    {
    TRACE_INFO( _L("CHFPAtUrcHandler::RunL()") );
    iUrcHandleState = EHFPStateIdle;
    TInt err = iStatus.Int();

    // Send received URC message
    if (err == KErrNone)
        {
        if ( iRecvBuffer.Length() == 0 )
            {
            TRACE_INFO( _L("CHFPAtUrcHandler::RunL() (empty buffer) complete") );
            iObserver.UnsolicitedResultFromATExtL(err, KNullDesC8);
            }
        iObserver.UnsolicitedResultFromATExtL(err, iRecvBuffer);
        
        IssueRequest();
        }
    else
        {
        //TRACE_INFO( _L("CHFPAtUrcHandler::RunL() (ERROR) complete (%d)"), err));
        iObserver.UnsolicitedResultFromATExtL(err, KNullDesC8);
        }

    TRACE_INFO( _L("CHFPAtUrcHandler::RunL() complete") );
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CHFPAtUrcHandler::DoCancel()
    {
    }
