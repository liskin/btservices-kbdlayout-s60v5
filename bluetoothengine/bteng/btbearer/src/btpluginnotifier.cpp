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
* Description:  BT Bearer plugin notification implementation file
*
*/



#include <locodbearerpluginobserver.h>
#include <btengsettings.h>
#include <CoreApplicationUIsSDKCRKeys.h>
#include "btpluginnotifier.h"
#include "debug.h"
#include <btfeaturescfg.h>

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTPluginNotifier::CBTPluginNotifier( MLocodBearerPluginObserver& aParam, 
    TUid aUid, TCenRepKeyType aKeyType, TUint32 aId )
:   CActive( EPriorityStandard ),
    iUid( aUid ),
    iId( aId ),
    iKeyType( aKeyType ),
    iHandler( aParam )
    {
    CActiveScheduler::Add( this );
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTPluginNotifier::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iSession = CRepository::NewL( iUid );
    TInt power = EBTPowerOff;
    iSession->Get( iId, power );
    
    TInt offlineModeOff = ECoreAppUIsNetworkConnectionAllowed;
	CRepository* repository = CRepository::NewL( KCRUidCoreApplicationUIs );
	repository->Get(KCoreAppUIsNetworkConnectionAllowed, offlineModeOff );
	delete repository;
	
	CBTEngSettings* settings = CBTEngSettings::NewL();
	
	BluetoothFeatures::TEnterpriseEnablementMode mode = BluetoothFeatures::EnterpriseEnablementL();
	TRACE_INFO( ( _L( "mode = %d" ), mode) )
    if( power == EBTPowerOn && offlineModeOff == ECoreAppUIsNetworkConnectionAllowed && mode != BluetoothFeatures::EDisabled )
        {
             // BT was on before boot, turn it on again
        TRACE_INFO( ( _L( "Turning BT on" ) ) )
        TInt err = settings->SetPowerState( EBTPowerOn );
        TRACE_INFO( ( _L( "SetPowerState returned %d" ), err ) )
        if( !err )
            {
            iHandler.NotifyBearerStatus( ELocodBearerBT, power );
            }
        }
    else
        {
        TRACE_INFO( ( _L( "Turning BT off" ) ) )
        (void) settings->SetPowerState( EBTPowerOff );	// Result is not important here
        }
    delete settings;
	if ( mode != BluetoothFeatures::EDisabled ) // only subscribe if there's any point (NB changing Enterprise Disabling mode requires a reboot)
		SubscribeL();
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTPluginNotifier* CBTPluginNotifier::NewL( MLocodBearerPluginObserver& aParam, 
    TUid aUid, TCenRepKeyType aKeyType, TUint32 aId )
    {
    TRACE_FUNC_ENTRY
    CBTPluginNotifier* self = new( ELeave ) CBTPluginNotifier( aParam, aUid, 
                                                                aKeyType, aId );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTPluginNotifier::~CBTPluginNotifier()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    delete iSession;
    }


// ---------------------------------------------------------------------------
// CBTPluginNotifier::SubscribeL
// ---------------------------------------------------------------------------
//
void  CBTPluginNotifier::SubscribeL()
    {
    TRACE_FUNC_ENTRY
    if( IsActive() )
        {
        return;
        }
    User::LeaveIfError( iSession->NotifyRequest( iId, iStatus ) );
    SetActive();
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when our subscription 
// to the setting has been cancelled.
// ---------------------------------------------------------------------------
//
void CBTPluginNotifier::DoCancel()
    {
    iSession->NotifyCancel( iId );
    }


// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when the status has changed.
// ---------------------------------------------------------------------------
//
void CBTPluginNotifier::RunL()
    {
    TUint32 status = static_cast<TUint32>( iStatus.Int() );
    TRACE_FUNC_ARG( ( _L( "RunL1 iStatus: %d]" ), status ) )

    if( status != iId && status != NCentralRepositoryConstants::KInvalidNotificationId )
        {
        return;
        }
    SubscribeL();
    switch( iKeyType )
        {
        case EKeyInt:
            {
            TRACE_INFO( ( _L( "[CBTPluginNotifier::RunL2 %d]" ), status ) )
            TInt newValue = 1;
            iSession->Get( iId, newValue );
            iHandler.NotifyBearerStatus( ELocodBearerBT, newValue );
            }
            break;
        default:
            {
            TRACE_INFO( ( _L( "[CBTPluginNotifier::RunL3 %d]" ), status ) )
            }
            break;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Called by the active scheduler when RunL leaves.
// ---------------------------------------------------------------------------
//
int CBTPluginNotifier::RunError(TInt aError)
    {
    TRACE_FUNC_ARG( ( _L( "RunError aError: %d]" ), aError ) )
    (void) aError;
    return KErrNone;
    }



