/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implement PSM plug-in and handle BT PowerState with PSM mode
*
*/


#include <e32property.h>
#include <bt_subscribe.h>
#include <psmtypes.h>
#include <psmsettingsprovider.h>

#include "btengpsmplugin.h"
#include "debug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CBTPsmPlugin::NewL
// Two phased constructor
// ---------------------------------------------------------------------------
//
CBTPsmPlugin* CBTPsmPlugin::NewL(TPsmPluginCTorParams& aInitParams)
    {
    CBTPsmPlugin* self = new (ELeave) CBTPsmPlugin(aInitParams);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CBTPsmPlugin::~CBTPsmPlugin
// Destructor
// ---------------------------------------------------------------------------
//
CBTPsmPlugin::~CBTPsmPlugin()
    {
    TRACE_FUNC_ENTRY

    delete iBtEngSettings;

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// From class CPSMBase
// CBTPsmPlugin::NotifyModeChange()
// Handle BT power state when PowerSaveMode enables/disables.
// ---------------------------------------------------------------------------
//
void CBTPsmPlugin::NotifyModeChange( const TInt aMode )
    {
    iMode = aMode;
    TRAP_IGNORE(NotifyModeChangeL());
    }

// ---------------------------------------------------------------------------
// From class MBTEngSettingsObserver
// CBTPsmPlugin::PowerStateChanged()
// Save the changed power state into PSM storage as settings in Normal mode
// if the change was caused by the user.
// ---------------------------------------------------------------------------
//
void CBTPsmPlugin::PowerStateChanged( TBTPowerStateValue aState )
    {
    TRACE_FUNC_ENTRY

    if( iMode == EPsmsrvModePowerSave && iAllowToConfig)
        {
        TRAP_IGNORE( SavePsmConfigurationL( aState ) );
        }

    if( !iAllowToConfig )
        {
        iAllowToConfig = ETrue;
        }

    TRACE_FUNC_EXIT
    };

// ---------------------------------------------------------------------------
// From class MBTEngSettingsObserver
// CBTPsmPlugin::VisibilityModeChanged()
// Power Saving Mode change does not affect Bluetooth visibility.
// ---------------------------------------------------------------------------
//
void CBTPsmPlugin::VisibilityModeChanged( TBTVisibilityMode /*aState*/ )
    {
    }

// ---------------------------------------------------------
// CBTPsmPlugin::CBTPsmPlugin
// Default C++ constructor
// ---------------------------------------------------------
CBTPsmPlugin::CBTPsmPlugin( TPsmPluginCTorParams& aInitParams )
    : CPsmPluginBase( aInitParams ), iMode( KErrNotFound ), iAllowToConfig( EFalse )
    {
    }

// ---------------------------------------------------------
// CBTPsmPlugin::ConstructL
// Symbian 2nd phase constructor can leave.
// ---------------------------------------------------------
//
void CBTPsmPlugin::ConstructL()
    {
    TRACE_FUNC_ENTRY

    iBtEngSettings = CBTEngSettings::NewL(this);

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CBTPsmPlugin::NotifyModeChangeL()
// Leave function called by NotifyModeChange()
// Handle BT power state when PowerSaveMode enables/disables.
// ---------------------------------------------------------------------------
//
void CBTPsmPlugin::NotifyModeChangeL()
    {
    TRACE_FUNC_ENTRY
    TInt connNum = 0;
    TInt retVal = RProperty::Get(KPropertyUidBluetoothCategory, KPropertyKeyBluetoothGetPHYCount, connNum);
    TRACE_INFO((_L("[BTPsmPlugin]\t GetPHYCount result: %d"), retVal));
    
    if( iMode == EPsmsrvModePowerSave )
        {
        if( !retVal )
            {
            TRACE_INFO((_L("[BTPsmPlugin]\t PHYCount=: %d"), connNum));

            if( connNum > 0 )  // active connection exists
                {
                iAllowToConfig = ETrue;
                }
            else //no active connections
                {
                HandlePowerStateL();
                }
            }
        }

    if( iMode == EPsmsrvModeNormal )
        {
        if( !retVal && connNum == 0 )  // active connection exists
            {
            HandlePowerStateL();
            }
        iAllowToConfig = EFalse;
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CBTPsmPlugin::HandlePowerStateL()
// Handle how to change BT power state
// ---------------------------------------------------------------------------
//
void CBTPsmPlugin::HandlePowerStateL()
    {
    TRACE_FUNC_ENTRY

    TBTPowerStateValue powerBeforeChange;
    iBtEngSettings->GetPowerState( powerBeforeChange );
    if( powerBeforeChange == EBTPowerOff )
        {
        iAllowToConfig = ETrue;
        }

    TBTPowerStateValue powerToChange = EBTPowerOff;
    TRAPD(err, powerToChange = GetSettingsFromPsmStorageL() );
    TRACE_INFO((_L("[BTPsmPlugin]\t CBTPsmPlugin::GetSettingsFromPsmStorageL Leave status: %d"), err));
    // If we can't read previous BT state information from PSM configuration, 
    // BT Power is not toogled only if PSM is switching of. 
    // When switching PSM On BT should be always turned Off.
    // Active connections were checked before.
    if( powerBeforeChange != powerToChange && ( !err || iMode == EPsmsrvModePowerSave ) )
        {
        TInt errCode  = iBtEngSettings->SetPowerState( powerToChange );
        TRACE_INFO((_L("[BTPsmPlugin]\t CBTPsmPlugin::HandlePowerStateL() %d"), errCode));
        }

    if( iMode == EPsmsrvModePowerSave )
        {
        SavePsmConfigurationL( powerBeforeChange );
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CBTPsmPlugin::GetSettingsFromPsmStorageL()
// Retrive power state from PSM configuration file
// ---------------------------------------------------------------------------
//
TBTPowerStateValue CBTPsmPlugin::GetSettingsFromPsmStorageL()
    {
    TRACE_FUNC_ENTRY

    TPsmsrvConfigInfo info;
    info.iConfigId = 1;
    info.iConfigType = EConfigTypeInt;
    info.iIntValue = 0;
    info.iBlocked = EFalse;

    RConfigInfoArray configInfoArray;
    CleanupClosePushL( configInfoArray );

    configInfoArray.AppendL( info );
    iSettingsProvider.GetSettingsL(configInfoArray, KCRUidBluetoothPowerState.iUid);

    TBTPowerStateValue power = EBTPowerOff;
    if( configInfoArray.Count() > 0 )
        {
        power = (TBTPowerStateValue) configInfoArray[0].iIntValue;
        }
    CleanupStack::PopAndDestroy( &configInfoArray );

    TRACE_FUNC_EXIT
    return power;
    }

// ---------------------------------------------------------
// CBTPsmPlugin::UpdatePsmConfiguration
// Write BTPowerState into PSM Configuration file
// ---------------------------------------------------------
//
void CBTPsmPlugin::SavePsmConfigurationL(TBTPowerStateValue aState)
    {
    TRACE_FUNC_ENTRY

    TPsmsrvConfigInfo infoForNormal;
    infoForNormal.iConfigId = 1;
    infoForNormal.iConfigType = EConfigTypeInt;
    infoForNormal.iIntValue = aState;
    infoForNormal.iBlocked = EFalse;

    RConfigInfoArray configInfo;
    configInfo.AppendL(infoForNormal);
    iSettingsProvider.BackupSettingsL( configInfo, KCRUidBluetoothPowerState.iUid );
    configInfo.Close();

    TRACE_FUNC_EXIT
    }
