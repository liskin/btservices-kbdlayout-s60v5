/*
* Copyright (c) 2002 - 2007 Nokia Corporation and/or its subsidiary(-ies).
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



// INCLUDE FILES
#include <e32std.h>
#include <e32base.h>
#include <e32cmn.h>
#include <StifParser.h>
#include <Stiftestinterface.h>

#include <btengconnman.h>
#include <btengdevman.h>
#include <btengdiscovery.h>
#include <btengsettings.h>

#include "btapitest.h"
#include "bttestlogger.h"
#include "btapisettings.h"
#include "btapiconnman.h"
#include "btapidevman.h"
#include "btapidiscovery.h"


// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBtApiTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CBtApiTest::Delete() 
    {

    }

// -----------------------------------------------------------------------------
// CBtApiTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CBtApiTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {
        ENTRY( "PowerState", CBTEngSettings_SetAndGetPowerState ),
        ENTRY( "Visibility", CBTEngSettings_SetAndGetVisibility ),
        ENTRY( "LocalName", CBTEngSettings_SetAndGetLocalName ),
        ENTRY( "AddDevices", CBTEngDevMan_AddDevices ),
        ENTRY( "GetDevices", CBTEngDevMan_GetDevices ),
        ENTRY( "ModifyDevice", CBTEngDevMan_ModifyDevice ),
        ENTRY( "SearchRemoteDevice", CBTEngDiscovery_SearchRemoteDevice ),
        ENTRY( "RegisterAndDeleteSdpRecord", CBTEngDiscovery_RegisterAndDeleteSdpRecord ),
        ENTRY( "RemoteSdpQuery", CBTEngDiscovery_RemoteSdpQuery ),
        ENTRY( "ParseSdpAttrValues", CBTEngDiscovery_ParseSdpAttrValues ),
        ENTRY( "ConnectDisconnectAndCancelConnect", CBTEngConnMan_ConnectDisconnectAndCancelConnect ),
        ENTRY( "GetAddresses", CBTEngConnMan_GetAddresses ),
        ENTRY( "IsConnectable", CBTEngConnMan_IsConnectable ),
        ENTRY( "PairDevice", CBTEngConnMan_PairDevice ),
        ENTRY( "PrepareDiscovery", CBTEngConnMan_PrepareDiscovery )
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CBtApiTest::TestCompleted
// -----------------------------------------------------------------------------
//  
void CBtApiTest::TestCompleted( TInt aErr, const TUint8* aFunc, const TDesC& aArg )
    {
    iTestLogger->LogResult( (TPtrC8( aFunc )), aArg, aErr );
    Signal( aErr );
    }

// -----------------------------------------------------------------------------
// CBtApiTest::Logger
// -----------------------------------------------------------------------------
//
CBtTestLogger& CBtApiTest::Logger()
    {
    return *iTestLogger;
    }


// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngSettings_SetAndGetPowerState
// -----------------------------------------------------------------------------
//
TInt CBtApiTest::CBTEngSettings_SetAndGetPowerState( CStifItemParser& /*aItem*/ )
    {
    TInt ret = KErrNone;
    
    ret = iSettings->SwitchPowerState();
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("SwitchPowerState 1"));
        return KErrNone;
        }
    ret = iSettings->GetPowerState();
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("GetPowerState"));
        return KErrNone;
        }
    ret = iSettings->TurnBtOn();

    TestCompleted( ret, TLFUNCLOG, _L("TurnBtOn)"));
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngSettings_SetAndGetVisibility
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngSettings_SetAndGetVisibility( CStifItemParser& /*aItem*/ )
    {
    TInt ret = KErrNone;
    
    ret = iSettings->SetVisibilityMode( EBTVisibilityModeHidden );
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("Set EBTVisibilityModeHidden"));
        return KErrNone;
        }
    ret = iSettings->GetVisibilityMode();
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("Get EBTVisibilityModeHidden"));
        return KErrNone;
        }
    ret = iSettings->SetVisibilityMode( EBTVisibilityModeTemporary, 10 );
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("Set EBTVisibilityModeTemporary"));
        return KErrNone;
        }
    ret = iSettings->GetVisibilityMode();
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("Get EBTVisibilityModeTemporary"));
        return KErrNone;
        }
    ret = iSettings->SetVisibilityMode( EBTVisibilityModeGeneral );
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("Set EBTVisibilityModeGeneral"));
        return KErrNone;
        }
    ret = iSettings->GetVisibilityMode();
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("Get EBTVisibilityModeGeneral"));
        return KErrNone;
        }
        
    TestCompleted( ret, TLFUNCLOG, _L(""));
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngSettings_SetAndGetLocalName
// -----------------------------------------------------------------------------
//
TInt CBtApiTest::CBTEngSettings_SetAndGetLocalName( CStifItemParser& aItem )
    {
    TPtrC name(KNullDesC);
    TInt ret = aItem.GetNextString( name );
    iTestLogger->Log( CBtTestLogger::ETLInfo, _L( "Set new local name: %S" ),  &name);
    if ( ret )
        {
        TestCompleted( KErrArgument, TLFUNCLOG, name );
        return KErrNone;
        }

    ret = iSettings->SetLocalName( name );
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, name );
        return KErrNone;
        }
    
    ret = iSettings->GetLocalName();

    TestCompleted( ret, TLFUNCLOG, name );
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDevMan_AddDevice
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngDevMan_AddDevices( CStifItemParser& /*aItem*/ )
    {
    TInt leave = KErrNone;
    TInt err = KErrNone;
    TRAP( leave, err = iDevMan->AddDevicesL() );
    if ( leave )
        {
        TestCompleted( leave, TLFUNCLOG, _L ("Leave occured!") );
        return KErrNone;
        }
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDevMan_GetDevices
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngDevMan_GetDevices( CStifItemParser& /*aItem*/ )
    {
    TInt leave = KErrNone;
    TInt err = KErrNone;
    TRAP( leave, err = iDevMan->GetDevicesL() );
    if ( leave )
        {
        TestCompleted( leave, TLFUNCLOG, _L ("Leave occured!") );
        return KErrNone;
        }
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDevMan_ModifyDevice
// -----------------------------------------------------------------------------
//
TInt CBtApiTest::CBTEngDevMan_ModifyDevice( CStifItemParser& /*aItem*/ )
    {
    TInt leave = KErrNone;
    TInt err = KErrNone;
    TRAP( leave, err = iDevMan->ModifyDevicesL() );
    if ( leave )
        {
        TestCompleted( leave, TLFUNCLOG, _L ("Leave occured!") );
        return KErrNone;
        }
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDiscovery_SearchRemoteDevice
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngDiscovery_SearchRemoteDevice( CStifItemParser& /*aItem*/ )
    {
    TInt leave = KErrNone;
    TInt err = KErrNone;
    TRAP( leave, err = iDiscovery->SearchRemoteDeviceL() );
    if ( leave )
        {
        TestCompleted( leave, TLFUNCLOG, _L ("Leave occured!") );
        return KErrNone;
        }
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDiscovery_RegisterAndDeleteSdpRecord
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngDiscovery_RegisterAndDeleteSdpRecord( CStifItemParser& /*aItem*/ )
    {
    TInt err = KErrNone;
    err = iDiscovery->RegisterSdpRecord();
    if( err )
        {
        TestCompleted( err, TLFUNCLOG, _L( "Register SDP record failed!" ) );
        return KErrNone;
        }
    err = iDiscovery->DeleteSdpRecord();
    if( err )
        {
        TestCompleted( err, TLFUNCLOG, _L( "Delete SDP record failed!" ) );
        }
        
    TestCompleted( err, TLFUNCLOG, _L("") );    
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDiscovery_RemoteSdpQuery
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngDiscovery_RemoteSdpQuery( CStifItemParser& aItem )
    {
    TInt leave = KErrNone;
    TInt err = KErrNone;
     
    TPtrC item( KNullDesC );
    err = aItem.GetNextString( item );
    if ( err )
        {
        TestCompleted( err, TLFUNCLOG, _L ("StifItemParser::GetNextString failed!") );
        return KErrNone;
        }
    TBTDevAddr addr;
    addr.SetReadable( item );
    
    TRAP( leave, err = iDiscovery->RemoteSdpQueryL( addr ) );
    if ( leave )
        {
        TestCompleted( leave, TLFUNCLOG, _L ("RemoteSdpQueryL failed! Leave occured!") );
        return KErrNone;
        }
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngDiscovery_ParseSdpAttrValues
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngDiscovery_ParseSdpAttrValues( CStifItemParser& /*aItem*/ )
    {
    TInt err = KErrNone;
    err = iDiscovery->ParseSdpAttrValues();
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngConnMan_ConnectDisconnectAndCancelConnect
// -----------------------------------------------------------------------------
//   
TInt CBtApiTest::CBTEngConnMan_ConnectDisconnectAndCancelConnect( CStifItemParser& aItem )
    {
    TInt leave = KErrNone;
    TInt err = KErrNone;
    
    TPtrC item( KNullDesC );
    err = aItem.GetNextString( item );
    if ( err )
        {
        TestCompleted( err, TLFUNCLOG, _L ("StifItemParser::GetNextString failed!") );
        return KErrNone;
        }
    
    TBTDevAddr addr;
    addr.SetReadable( item );
    
    TBTDeviceClass cod(EMajorServiceAudio, 0, 0);
 
    err = iConnman->ConnectIfNotConnected( addr, cod );
    if ( err )
        {
        // TestCompleted( err, TLFUNCLOG, _L ("ConnectIfNotConnected failed!") );
        // return KErrNone;    
        }
        
    err = iConnman->DisconnectIfConnected( addr );
    if ( err )
        {
        // TestCompleted( err, TLFUNCLOG, _L ("ConnectIfNotConnected failed!") );
        // return KErrNone;    
        }
        
    err = iConnman->ConnectAndCancel( addr, cod );
    if ( err )
        {
        // TestCompleted( err, TLFUNCLOG, _L ("ConnectAndCancel failed!") );
        // return KErrNone;    
        }
        
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngConnMan_GetAddresses
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngConnMan_GetAddresses( CStifItemParser& aItem )
    {
    TPtrC item( KNullDesC );
    TInt err = aItem.GetNextString( item );
    if ( err )
        {
        TestCompleted( err, TLFUNCLOG, _L ("StifItemParser::GetNextString failed!") );
        return KErrNone;
        }
    
    TBTDevAddr addr;
    addr.SetReadable( item );
    
    err = iConnman->GetAddresses( addr );
    TestCompleted( err, TLFUNCLOG, item );        
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngConnMan_IsConnectable
// -----------------------------------------------------------------------------
//
TInt CBtApiTest::CBTEngConnMan_IsConnectable( CStifItemParser& /*aItem*/ )
    {
    TInt err = KErrNone;
    err = iConnman->IsConnectable();
    TestCompleted( err, TLFUNCLOG, _L ("") );        
    return KErrNone;
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngConnMan_PairDevice
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngConnMan_PairDevice( CStifItemParser& aItem )
    {
    TPtrC item( KNullDesC );
    TInt err = aItem.GetNextString( item );
    if ( err )
        {
        TestCompleted( err, TLFUNCLOG, _L ("StifItemParser::GetNextString failed!") );
        return KErrNone;
        }
    TBTDevAddr addr;
    addr.SetReadable( item );
    
    err = iConnman->PairDevice( addr );
    TestCompleted( err, TLFUNCLOG, item );        
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngConnMan_PrepareDiscovery
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngConnMan_PrepareDiscovery( CStifItemParser& aItem )
    {
    TPtrC item( KNullDesC );
    TInt err = aItem.GetNextString( item );
    if ( err )
        {
        TestCompleted( err, TLFUNCLOG, _L ("StifItemParser::GetNextString failed!") );
        return KErrNone;
        }
    TBTDevAddr addr;
    addr.SetReadable( item );
    
    err = iConnman->PrepareDiscovery( addr );
    TestCompleted( err, TLFUNCLOG, item );        
    return KErrNone;
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  End of File
