/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/



// INCLUDE FILES
#include <e32property.h>
#include "bthidpskeywatcher.h"
#include "bthidPsKey.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::CBTMouseCursorStatusObserver()
//----------------------------------------------------------
//
CBTMouseCursorStatusObserver::CBTMouseCursorStatusObserver(): CActive(CActive::EPriorityStandard) 
    {                                
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::NewL()
//----------------------------------------------------------
// 
CBTMouseCursorStatusObserver* CBTMouseCursorStatusObserver::NewL() 
    {
    CBTMouseCursorStatusObserver* self = new(ELeave) CBTMouseCursorStatusObserver();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
 
// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::ConstructL()
//----------------------------------------------------------
//   
void CBTMouseCursorStatusObserver::ConstructL()
    {
    
    User::LeaveIfError(iProperty.Attach( KPSUidBthidSrv, KBTMouseCursorState));
    }

// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::~CBTMouseCursorStatusObserver
//----------------------------------------------------------
//
CBTMouseCursorStatusObserver::~CBTMouseCursorStatusObserver()
    {
    Cancel();
    iProperty.Close();
    }
    
// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::StartObservingL()
//----------------------------------------------------------
//
void CBTMouseCursorStatusObserver::SubscribeMouseCursorStatusL(MMouseCursorStatusObserver* aObserver)
    {    
    
    ASSERT(aObserver);
    
    iObserver = aObserver;
    
    iProperty.Subscribe(iStatus);
    SetActive();
    }
    

// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::DoCancel()
// ---------------------------------------------------------
//
void CBTMouseCursorStatusObserver::DoCancel()
    {       
    iProperty.Cancel();
    }

// ---------------------------------------------------------
// CBTMouseCursorStatusObserver::RunL()
// ---------------------------------------------------------
//
void CBTMouseCursorStatusObserver::RunL()
    {
    
    TInt btMouseCursorState = ECursorNotInitialized;
    TInt err = iStatus.Int();
    
	if (err == KErrNotFound)
		{
		// someone deleted the P&S key - this is ok as we on error
		// set the cursor to not initialized.
		err = KErrNone;
		}

    iProperty.Subscribe(iStatus);
    SetActive();
    
    if (err == KErrNone)
        {
        err = iProperty.Get(btMouseCursorState);
        if (err == KErrNotFound)
            {
			// probably no longer present, so set to not initialized.
			btMouseCursorState = ECursorNotInitialized;
			err = KErrNone;
			}
		if(err == KErrNone)
			{
			iObserver->MouseCursorStatusChangedL(btMouseCursorState);
			}
        }
    }
