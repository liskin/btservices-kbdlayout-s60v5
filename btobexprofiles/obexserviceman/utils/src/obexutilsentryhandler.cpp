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
* Description:  Handles the global progress dialog 
*
*/




#include <msvstore.h>
#include <mmsvattachmentmanager.h>

#include "obexutilsentryhandler.h"
#include "obexutilsdebug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CObexutilsEntryhandler()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler::CObexutilsEntryhandler(): CActive ( EPriorityNormal )
    {
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// NewL()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler* CObexutilsEntryhandler::NewL()
    {
    CObexutilsEntryhandler* self = CObexutilsEntryhandler::NewLC();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler* CObexutilsEntryhandler::NewLC()
    {
    CObexutilsEntryhandler* self = new( ELeave ) CObexutilsEntryhandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// AddLinkAttachment()
// ---------------------------------------------------------------------------
//
TInt CObexutilsEntryhandler::AddEntryAttachment(
    const TDesC &aFilePath, 
    CMsvAttachment* anAttachInfo, 
    CMsvStore* aStore)
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::AddEntryAttachment()"));       
    
    iStatus = KRequestPending;
  
    TRAPD(error, aStore->AttachmentManagerL().AddLinkedAttachmentL(aFilePath,anAttachInfo, iStatus););
    
    if (error != KErrNone )
        {        
        //Complete request
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, error);
        }
    
    SetActive();
    iSyncWaiter.Start();
    
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::AddEntryAttachment() Done"));
    return iStatus.Int();
    }


// ---------------------------------------------------------------------------
// From class CActive.
// RunL()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::RunL()
    {
    if ( iSyncWaiter.IsStarted() )
        {
        iSyncWaiter.AsyncStop();
        }
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::RunL() Done"));           
    }

// ---------------------------------------------------------------------------
// ~CObexutilslinkhandler()
// ---------------------------------------------------------------------------
//
CObexutilsEntryhandler::~CObexutilsEntryhandler()
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::Destructor"));     
    Cancel();    
    }

// ---------------------------------------------------------------------------
// From class CActive.
// DoCancel()
// ---------------------------------------------------------------------------
//
void CObexutilsEntryhandler::DoCancel()
    {
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoCancel()"));           
    if ( iSyncWaiter.IsStarted() )
        {
        iSyncWaiter.AsyncStop();
        }
    FLOG(_L("[OBEXUTILS]\t CObexutilsEntryhandler::DoCancel() done"));               
    }
