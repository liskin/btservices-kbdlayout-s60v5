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
        ENTRY( "PowerState", CBtApiTest::CBTEngSettings_GetAndSetPowerState ),
        ENTRY( "Visibility", CBtApiTest::CBTEngSettings_SetAndGetVisibility ),
        ENTRY( "LocalName", CBtApiTest::CBTEngSettings_SetAndGetLocalName ),
        ENTRY( "GetOfflineModeSettingsApi", CBtApiTest::CBTEngSettings_GetOfflineModeSettingsApi ),
        ENTRY( "ChangePowerStateTemporarily", CBtApiTest::CBTEngSettings_ChangePowerStateTemporarily ),
        ENTRY( "CP", CBtApiTest::CBTEngSettings_ChangePowerStateTemporarily ),
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
TInt CBtApiTest::CBTEngSettings_GetAndSetPowerState( CStifItemParser& /*aItem*/ )
    {
  /*  
    TInt err = KErrNone;
    TBTPowerStateValue state, expState;
    TInt i = 0;
    
    for ( i=0 ; i<2 ; i++ )
    	{
    	state = EBTPowerOn;
    	err = iSettings->SetPowerState( state );
    	iSettings->GetExpectedState( expState );
    	iTestLogger->Log(CBtTestLogger::ETLDebug, _L( "CBTApiSettings::SetAndGetPowerState: %d) Error %d -:- state %d -:- expected state %d" ), i, err, state, expState );
    	iTestLogger->LogNewLine();
    	if ( err )
    		{
    		TestCompleted( err, TLFUNCLOG, _L("SetAndGetPowerState: Problem with state seting)"));	
    		return err;
    		}
    	}
    TestCompleted( err, TLFUNCLOG, _L("SetAndGetPowerState: Done)"));
    return err;
*/    
/*    
    for ( i=0 ; i<2 ; i++ )
    	{
    	iSettings->GetExpectedState( expState );
    	err = iSettings->GetPowerState( state );
    	iTestLogger->Log(CBtTestLogger::ETLDebug, _L( "CBTApiSettings::SetAndGetPowerState: %d) Error %d -:- state %d -:- expected state %d" ), i, err, state, expState );
    	iTestLogger->LogNewLine();
    	if ( err )
    		{
    		TestCompleted( err, TLFUNCLOG, _L("SetAndGetPowerState: Problem with state reading)"));	
    		return err;
    		}
    	}
    TestCompleted( err, TLFUNCLOG, _L("SetAndGetPowerState: Done)"));
	return err;
  */  
    
/*************************
 * Working implementation
 * **********************
 
    TInt err = KErrNone;
    
    err = iSettings->TurnBtOn();
    if( err )
        {
        TestCompleted( err, TLFUNCLOG, _L("TurnBTOn"));
        return err;
        }
 */
    TInt err = KErrNone;
    
    err = iSettings->SwitchPowerState();
    if( err )
    	{
    	TestCompleted( err, TLFUNCLOG, _L("SwitchPowerState (1)"));
    	return err;
    	}
    
	TestCompleted( err, TLFUNCLOG, _L("SetAndGetPowerState: Done)"));
    return err;    
     
/*
    TInt ret = KErrNone;
        
    ret = iSettings->SwitchPowerState();
    _LIT(KSetAndGetPowerStateSwitchPowerState, "\nSetAndGetPowerState:> SwitchPowerState = %d");
    iLog->Log(KSetAndGetPowerStateSwitchPowerState, ret);    
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("SwitchPowerState 1"));
        return KErrNone;
        }
  
    ret = iSettings->GetPowerState();
    _LIT(KSetAndGetPowerStateGetPowerState, "\nSetAndGetPowerState:> GetPowerState err = %d");
    iLog->Log(KSetAndGetPowerStateGetPowerState, ret);
    if( ret )
        {
        TestCompleted( ret, TLFUNCLOG, _L("GetPowerState"));
        return KErrNone;
        }

    ret = iSettings->TurnBtOn();
    _LIT(KSetAndGetPowerStateTurnBtOn, "\nSetAndGetPowerState:> TurnBtOn = %d");
    iLog->Log(KSetAndGetPowerStateTurnBtOn, ret);
    
    TestCompleted( ret, TLFUNCLOG, _L("TurnBtOn)"));
    
    return KErrNone;
    */
    }
    
// -----------------------------------------------------------------------------
// CBtApiTest::CBTEngSettings_SetAndGetVisibility
// -----------------------------------------------------------------------------
//    
TInt CBtApiTest::CBTEngSettings_SetAndGetVisibility( CStifItemParser& /*aItem*/ )
    {
    TInt ret = KErrNone;
    
    ret = iSettings->SetVisibilityNormalState();
    if( ret )
       	{
       		TestCompleted( ret, TLFUNCLOG, _L("SetVisibilityNormalState() method error"));
       		return KErrNone;
       	}
    
    
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

    ret = iSettings->SetVisibilityMode( EBTVisibilityModeTemporary, 1 );
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
    

TInt CBtApiTest::CBTEngSettings_GetOfflineModeSettingsApi( CStifItemParser& /*aItem*/ )
    {
    TPtrC name(KNullDesC);
    TInt err(KErrNone);
    
    TCoreAppUIsNetworkConnectionAllowed offline; 
    TBTEnabledInOfflineMode offlineAllowed; 

    err = iSettings->GetOfflineModeSettings(offline, offlineAllowed);
    iTestLogger->Log( CBtTestLogger::ETLInfo, _L( "Start GetOfflineModeSettingsApiTest> %d %d" ), offline, offlineAllowed);
    if( err )
        {
        TestCompleted( err, TLFUNCLOG, _L("") );
        return err;
        }
    
    switch( offline )
    	{
    	case ECoreAppUIsNetworkConnectionNotAllowed:
    		iTestLogger->Log( CBtTestLogger::ETLInfo, _L( "Network connection not allowed (%d)" ), ECoreAppUIsNetworkConnectionNotAllowed);
			break;
    	case ECoreAppUIsNetworkConnectionAllowed:
    		iTestLogger->Log( CBtTestLogger::ETLInfo, _L( "Network connection allowed (%d)" ), ECoreAppUIsNetworkConnectionAllowed);
			break;
    	}

    switch( offlineAllowed )
    	{
    	case EBTDisabledInOfflineMode:
    		iTestLogger->Log( CBtTestLogger::ETLInfo, _L( "BT activation disabled in offline mode (%d)" ), EBTDisabledInOfflineMode);
    		break;
		case EBTEnabledInOfflineMode:
			iTestLogger->Log( CBtTestLogger::ETLInfo, _L( "BT activation enabled in offline mode (%d)" ), EBTEnabledInOfflineMode);
   			break;
    	}    
    
    TestCompleted( err, TLFUNCLOG, _L("") );
    return err;
    }
    
TInt CBtApiTest::CBTEngSettings_ChangePowerStateTemporarily( CStifItemParser& /*aItem*/ )
{
	TPtrC name(KNullDesC);
	TInt err(KErrNone);
	
	err = iSettings->ChangePowerStateTemporarily();
	if( err )
	  	{
			TestCompleted(err, TLFUNCLOG, _L(""));
			return err;
	    }
	
	TestCompleted(err, TLFUNCLOG, _L(""));
	return err;
}

// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  End of File
