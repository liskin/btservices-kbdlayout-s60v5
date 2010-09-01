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
* Description:  Listen to CenRep and PubSub Keys
*
*/


#include <e32base.h>
#include <btserversdkcrkeys.h>
#include <btengprivatecrkeys.h>
#include <bt_subscribe.h>
#include "BTUIKeyWatcher.h"
#include "BTUIMainView.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTUIKeyWatcher* CBTUIKeyWatcher::NewL( TAny* aCaller, TInt aServiceID  )
    {
    CBTUIKeyWatcher* self = new( ELeave ) CBTUIKeyWatcher( aCaller );
    CleanupStack::PushL( self );
    self->ConstructL( aServiceID );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTUIKeyWatcher::CBTUIKeyWatcher( TAny* aCaller )
:   iCaller( aCaller )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTUIKeyWatcher::ConstructL( TInt aServiceID )
    {
    TRACE_FUNC_ENTRY
       
    if( aServiceID & KBTUIPHYCountWatcher )
    	{
	    TInt err = iPHYCountKey.Attach( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothPHYCount );
	    if( !err )
	        {
	        iPHYCountWatcher = CBTUIActive::NewL( this, KBTUIPHYCountWatcher, CActive::EPriorityStandard );
	        if( iPHYCountWatcher != NULL )
	        	{
	        	iPHYCountKey.Subscribe( iPHYCountWatcher->RequestStatus() );
	        	iPHYCountWatcher->GoActive();
	        	}
	        else
	        	{
	        	TRACE_INFO(_L("new iPHYCountWatcher failed"));
	        	}
	        }
	    else
	    	{
	    	TRACE_INFO((_L("Attach failed %d"), err))
	    	}	
    	}
    
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTUIKeyWatcher::~CBTUIKeyWatcher()
    {
    TRACE_FUNC_ENTRY
   
    if( iPHYCountKey.Handle() )
        {
        iPHYCountKey.Cancel();
        }
    if( iPHYCountWatcher )
    	{
    	delete iPHYCountWatcher;
    	iPHYCountWatcher = NULL;
    	}
    iPHYCountKey.Close();
    
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Processes a changed key value.
// ---------------------------------------------------------------------------
//
void CBTUIKeyWatcher::RequestCompletedL( CBTUIActive* aActive, TInt aId, TInt /*aStatus*/ )
    {
    switch( aId )
        {
        case KBTUIPHYCountWatcher:
            iPHYCountKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            static_cast<CBTUIMainView*>(iCaller)->UpdateParentView();
            break;
        default:
        	TRACE_INFO((_L("wrong key notification! id=%d"), aId))
            break;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles a leave in RunL/RequestCompletedL by checking that all 
// the subscriptions are active.
// ---------------------------------------------------------------------------
//
void CBTUIKeyWatcher::HandleError( CBTUIActive* aActive, TInt aId, TInt aError )
    {
    TRACE_INFO((_L("status = %d"), aError))
    (void) aError;
    if( !aActive->IsActive() )
        {
        switch( aId )
            {
            case KBTUIPHYCountWatcher:
                iPHYCountKey.Subscribe( aActive->RequestStatus() );
                aActive->GoActive();
                break;
            default:         
            	TRACE_INFO((_L("wrong key notification! id=%d"), aId))
                break;
            }
        }
    }
