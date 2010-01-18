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
* Description:  Watcher for BTEng server PubSub and CenRep keys.
*
*/



#include <startupdomainpskeys.h>
#include <coreapplicationuisdomainpskeys.h>
#include <ctsydomainpskeys.h>
#include <centralrepository.h>
#include <bt_subscribe.h>

#include "btengsrvkeywatcher.h"
#include "btengserver.h"
#include "btengsrvpluginmgr.h"
#include "btengprivatepskeys.h"
#include "btengprivatecrkeys.h"
#include "debug.h"

/**  Identification for active object */
const TInt KBTEngDutWatcher = 15;
/**  Identification for active object */
const TInt KBTEngLockWatcher = 16;
/**  Identification for active object */
const TInt KBTEngSysWatcher = 17;
/**  Identification for active object */
const TInt KBTEngPHYCountWatcher = 18;
/**  Identification for active object */
const TInt KBTEngBtConnectionWatcher = 19;
/**  Identification for active object */
const TInt KBTEngScanningWatcher = 20;
/**  Identification for active object */
const TInt KBTEngEmergencyWatcher = 21;
/**  Identification for active object */
const TInt KBTEngSapWatcher = 22;
/**  Identification for active object */
const TInt KBTEngAddrWatcher = 23;
/**  Identification for active object */
const TInt KBTEngRegistryWatcher = 24;
/**  Identification for active object */
const TInt KBTEngSspDebugWatcher = 25;
/**  Buffer size for BT device address as stored in CenRep */
const TInt KCenRepAddrBufSize = 2 * KBTDevAddrSize;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSrvKeyWatcher::CBTEngSrvKeyWatcher( CBTEngServer* aServer )
:   iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSrvKeyWatcher::ConstructL()
    {
    TRACE_FUNC_ENTRY
    TInt err = iDutModeKey.Attach( KPSUidBluetoothTestingMode, KBTDutEnabled );
    if( !err )
        {
        iDutModeWatcher = CBTEngActive::NewL( *this, KBTEngDutWatcher, 
                                               CActive::EPriorityStandard );
        iDutModeKey.Subscribe( iDutModeWatcher->RequestStatus() );
        iDutModeWatcher->GoActive();
        }
#ifdef RD_REMOTELOCK
    err = iPhoneLockKey.Attach( KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus );
    if( !err )
        {
        iPhoneLockWatcher = CBTEngActive::NewL( *this, KBTEngLockWatcher, 
                                                 CActive::EPriorityStandard );
        iPhoneLockKey.Subscribe( iPhoneLockWatcher->RequestStatus() );
        iPhoneLockWatcher->GoActive();
        }
#endif  //RD_REMOTELOCK

    err = iSystemStateKey.Attach( KPSUidStartup, KPSGlobalSystemState );
    if( !err )
        {
        iSystemStateWatcher = CBTEngActive::NewL( *this, KBTEngSysWatcher, 
                                                   CActive::EPriorityStandard );
        iSystemStateKey.Subscribe( iSystemStateWatcher->RequestStatus() );
        iSystemStateWatcher->GoActive();
        }
        
    err = iPHYCountKey.Attach( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothPHYCount );
    if( !err )
        {
        iPHYCountWatcher = CBTEngActive::NewL( *this, KBTEngPHYCountWatcher, 
                                               CActive::EPriorityStandard );
        iPHYCountKey.Subscribe( iPHYCountWatcher->RequestStatus() );
        iPHYCountWatcher->GoActive();
        }
        
    err = iBtConnectionKey.Attach( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothConnecting );
    if( !err )
        {
        iBtConnectionWatcher = CBTEngActive::NewL( *this, KBTEngBtConnectionWatcher, 
                                               CActive::EPriorityStandard );
        iBtConnectionKey.Subscribe( iBtConnectionWatcher->RequestStatus() );
        iBtConnectionWatcher->GoActive();
        }

    err = iBtScanningKey.Attach( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothSetScanningStatus );
    if( !err )
        {
        iBtScanningWatcher = CBTEngActive::NewL( *this, KBTEngScanningWatcher, 
                                               CActive::EPriorityStandard );
        iBtScanningKey.Subscribe( iBtScanningWatcher->RequestStatus() );
        iBtScanningWatcher->GoActive();
        }

    err = iEmergencyCallKey.Attach( KPSUidCtsyEmergencyCallInfo, KCTSYEmergencyCallInfo );
    if( !err )
        {
        iEmergencyCallWatcher = CBTEngActive::NewL( *this, KBTEngEmergencyWatcher, 
                                                   CActive::EPriorityStandard );
        iEmergencyCallKey.Subscribe( iEmergencyCallWatcher->RequestStatus() );
        iEmergencyCallWatcher->GoActive();
        }

    err = iBtRegistryKey.Attach( KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetRegistryTableChange );
    if( !err )
        {
        iBtRegistryWatcher = CBTEngActive::NewL( *this, KBTEngRegistryWatcher, 
                                                 CActive::EPriorityStandard );
        iBtRegistryKey.Subscribe( iBtRegistryWatcher->RequestStatus() );
        iBtRegistryWatcher->GoActive();
        }

    err = iSspDebugModeKey.Attach( KPSUidBluetoothTestingMode, KBTSspDebugmode );
    if( !err )
        {
        iSspDebugModeWatcher = CBTEngActive::NewL( *this, KBTEngSspDebugWatcher, 
                                                    CActive::EPriorityStandard );
        iSspDebugModeKey.Subscribe( iSspDebugModeWatcher->RequestStatus() );
        iSspDebugModeWatcher->GoActive();
        }

        // Use TRAP here, because it leaves if the key does not exist.
    TRAP( err, iSapKeyCenRep = CRepository::NewL( KCRUidBTEngPrivateSettings ) );
    if( !err )
        {
        iSapModeWatcher = CBTEngActive::NewL( *this, KBTEngSapWatcher, 
                                                     CActive::EPriorityStandard );
        err = iSapKeyCenRep->NotifyRequest( KBTSapEnabled, 
                                             iSapModeWatcher->RequestStatus() );
        }
    if( !err )
        {
        iSapModeWatcher->GoActive();
        }
    err = iBdaddrKey.Attach( KPropertyUidBluetoothCategory, 
                              KPropertyKeyBluetoothGetLocalDeviceAddress );
                              
    if( !err )
        {
        CheckBDAddrL();
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSrvKeyWatcher* CBTEngSrvKeyWatcher::NewL( CBTEngServer* aServer )
    {
    CBTEngSrvKeyWatcher* self = new( ELeave ) CBTEngSrvKeyWatcher( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSrvKeyWatcher::~CBTEngSrvKeyWatcher()
    {
    if( iDutModeKey.Handle() )
        {
        iDutModeKey.Cancel();
        }
    delete iDutModeWatcher;
    iDutModeKey.Close();

#ifdef RD_REMOTELOCK
    if( iPhoneLockKey.Handle() )
        {
        iPhoneLockKey.Cancel();
        }
    delete iPhoneLockWatcher;
    iPhoneLockKey.Close();
#endif  //RD_REMOTELOCK

    if( iSystemStateKey.Handle() )
        {
        iSystemStateKey.Cancel();
        }
    delete iSystemStateWatcher;
    iSystemStateKey.Close();
    
    if( iPHYCountKey.Handle() )
        {
        iPHYCountKey.Cancel();
        }
    delete iPHYCountWatcher;
    iPHYCountKey.Close();
     
    if( iBtConnectionKey.Handle() )
        {
        iBtConnectionKey.Cancel();
        }
    delete iBtConnectionWatcher;
    iBtConnectionKey.Close();
    
    if( iBtScanningKey.Handle() )
        {
        iBtScanningKey.Cancel();
        }
    delete iBtScanningWatcher;
    iBtScanningKey.Close();

    if( iEmergencyCallKey.Handle() )
        {
        iEmergencyCallKey.Cancel();
        }
    delete iEmergencyCallWatcher;
    iEmergencyCallKey.Close();
    
    if( iSspDebugModeKey.Handle() )
        {
        iSspDebugModeKey.Cancel();
        }
    delete iSspDebugModeWatcher;
    iSspDebugModeKey.Close();

    if( iBtRegistryKey.Handle() )
    	{
    	iBtRegistryKey.Cancel();
    	}
    delete iBtRegistryWatcher;
    iBtRegistryKey.Close();

    if( iSapKeyCenRep )
        {
        iSapKeyCenRep->NotifyCancel( KBTSapEnabled );
        }
    delete iSapModeWatcher;
    delete iSapKeyCenRep;

    if( iBdaddrKey.Handle() )
        {
        iBdaddrKey.Cancel();
        }
    delete iBdaddrWatcher;
    iBdaddrKey.Close();
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Processes a changed key value.
// ---------------------------------------------------------------------------
//
void CBTEngSrvKeyWatcher::RequestCompletedL( CBTEngActive* aActive, TInt aId, 
    TInt aStatus )
    {
    TRACE_FUNC_ARG( ( _L( "status %d" ), aStatus ) )
    ASSERT( aStatus != KErrPermissionDenied );
    TInt val = 0;
    switch( aId )
        {
        case KBTEngDutWatcher:
            {
            TRACE_INFO( ( _L( "DUT mode key changed" ) ) )
            iDutModeKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            if( !aStatus && !iDutModeKey.Get( val ) )
                {
                iServer->SetDutMode( val );
                }
            }
            break;
        case KBTEngLockWatcher:
            {
            TRACE_INFO( ( _L( "phone lock key changed" ) ) )
#ifdef RD_REMOTELOCK
            iPhoneLockKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            TInt remoteLockVal = 0;
            remoteLockVal = ERemoteLocked;
            if( !aStatus && !iPhoneLockKey.Get( val ) && val == remoteLockVal )
                {
                iServer->SetPowerStateL( EBTPowerOff, EFalse );
                }
#endif  //RD_REMOTELOCK
            }
            break;
        case KBTEngSysWatcher:
            {
            TRACE_INFO( ( _L( "System state key changed" ) ) )
            iSystemStateKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            if( !aStatus && !iSystemStateKey.Get( val ) && 
                 val == ESwStateShuttingDown )
                {
                iServer->SetVisibilityModeL( EBTVisibilityModeNoScans , 0 );
                iServer->DisconnectAllL();   
                }
            }
            break;   
        case KBTEngPHYCountWatcher:
            {
            TRACE_INFO( ( _L( "PHY count key changed" ) ) )
            iPHYCountKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            iServer->SetUiIndicatorsL();
            }
            break;
        case KBTEngBtConnectionWatcher:
            {
            TRACE_INFO( ( _L( "BT connection key changed" ) ) )
            iBtConnectionKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            iServer->SetUiIndicatorsL();
            }
            break;        
        case KBTEngScanningWatcher:
            {
            TRACE_INFO( ( _L( "BT stack scanning key changed" ) ) )
            iBtScanningKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            if ( !iBtScanningKey.Get( val ) )
                {
                iServer->UpdateVisibilityModeL( val );
                }
            }
            break;
        case KBTEngEmergencyWatcher:
            {
            TRACE_INFO( ( _L( "Emergency call key changed" ) ) )
            iEmergencyCallKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            if( !aStatus && !iEmergencyCallKey.Get( val ) && val )
                {
                // An emergency call initiated -> Close SAP connection if it's active
                iServer->iPluginMgr->DisconnectProfile( EBTProfileSAP );
                }
            }
            break;
        case KBTEngSspDebugWatcher:
            {
            TRACE_INFO( ( _L( "Simple pairing debug mode key changed" ) ) )
            iSspDebugModeKey.Subscribe( aActive->RequestStatus() );
            aActive->GoActive();
            if( !aStatus && !iSspDebugModeKey.Get( val ) )
                {
                iServer->CheckSspDebugModeL( (TBool) val );
                }
            break;
            }
        case KBTEngRegistryWatcher:
        	{
            TRACE_INFO( ( _L( "BT Registry key changed" ) ) )
            TInt myChangedTable;

			iBtRegistryKey.Subscribe( aActive->RequestStatus() );
			aActive->GoActive();

            TInt err = iBtRegistryKey.Get( myChangedTable );
            if( !err && myChangedTable == KRegistryChangeRemoteTable )
            	{
            	TRACE_INFO( ( _L("BT Remote registry key changed") ) )
            	iServer->RemoteRegistryChangeDetected();            	
            	}
        	break;
        	}    
        case KBTEngSapWatcher:
            {
            TRACE_INFO( ( _L( "SAP mode key changed" ) ) )
            iSapKeyCenRep->NotifyRequest( KBTSapEnabled, aActive->RequestStatus() );
            aActive->GoActive();
            iSapKeyCenRep->Get( KBTSapEnabled, val );
            
            TBTPowerStateValue powerState = EBTPowerOff;
            User::LeaveIfError( iServer->GetHwPowerState( (TBTPowerStateValue&) powerState ) );
            if( aStatus >= 0 && powerState )
                {
                // Relevant only if BT is on
                if( val == EBTSapEnabled )
                    {
                    iServer->iPluginMgr->LoadBTSapPluginL();
                    }
                else
                    {
                    iServer->iPluginMgr->UnloadBTSapPlugin();
                    }
                }
            }
            break;
        case KBTEngAddrWatcher:
            {
            TBuf8<KBTDevAddrSize> addrPubSub;
			iBdaddrKey.Subscribe( aActive->RequestStatus() );
			aActive->GoActive();

            if( !aStatus && !iBdaddrKey.Get( addrPubSub ) && 
                 addrPubSub.Length() == KBTDevAddrSize )
                {
				iBdaddrKey.Cancel();

                TBTDevAddr addr( addrPubSub );
                TBuf<KCenRepAddrBufSize> addrCenRep;
                addr.GetReadable( addrCenRep );
                    // Write the key to CenRep and stop listening.
                CRepository* cenRep = CRepository::NewL( 
                                            KCRUidBluetoothLocalDeviceAddress );
                cenRep->Set( KBTLocalDeviceAddress, addrCenRep );
                if ( iBdaddrWatcher )
                    {
                    delete iBdaddrWatcher;
                    iBdaddrWatcher = NULL;
                    }
                iBdaddrKey.Close();
                }
            }
            break;
        default:
            {
            TRACE_INFO( ( _L( "[BTENG]\t wrong key notification! id=%d" ), aId ) )
            }
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
void CBTEngSrvKeyWatcher::HandleError( CBTEngActive* aActive, TInt aId, 
    TInt aError )
    {
    TRACE_FUNC_ARG( ( _L( "status %d" ), aError ) )
    (void) aError;
    if( !aActive->IsActive() )
        {
        switch( aId )
            {
            case KBTEngDutWatcher:
                {
                iDutModeKey.Subscribe( aActive->RequestStatus() );
                }
                break;
            case KBTEngLockWatcher:
                {
#ifdef RD_REMOTELOCK
                iPhoneLockKey.Subscribe( aActive->RequestStatus() );
#else
                return; // we don't want to go active without subscribing
#endif  //RD_REMOTELOCK
                }
                break;
            case KBTEngSysWatcher:
                {
                iSystemStateKey.Subscribe( aActive->RequestStatus() );
                }
                break;
            case KBTEngSspDebugWatcher:
                {
                iSspDebugModeKey.Subscribe( aActive->RequestStatus() );
                }
                break;
            case KBTEngSapWatcher:
                {
                iSapKeyCenRep->NotifyRequest( KBTSapEnabled, aActive->RequestStatus() );
                }
                break;
            default:
                {
                TRACE_INFO( ( _L( "[BTENG]\t wrong key notification! id=%d" ), aId ) )
                }
                return; // we don't want to go active without subscribing
            }
        aActive->GoActive();
        }
    }


// ---------------------------------------------------------------------------
// Checks if we have already stored the BD_ADDR in CenRep; if not, we get it
// from PubSub. If not available from PubSub yet, we subscribe to the key.
// ---------------------------------------------------------------------------
//
void CBTEngSrvKeyWatcher::CheckBDAddrL()
    {
    TBuf<KCenRepAddrBufSize> addrCenRep;
    CRepository* cenRep = NULL;
        // Use TRAP to catch a leave if the CenRep key does not exist.
    TRAPD( err, cenRep = CRepository::NewL( KCRUidBluetoothLocalDeviceAddress ) );
    CleanupStack::PushL( cenRep );
    if( !err )
        {
        err = cenRep->Get( KBTLocalDeviceAddress, addrCenRep );
        }
    if( err || addrCenRep.Length() != KCenRepAddrBufSize )
        {
        TBuf8<KBTDevAddrSize> addrPubSub;
        err = iBdaddrKey.Get( addrPubSub );
        if( !err && addrPubSub.Length() == KBTDevAddrSize )
            {
            TBTDevAddr addr( addrPubSub );  // For easy conversion
            addr.GetReadable( addrCenRep );
            cenRep->Set( KBTLocalDeviceAddress, addrCenRep );
            iBdaddrKey.Close();
            }
        else
            {
            iBdaddrWatcher = CBTEngActive::NewL( *this, KBTEngAddrWatcher, 
                                                        CActive::EPriorityStandard );
            iBdaddrKey.Subscribe( iBdaddrWatcher->RequestStatus() );
            iBdaddrWatcher->GoActive();
            }
        }
    else
        {
        iBdaddrKey.Close();
        }
    CleanupStack::PopAndDestroy( cenRep );
    }
