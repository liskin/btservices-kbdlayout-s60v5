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
* Description:  ?Description
*
*/



#include <e32base.h>
#include <centralrepository.h>

#include "btengprivatecrkeys.h"
#include "btengsettingsnotify.h"
#include "btengsettings.h"
#include "debug.h"

const TInt KPowerKeyReqId = 20;
const TInt KVisibilityKeyReqId = 21;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSettingsNotify::CBTEngSettingsNotify( MBTEngSettingsObserver* aObserver )
:   iObserver( aObserver )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSettingsNotify::ConstructL()
    {
    TRACE_FUNC_ENTRY
    if( iObserver )
        {
            // Power status:
        iPowerKeyWatcher = CBTEngActive::NewL( *this, KPowerKeyReqId, 
                                                CActive::EPriorityStandard );
        iPowerKeyCenRep = CRepository::NewL( KCRUidBluetoothPowerState );
        iPowerKeyCenRep->NotifyRequest( KBTPowerState, 
                                                iPowerKeyWatcher->RequestStatus() );
        iPowerKeyWatcher->GoActive();
            // Visibility mode:
        iVisiKeyWatcher = CBTEngActive::NewL( *this, KVisibilityKeyReqId, 
                                               CActive::EPriorityStandard );
        iVisiKeyCenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
        iVisiKeyCenRep->NotifyRequest( KBTDiscoverable, 
                                        iVisiKeyWatcher->RequestStatus() );
        iVisiKeyWatcher->GoActive();
        }
    
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSettingsNotify* CBTEngSettingsNotify::NewL( MBTEngSettingsObserver* aObserver )
    {
    CBTEngSettingsNotify* self = new( ELeave ) CBTEngSettingsNotify( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSettingsNotify::~CBTEngSettingsNotify()
    {
    delete iPowerKeyWatcher;
    delete iPowerKeyCenRep;
    delete iVisiKeyWatcher;
    delete iVisiKeyCenRep;
    iBTeng.Close();
    }

	
// ---------------------------------------------------------------------------
// Toggles Bluetooth power based on the existence of a session.
// ---------------------------------------------------------------------------
//
TInt CBTEngSettingsNotify::TogglePowerTemporarily()
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    if( !iBTeng.Handle() )
        {
        err = iBTeng.Connect();
        if( !err )
            {
            err = iBTeng.SetPowerState( EBTPowerOn, ETrue );
            }
        }
    else
        {
        iBTeng.Close();
        }
    return err;
    }
	
	
// ---------------------------------------------------------------------------
// From class MBTEngCenRepNotify.
// Handles notification of a setting change, and informs our observer.
// ---------------------------------------------------------------------------
//
void CBTEngSettingsNotify::RequestCompletedL( CBTEngActive* aActive, 
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "Id(%d), status(%d)" ), aActive->RequestId(), aStatus ) )

    if( aActive->RequestId() == KPowerKeyReqId && aStatus == KBTPowerState )
        {
        iPowerKeyCenRep->NotifyRequest( KBTPowerState, 
                                         iPowerKeyWatcher->RequestStatus() );
        iPowerKeyWatcher->GoActive();
        TInt value = 0;
        TInt err = iPowerKeyCenRep->Get( KBTPowerState, value );
        if( !err )
            {
            iObserver->PowerStateChanged( (TBTPowerStateValue) value );
            }
        }
    else if( aActive->RequestId() == KVisibilityKeyReqId && aStatus == KBTDiscoverable )
        {
        iVisiKeyCenRep->NotifyRequest( KBTDiscoverable, 
                                        iVisiKeyWatcher->RequestStatus() );
        iVisiKeyWatcher->GoActive();
        TInt value = 0;
        TInt err = iVisiKeyCenRep->Get( KBTDiscoverable, value );
        if( !err )
            {
            iObserver->VisibilityModeChanged( (TBTVisibilityMode) value );
            }
        }
    else
        {
            // Coudl be a repository-wide reset (KInvalidNotificationId),
            // or an invalid key ID. Anyway we know the ID of the active
            // object, so we can just reset the watcher.
        HandleError( aActive, aStatus );
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles cancelation of an outstanding request
// ---------------------------------------------------------------------------
//
void CBTEngSettingsNotify::CancelRequest( TInt aRequestId )
    {
    TRACE_FUNC_ARG( ( _L( "reqID %d" ), aRequestId ) )

    if( aRequestId == KPowerKeyReqId )
        {
        iPowerKeyCenRep->NotifyCancel( KBTPowerState );
        }
    else if( aRequestId == KVisibilityKeyReqId )
        {
        iVisiKeyCenRep->NotifyCancel( KBTDiscoverable );
        }
    TRACE_FUNC_EXIT 
    }

// ---------------------------------------------------------------------------
// From class MBTEngCenRepNotify.
// Handles error situation by just re-ordering notification from CenRep.
// ---------------------------------------------------------------------------
//
void CBTEngSettingsNotify::HandleError( CBTEngActive* aActive,
    TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "Id(%d), status(%d)" ), aActive->RequestId(), aError ) )
    (void) aError;
    if( aActive->RequestId() == KPowerKeyReqId )
        {
        delete iPowerKeyCenRep;
        iPowerKeyCenRep = NULL;
        TRAPD( err, iPowerKeyCenRep = 
                            CRepository::NewL( KCRUidBluetoothPowerState ) );
        if( !err )
            {
            iPowerKeyCenRep->NotifyRequest( KBTPowerState, 
                                             aActive->RequestStatus() );
            aActive->GoActive();
            }
        }
    else if( aActive->RequestId() == KVisibilityKeyReqId )
        {
        delete iVisiKeyCenRep;
        iVisiKeyCenRep = NULL;
        TRAPD( err, iVisiKeyCenRep = 
                        CRepository::NewL( KCRUidBTEngPrivateSettings ) );
        if( !err )
            {
            iVisiKeyCenRep->NotifyRequest( KBTDiscoverable, 
                                            aActive->RequestStatus() );
            aActive->GoActive();
            }
        }
    }
