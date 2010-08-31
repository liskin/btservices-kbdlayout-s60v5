/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine API for managing device settings.
*
*/



#include <centralrepository.h>
#include <featmgr.h>
#include <utf.h>

#include "btengsettings.h"
#include "btengprivatecrkeys.h"
#include "btengsettingsnotify.h"
#include "btengclient.h"
#include "debug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSettings::CBTEngSettings( MBTEngSettingsObserver* aObserver )
:   iObserver( aObserver )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSettings::ConstructL()
    {
    TRACE_FUNC_ENTRY
        // Check if BT is supported at all
    FeatureManager::InitializeLibL();
    TBool btSupported = FeatureManager::FeatureSupported( KFeatureIdBt );
    FeatureManager::UnInitializeLib();
    if( !btSupported )
        {
        TRACE_INFO( ( _L( "[BTENGSETTINGS]\t ConstructL: BT not supported!!" ) ) )
        User::Leave( KErrNotSupported );
        }
    if( iObserver )
        {
        iSettingsWatcher = CBTEngSettingsNotify::NewL( iObserver );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngSettings* CBTEngSettings::NewL( MBTEngSettingsObserver* aObserver )
    {
    CBTEngSettings* self = CBTEngSettings::NewLC( aObserver );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CBTEngSettings* CBTEngSettings::NewLC( MBTEngSettingsObserver* aObserver )
    {
    CBTEngSettings* self = new( ELeave ) CBTEngSettings( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSettings::~CBTEngSettings()
    {
    TRACE_FUNC_ENTRY
    delete iSettingsWatcher;
    }


// ---------------------------------------------------------------------------
// Gets the current BT hardware power state 
// (on or off) from central repository.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::GetPowerState( TBTPowerStateValue& aState )
    {
    TRACE_FUNC_ENTRY
    aState = EBTPowerOff;
    TRAPD( err, GetCenRepKeyL( KCRUidBluetoothPowerState, KBTPowerState, 
                               (TInt&) aState, EBTPowerOff, EBTPowerOn ) );
    return err;
    }


// ---------------------------------------------------------------------------
// Sets the Bluetooth hardware power state.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::SetPowerState( TBTPowerStateValue aState )
    {
    TRACE_FUNC_ENTRY
    if( aState != EBTPowerOff && aState != EBTPowerOn )
        {
        return KErrArgument;
        }
    RBTEng bteng;
    TInt err = bteng.Connect();
    if( !err )
        {
        err = bteng.SetPowerState( aState, EFalse );
        bteng.Close();
        }
    TRACE_FUNC_EXIT
    return err;
    }


// ---------------------------------------------------------------------------
// Gets the current Bluetooth hardware visibility mode 
// (visible or hidden) from central repository.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::GetVisibilityMode( TBTVisibilityMode& aMode )
    {
    TRACE_FUNC_ENTRY
    aMode = EBTVisibilityModeGeneral;
    TRAPD( err, GetCenRepKeyL( KCRUidBTEngPrivateSettings, KBTDiscoverable, 
                               (TInt&) aMode, EBTVisibilityModeHidden, 
                               EBTVisibilityModeGeneral, 
                               EBTVisibilityModeTemporary, 
                               EBTVisibilityModeTemporary ) );
    return err;
    }


// ---------------------------------------------------------------------------
// Sets the Bluetooth hardware visibility mode. The visible mode can also 
// be temporary visible, specified by the aTime parameter (time in seconds).
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::SetVisibilityMode( TBTVisibilityMode aMode, TInt aTime )
    {
    TRACE_FUNC_ENTRY
    if( aMode != EBTVisibilityModeHidden && aMode != EBTVisibilityModeGeneral && 
        aMode != EBTVisibilityModeTemporary )
        {
        return KErrArgument;
        }
    RBTEng bteng;
    TInt err = bteng.Connect();
    if( !err )
        {
        err = bteng.SetVisibilityMode( aMode, aTime );
        bteng.Close();
        }

    TRACE_FUNC_EXIT
    return err;
    }


// ---------------------------------------------------------------------------
// Gets the Bluetooth friendly name the Bluetooth stack.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::GetLocalName( TDes& aName )
    {
    TRACE_FUNC_ENTRY
    aName.Zero();
    TBool nameStatus = EFalse;
    TBuf<KMaxBluetoothNameLen> tmpName;
    
    TInt err = GetLocalNameModified( nameStatus );
    
    if( err || nameStatus == EBTLocalNameDefault )
        {
        return err;
        }

    err = RProperty::Get( KPropertyUidBluetoothCategory, 
                           KPropertyKeyBluetoothGetDeviceName, aName );
    if( !err )
        {
        err = RProperty::Get( KPropertyUidBluetoothCategory, 
                               KPropertyKeyBluetoothSetDeviceName, tmpName );
        if ( tmpName.Compare( aName ) )
            {
                // The name has not yet been updated. Use the new one.
            aName.Copy( tmpName );
            }
        }
        
    if( err || !aName.Length() )
        {
            // An error occured, try still registry
        aName.Zero();
        err = HandleBTRegistryNameSetting( const_cast<TDes&>( aName ) );
        TRACE_INFO( ( _L( "[BTENGSETTINGS]\t registry set result: %d"), err) );
        }                            
                                     	        	                               
    TRACE_INFO( ( _L( "[BTENGSETTINGS]\t GetLocalName - result %d, name: %S" ),
                        err, &aName ) )
    return err;
    }

// ---------------------------------------------------------------------------
// Sets the Bluetooth friendly name.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::SetLocalName( const TDes& aName )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_ARG( ( _L( "aName lenght: %d" ), aName.Length() ) )
    if( aName.Length() == 0 || aName.Length() > KMaxBluetoothNameLen )
        {
            // Zero-length name means that the name has not been set;
            // such reset should not be done through BTEngSettings.
        return KErrArgument;
        }
        
    TInt err = RProperty::Set( KPropertyUidBluetoothCategory, 
                           KPropertyKeyBluetoothSetDeviceName, aName );
    TRACE_INFO( ( _L( "[BTENGSETTINGS]\t RProperty::Set: %d"), err) );
    
        // Always update the name to registry
    TInt err2 = HandleBTRegistryNameSetting( const_cast<TDes&>( aName ) );

    TBool nameChanged = EFalse;
    if( !err || !err2 )
        {				       	
            // Check if this is the first time name setting.
        err = GetLocalNameModified( nameChanged );
        TRACE_INFO( ( _L( "[BTENGSETTINGS]\t GetLocalNameModified: err %d, namechanged %d"), 
        		err, nameChanged) );               	
        }
        
    if( ( !err || !err2 )  && nameChanged == EBTLocalNameDefault )
        {
            // Set the name as changed.
        CRepository* cenRep = NULL;
        TRAP( err, cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings ) );
        if( !err )
            { 
            err = cenRep->Set( KBTLocalNameChanged, EBTLocalNameSet );
            TRACE_INFO( ( _L( "[BTENGSETTINGS]\t cenRep->Set( KBTLocalNameChanged, EBTLocalNameSet ): %d"), err) ); 
            }
        delete cenRep;
        }
        
    TRACE_FUNC_RES( ( _L( "result %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// Sets the Bluetooth hardware power state for temporary connections.
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::ChangePowerStateTemporarily()
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    if( !iSettingsWatcher )
        {
        TRAP( err, iSettingsWatcher = CBTEngSettingsNotify::NewL( NULL ) );
        }
    if( !err )
        {
        err = iSettingsWatcher->TogglePowerTemporarily();
        }
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Get the Bluetooth offline mode settings
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTEngSettings::GetOfflineModeSettings( 
        TCoreAppUIsNetworkConnectionAllowed& aOffline, 
        TBTEnabledInOfflineMode& aOfflineAllowed )
    {
    TRACE_FUNC_ENTRY
    aOffline = ECoreAppUIsNetworkConnectionAllowed;
    aOfflineAllowed = EBTEnabledInOfflineMode;
    TRAPD( err, GetCenRepKeyL( KCRUidCoreApplicationUIs, 
                               KCoreAppUIsNetworkConnectionAllowed, (TInt&) aOffline, 
                               ECoreAppUIsNetworkConnectionNotAllowed, 
                               ECoreAppUIsNetworkConnectionAllowed ) );
    if( !err && aOffline == ECoreAppUIsNetworkConnectionNotAllowed )
        {
        TRAP( err, GetCenRepKeyL( KCRUidBluetoothEngine, KBTEnabledInOffline, 
                                  (TInt&) aOfflineAllowed, EBTDisabledInOfflineMode, 
                                  EBTEnabledInOfflineMode ) );
        }
    return err;
    }


// ---------------------------------------------------------------------------
// Checks from central repository whether the Bluetooth friendly name 
// has been modified .
// ---------------------------------------------------------------------------
//
TInt CBTEngSettings::GetLocalNameModified( TBool& aStatus )
    {
    TRACE_FUNC_ENTRY

    aStatus = (TBool) EBTLocalNameDefault;
    TRAPD( err, GetCenRepKeyL( KCRUidBTEngPrivateSettings, KBTLocalNameChanged, 
                               (TInt&) aStatus, EBTLocalNameDefault, 
                               EBTLocalNameSet ) );
    return err;
    }


// ---------------------------------------------------------------------------
// Gets the specified CenRep key and checks if it is within a valid range; 
// if not, it it reset to the default value.
// ---------------------------------------------------------------------------
//
void CBTEngSettings::GetCenRepKeyL( const TUid aUid, const TInt aKey, TInt& aValue, 
                                    const TInt aMinRange1, const TInt aMaxRange1, 
                                    const TInt aMinRange2, const TInt aMaxRange2 )
    {
    TRACE_FUNC_ARG( ( _L( "aKey: %d" ), aKey ) )
// [SvV] Test that both logging macro do the same:
    TRACE_INFO( ( _L( "[BTENGSETTINGS]\t GetCenRepKeyL - start (aKey: %d)" ), aKey ) )
    TInt val = aValue;

    CRepository* cenRep = CRepository::NewL( aUid );
    TInt err = cenRep->Get( aKey, val );
        // Check that the value is within the allowed ranges.
    TBool validRange = ETrue;
    if( val < aMinRange1 || val > aMaxRange1 )
        {
            // The value is not within the first range
        if( !aMinRange2 && !aMaxRange2 )
            {
                // No second range is specified, so the value is out-of-range.
            validRange = EFalse;
            }
        else if( !( val >= aMinRange2 && val <= aMaxRange2 ) )
            {
                // The value is not within the second range either, 
                // so the value is out-of-range.
            validRange = EFalse;
            }
        }
    if( err || !validRange )
        {
        TRACE_INFO( ( _L( "Error occured! err(%d) val(%d), range %d-%d (2nd: %d-%d)" ), 
                       err, val, aMinRange1, aMaxRange1, aMinRange2, aMaxRange2 ) )
            // If an error occured, we cannot trust the central repository,
            // so return to default value to be safe.
        val = aValue;   // Revert to the original value
        (void) cenRep->Set( aKey, val );    // Just try once, ignore return value
        }
    delete cenRep;
    aValue = val;

    TRACE_INFO( ( _L( "[BTENGSETTINGS]\t GetCenRepKeyL - key value: %d; err: %d" ), 
                val, err ) )
    TRACE_FUNC_RES( ( _L( "key value: %d; err: %d" ), val, err ) )
    User::LeaveIfError( err );
    }


// ---------------------------------------------------------------------------
// Gets or sets the BT local name from Bluetooth Registry.
// ---------------------------------------------------------------------------
//
TInt CBTEngSettings::HandleBTRegistryNameSetting( TDes& aName )
    {
    TRACE_FUNC_ENTRY
    RBTRegServ btRegServ;
    RBTLocalDevice btReg;
    TInt err = btRegServ.Connect();
    if( !err )
        {
        err = btReg.Open( btRegServ );
        }
    TBTLocalDevice localDev;
    if( !err )
        {
            // Read the BT local name from BT Registry.
        err = btReg.Get( localDev );
        }
           
    if( !err )
        {
            // BT registry keeps the device name in UTF-8 format.
        TBuf8<KMaxBluetoothNameLen> utf8Name;
        if( aName.Length() == 0 )
            {
                // The error can be > 0 if there are unconverted characters.
            err = CnvUtfConverter::ConvertToUnicodeFromUtf8( aName, 
                                                        localDev.DeviceName() );
            TRACE_INFO( ( _L( "[BTENGSETTINGS]\t getting name from registry: err %d %S" ), 
            		err, &aName))
            }
        else
            {
                // Write the supplied  name to BT Registry.
                // The error can be > 0 if there are unconverted characters.
            err = CnvUtfConverter::ConvertFromUnicodeToUtf8( utf8Name, aName );
            if( !err )
                {
                localDev.SetDeviceName( utf8Name );
                err = btReg.Modify( localDev );
                }
            TRACE_INFO( ( _L( "[BTENGSETTINGS]\t setting name to registry:err %d, %S" ), 
            		err, &aName))
            }
        }
                
    btReg.Close();
    btRegServ.Close();
    TRACE_FUNC_EXIT
    
    return err;
    }
