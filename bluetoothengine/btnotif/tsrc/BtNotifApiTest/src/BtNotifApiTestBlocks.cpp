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
* Description:   ?Description
*
*/



// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>
#include "BtNotifApiTest.h"

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

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBtNotifApiTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CBtNotifApiTest::Delete() 
    {
        delete iNotifier;
        iNotifier = NULL;
        delete iBtSettings;
        iBtSettings = NULL;
        delete iBtEngDevManObserver;
        iBtEngDevManObserver = NULL;
        delete iBtDeviceArray;
        iBtDeviceArray = NULL;
        delete iTestDevice;
        iTestDevice = NULL;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CBtNotifApiTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
                
        ENTRY( "PairedDeviceSettingNotifierTest", CBtNotifApiTest::PairedDeviceSettingNotifierTest ),
        ENTRY( "FinishPairedDeviceSettingQuery", CBtNotifApiTest::FinishPairedDeviceSettingQuery ),
                
        ENTRY( "PbapAuthNotifierTest", CBtNotifApiTest::PbapAuthNotifierTest ),
        ENTRY( "FinishPbapAuthQuery", CBtNotifApiTest::FinishPbapAuthQuery ),
                
        ENTRY( "PasskeyDisplayNotifierTest", CBtNotifApiTest::PasskeyDisplayNotifierTest ),
        ENTRY( "FinishPasskeyDisplayQuery", CBtNotifApiTest::FinishPasskeyDisplayQuery ),

        ENTRY( "NumericComparisonQueryNotifierTest", CBtNotifApiTest::NumericComparisonQueryNotifierTest ),
        ENTRY( "FinishNumericComparisonQuery", CBtNotifApiTest::FinishNumericComparisonQuery ),
        
        ENTRY( "AuthQueryNotifierTest", CBtNotifApiTest::AuthQueryNotifierTest ),
        ENTRY( "FinishAuthQuery", CBtNotifApiTest::FinishAuthQuery ),
        
        ENTRY( "PinPasskeyQueryNotifierTest", CBtNotifApiTest::PinPasskeyQueryNotifierTest ),
        ENTRY( "FinishPinPasskeyQuery", CBtNotifApiTest::FinishPinPasskeyQuery ),
        
        ENTRY( "InquiryNotifierTest", CBtNotifApiTest::InquiryNotifierTest ),
        ENTRY( "FinishInquiry", CBtNotifApiTest::FinishInquiry ),
        
        ENTRY( "ObexPasskeyQueryNotifierTest", CBtNotifApiTest::ObexPasskeyQueryNotifierTest ),
        ENTRY( "FinishObexPasskeyQuery", CBtNotifApiTest::FinishObexPasskeyQuery ),
        
        ENTRY( "PowerModeQueryNotifierTest", CBtNotifApiTest::PowerModeQueryNotifierTest ),
        ENTRY( "FinishPowerModeQuery", CBtNotifApiTest::FinishPowerModeQuery ),
        
        ENTRY( "GenericInfoNotifierTest", CBtNotifApiTest::GenericInfoNotifierTest ),
        
        ENTRY( "GenericQueryNotifierTest", CBtNotifApiTest::GenericQueryNotifierTest ),
        ENTRY( "FinishGenericQuery", CBtNotifApiTest::FinishGenericQuery ),
        
        ENTRY( "TurnLightsOn", CBtNotifApiTest::TurnLightsOn ),
                
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::PairedDeviceSettingNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::PairedDeviceSettingNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TBTRegistrySearch regSearch;
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    regSearch.FindAll();    
    res = iBtEngDevManObserver -> GetDevices( regSearch, iBtDeviceArray );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Getting devices from register failed with value: %d"), res );
        return res;
        }

    if ( iBtDeviceArray -> Count() > 0 )
        {
        iBtDeviceArray -> Reset();
        res = iBtEngDevManObserver -> DeleteDevices( regSearch );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Deleting devices from register failed with value: %d"), res );
            return res;
            }
        }

    TBTDevAddr addr( 0x12345678 );
    CBTDevice *testDevice = CBTDevice::NewL( addr );
    CleanupStack::PushL( testDevice );
    testDevice -> SetPaired( ELinkKeyAuthenticated );
    TTime time;
    time.UniversalTime();
    testDevice -> SetUsed( time );
    testDevice -> SetDeviceNameL( _L8( "TestedDevice" ) );
    CleanupStack::Pop( testDevice );
    
    res = iBtEngDevManObserver -> AddDevice( *testDevice );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Adding device to register failed with value: %d"), res );
        delete testDevice;
        return res;
        }

    TInt paramStatus = KErrNone;  
    TBTPairedDeviceSettingParams notifierParams;    
    notifierParams.iPairingStatus = paramStatus;
    notifierParams.iRemoteAddr = testDevice -> BDAddr();
    TBTPairedDeviceSettingParamsPckg notifierParamsPckg( notifierParams );
       
    TPckgBuf<TBool> notifierResponsePckg;
    
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTPairedDeviceSettingNotifierUid, notifierParamsPckg, notifierResponsePckg );
    iLog -> Log( _L("Notifier started") );    
    
    User::WaitForRequest( iReqStatus );
  
    iLog -> Log( _L("Request status value: %d"), iReqStatus.Int() );
    iLog -> Log( _L("Input status value: %d"), notifierParams.iPairingStatus );
    iLog -> Log( _L("Pairing status value: %d"), notifierResponsePckg() );
    
    delete testDevice;
    iNotifier -> Close();  
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishPairedDeviceSettingQuery( CStifItemParser& aItem )
    {
    iLog -> Log( _L("Test case passed") );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::PbapAuthNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::PbapAuthNotifierTest( CStifItemParser& aItem )
    {
    TInt res;

    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    TPbapAuthNotifierParams notifierParams = TPbapAuthNotifierParams();
    TPbapAuthNotifierParamsPckg notifierParamsPckg( notifierParams );
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KPbapAuthNotifierUid, notifierParamsPckg, iPbapAuthNotifierResponsePckg );
    iLog -> Log( _L("Notifier started") );

    return KErrNone;
    }


TInt CBtNotifApiTest::FinishPbapAuthQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TTestOption option;
    TPbapAuthNotifierResponse expectedData;
    
    if ( !expectedData.SetPassword( _L( "0000" ) ) )
        {
        iLog -> Log( _L("SetPassword failed") );
        iNotifier -> Close();
        return KErrGeneral;
        }       
    
    res = aItem.GetString( _L( "FinishPbapAuthQuery" ), optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    User::WaitForRequest( iReqStatus );    
    if ( iReqStatus.Int()!= KErrNone && iReqStatus.Int()!= KErrCancel )
        {
        iLog -> Log( _L("Failed to get notifier's response: %d"), iReqStatus.Int() );
        iNotifier -> Close();
        return iReqStatus.Int();
        }
    iLog -> Log( _L("Notifier responded") );
    
    switch ( option )
        {
        case ECheckPasskey:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Equal passkeys expected") );
            STIF_ASSERT_EQUALS( expectedData.Password(), iPbapAuthNotifierResponsePckg().Password() );                       
            break;
        case EQueryCanceled:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
            STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
            break;    
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
            
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::PasskeyDisplayNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::PasskeyDisplayNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TBTRegistrySearch regSearch;
    TPtrC initOptionName( KNullDesC );
    TUint32 numericalValue = 999;
    TBTPasskeyDisplayParams notifierParams;
    TBTPasskeyDisplayParamsPckg emptyReplyPckg;
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    regSearch.FindAll();    
    res = iBtEngDevManObserver -> GetDevices( regSearch, iBtDeviceArray );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Getting devices from register failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }

    if ( iBtDeviceArray -> Count() > 0 )
        {
        iBtDeviceArray -> Reset();
        res = iBtEngDevManObserver -> DeleteDevices( regSearch );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Deleting devices from register failed with value: %d"), res );
            iNotifier -> Close();
            return res;
            }
        }
    
    iTestDevice -> SetDeviceAddress( 0x12345678 );
    iTestDevice -> SetDeviceNameL( _L8( "Tested_Device" ) );
    iTestDevice -> SetFriendlyNameL( _L( "TestedDevice" ) );
    
    res = iBtEngDevManObserver -> AddDevice( *iTestDevice );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Adding device to register failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    res = aItem.GetString( _L( "PasskeyDisplayNotifierTest" ), initOptionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        } 
    
    if ( !initOptionName.Compare( _L( "LOCAL" ) ) )
        {
        iLog -> Log( _L("Locally initiated connection") );
        notifierParams = TBTPasskeyDisplayParams( iTestDevice -> BDAddr(), iTestDevice -> FriendlyName(), numericalValue, ETrue );
        }
    else if ( !initOptionName.Compare( _L( "REMOTE" ) ) )
        {
        iLog -> Log( _L("Remote side initiated connection") );
        notifierParams = TBTPasskeyDisplayParams( iTestDevice -> BDAddr(), iTestDevice -> FriendlyName(), numericalValue, EFalse );
        }
    else
        {
        iLog -> Log( _L("Option unavailable") );
        return KErrNotFound;
        }
       
    TBTPasskeyDisplayParamsPckg notifierParamsPckg( notifierParams );
    
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTPasskeyDisplayNotifierUid, notifierParamsPckg, emptyReplyPckg );
    iLog -> Log( _L("Notifier started") );

    return KErrNone;
    }

TInt CBtNotifApiTest::FinishPasskeyDisplayQuery( CStifItemParser& aItem )
    {
    TInt res;
    TBTRegistrySearch regSearch;
    TPtrC deviceBlockOptionName( KNullDesC );
    
    res = aItem.GetString( _L( "FinishPasskeyDisplayQuery" ), deviceBlockOptionName );   
    
    User::WaitForRequest( iReqStatus );   
    
    if ( !deviceBlockOptionName.Compare( _L( "BLOCK" ) ) )
        {
        iLog -> Log( _L("Block test device") );
        iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrAccessDenied );
        STIF_ASSERT_EQUALS( KErrAccessDenied, iReqStatus.Int() );
        }
    else if ( !deviceBlockOptionName.Compare( _L( "NONBLOCK" ) ) )
        {
        iLog -> Log( _L("Don't block test device") );
        iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
        STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
        }
    else
        {
        iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
        STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
        }
    
    iBtDeviceArray -> Reset();
    res = iBtEngDevManObserver -> DeleteDevices( regSearch );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Deleting devices from register failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::NumericComparisonQueryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::NumericComparisonQueryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC initOptionName( KNullDesC );
    TUint32 numericalValue = 999999;
    TBTNumericComparisonParams notifierParams;
    
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    res = aItem.GetString( _L( "NumericComparisonQueryNotifierTest" ), initOptionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        } 
    
    if ( !initOptionName.Compare( _L( "LOCAL" ) ) )
        {
        iLog -> Log( _L("Locally initiated connection") );
        notifierParams = TBTNumericComparisonParams( TBTDevAddr(), _L( "Test Device" ), numericalValue, TBTNumericComparisonParams::ERemoteCannotConfirm, ETrue );
        }
    else if ( !initOptionName.Compare( _L( "REMOTE" ) ) )
        {
        iLog -> Log( _L("Remote side initiated connection") );
        notifierParams = TBTNumericComparisonParams( TBTDevAddr(), _L( "Test Device" ), numericalValue, TBTNumericComparisonParams::ERemoteCannotConfirm, EFalse );
        }
    else
        {
        iLog -> Log( _L("Option unavailable") );
        return KErrNotFound;
        }
    
    TBTNumericComparisonParamsPckg notifierParamsPckg( notifierParams );
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTNumericComparisonNotifierUid, notifierParamsPckg, iNumericComparisonQueryReply );
    iLog -> Log( _L("Notifier started") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishNumericComparisonQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TPtrC deviceBlockOptionName( KNullDesC );
    TTestOption option;
    
    res = aItem.GetString( _L( "FinishNumericComparisonQuery" ), optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    User::WaitForRequest( iReqStatus );    
    if ( iReqStatus.Int()!= KErrNone && iReqStatus.Int()!= KErrCancel && iReqStatus.Int()!= KErrAccessDenied )
        {
        iLog -> Log( _L("Failed to get notifier's response: %d"), iReqStatus.Int() );
        iNotifier -> Close();
        return iReqStatus.Int();
        }
    iLog -> Log( _L("Notifier responded") );
    
    switch ( option )
        {
        case EQueryDiscarded:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Notifier's response value: %d, expected: %b"), iNumericComparisonQueryReply(), EFalse );  
            STIF_ASSERT_FALSE( iNumericComparisonQueryReply() );
            break;
        case EQueryAccepted:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Notifier's response value: %d, expected: %b"), iNumericComparisonQueryReply(), ETrue );  
            STIF_ASSERT_TRUE( iNumericComparisonQueryReply() );
            break;
        case EQueryCanceled:
            res = aItem.GetNextString( deviceBlockOptionName );   
            if ( res != KErrNone )
                {
                iLog -> Log( _L("GetNextString failed with value: %d"), res );
                iNotifier -> Close();
                return res;
                }
            
            if ( !deviceBlockOptionName.Compare( _L( "BLOCK" ) ) )
                {
                iLog -> Log( _L("Block test device") );
                iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrAccessDenied );
                STIF_ASSERT_EQUALS( KErrAccessDenied, iReqStatus.Int() );
                }
            else if ( !deviceBlockOptionName.Compare( _L( "NONBLOCK" ) ) )
                {
                iLog -> Log( _L("Don't block test device") );
                iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
                STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
                }
            else
                {
                iLog -> Log( _L("Option unavailable") );
                return KErrNotFound;
                }
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }

    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::AuthQueryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::AuthQueryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    TBTAuthorisationParams notifierParams;
    notifierParams.iName = _L( "Test Device" );
    TBTAuthorisationParamsPckg notifierParamsPckg( notifierParams );
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTManAuthNotifierUid, notifierParamsPckg, iAuthQueryReply );
    iLog -> Log( _L("Notifier started") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishAuthQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TPtrC deviceBlockOptionName( KNullDesC );
    TTestOption option;
    
    res = aItem.GetString( _L("FinishAuthQuery"), optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    res = aItem.GetNextString( deviceBlockOptionName );
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    User::WaitForRequest( iReqStatus );
    if ( iReqStatus.Int()!= KErrNone && iReqStatus.Int()!= KErrCancel && iReqStatus.Int()!= KErrAccessDenied )
        {
        iLog -> Log( _L("Failed to get notifier's response: %d"), iReqStatus.Int() );
        iNotifier -> Close();
        return iReqStatus.Int();
        }
    iLog -> Log( _L("Notifier responded") );
    
    switch ( option )
        {
        case EQueryDiscarded:
            if ( !deviceBlockOptionName.Compare( _L( "BLOCK" ) ) )
                {
                iLog -> Log( _L("Block test device") );
                iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrAccessDenied );
                STIF_ASSERT_EQUALS( KErrAccessDenied, iReqStatus.Int() );
                iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iAuthQueryReply(), EFalse );  
                STIF_ASSERT_FALSE( iAuthQueryReply() );
                }
            else if ( !deviceBlockOptionName.Compare( _L( "NONBLOCK" ) ) )
                {
                iLog -> Log( _L("Don't block test device") );
                iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
                STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
                iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iAuthQueryReply(), EFalse );  
                STIF_ASSERT_FALSE( iAuthQueryReply() );
                }
            else
                {
                iLog -> Log( _L("Option unavailable") );
                return KErrNotFound;
                }
            break;
        case EQueryAccepted:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iAuthQueryReply(), ETrue );
            STIF_ASSERT_TRUE( iAuthQueryReply() );
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::PinPasskeyQueryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::PinPasskeyQueryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC initOptionName( KNullDesC );
    TUint pinCodeMinLength = 4;
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    res = aItem.GetString( _L( "PinPasskeyQueryNotifierTest" ), initOptionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    TBTPinCodeEntryNotifierParams notifierParams;
    
    if ( !initOptionName.Compare( _L( "LOCAL" ) ) )
        {
        iLog -> Log( _L("Locally initiated connection") );
        notifierParams = TBTPinCodeEntryNotifierParams( TBTDevAddr(), _L( "Test Device" ), pinCodeMinLength, ETrue, EFalse, pinCodeMinLength );
        }
    else if ( !initOptionName.Compare( _L( "REMOTE" ) ) )
        {
        iLog -> Log( _L("Remote side initiated connection") );
        notifierParams = TBTPinCodeEntryNotifierParams( TBTDevAddr(), _L( "Test Device" ), pinCodeMinLength, EFalse, EFalse, pinCodeMinLength );
        }
    else
        {
        iLog -> Log( _L("Option unavailable") );
        return KErrNotFound;
        }
          
    TBTPinCodeEntryNotifierParamsPckg notifierParamsPckg( notifierParams );
       
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTPinCodeEntryNotifierUid, notifierParamsPckg, iEnteredPinPasskey );
    iLog -> Log( _L("Notifier started") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishPinPasskeyQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TPtrC connectionOptionName( KNullDesC );
    TPtrC deviceBlockOptionName( KNullDesC );
    TTestOption option;    
    TPINCodeV10 pinKey;
    
    pinKey.iLength = 4;
    pinKey.iPIN[0] = '0';
    pinKey.iPIN[1] = '0';
    pinKey.iPIN[2] = '0';
    pinKey.iPIN[3] = '0';
    
    for ( int i = pinKey.iLength; i< KHCIPINCodeSize; i++ )
        {
        pinKey.iPIN[i] = NULL;
        } 
    iPinPasskey = pinKey;   
    
    res = aItem.GetString( _L( "FinishPinPasskeyQuery" ), optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    User::WaitForRequest( iReqStatus );
    
    switch( option )
        {
        case ECheckPasskey:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Equal passkeys expected") );
            STIF_ASSERT_EQUALS( iPinPasskey, iEnteredPinPasskey );                       
            break;
        case EQueryCanceled:
            res = aItem.GetNextString( connectionOptionName );   
            if ( res == KErrNone )
                {
                if ( !connectionOptionName.Compare( _L( "DISCARDCONN" ) ) )
                    {
                    res = aItem.GetNextString( deviceBlockOptionName );
                    if ( res != KErrNone )
                        {
                        iLog -> Log( _L("GetNextString failed with value: %d"), res );
                        iNotifier -> Close();
                        return res;
                        }
                    
                    if ( !deviceBlockOptionName.Compare( _L( "BLOCK" ) ) )
                        {
                        iLog -> Log( _L("Block test device") );
                        iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrAccessDenied );
                        STIF_ASSERT_EQUALS( KErrAccessDenied, iReqStatus.Int() );
                        }
                    else if ( !deviceBlockOptionName.Compare( _L( "NONBLOCK" ) ) )
                        {
                        iLog -> Log( _L("Don't block test device") );
                        iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
                        STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
                        }
                    else
                        {
                        iLog -> Log( _L("Block connection option unavailable") );
                        return KErrNotFound;
                        }                    
                    }
                else if ( !connectionOptionName.Compare( _L( "ACCEPTCONN" ) ) )
                    {
                    iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
                    STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
                    }      
                else 
                    {
                    iLog -> Log( _L("Connection option unavailable") );
                    return KErrNotFound;
                    }
                }
            else if ( res == KErrNotFound )
                {
                iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
                STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
                }
            else 
                return res;
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::InquiryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::InquiryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC initOptionName( KNullDesC );
    TBTRegistrySearch regSearch;
    TBTPowerStateValue btPowerState;
    TBTDeviceSelectionParamsPckg emptyNotifierParamsPckg;
        
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    res = iBtSettings -> GetPowerState( btPowerState );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to get BT power state with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    if ( btPowerState == EBTPowerOn )
        {
        res = iBtSettings -> SetPowerState( EBTPowerOff );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Failed to set BT off with value: %d"), res );
            iNotifier -> Close();
            return res;
            } 
        }
    
    regSearch.FindAll();    
    res = iBtEngDevManObserver -> GetDevices( regSearch, iBtDeviceArray );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Getting devices from register failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }

    if ( iBtDeviceArray -> Count() > 0 )
        {
        iBtDeviceArray -> Reset();
        res = iBtEngDevManObserver -> DeleteDevices( regSearch );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Deleting devices from register failed with value: %d"), res );
            iNotifier -> Close();
            return res;
            }
        }
    
    res = aItem.GetString( _L( "InquiryNotifierTest" ), initOptionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    if ( !initOptionName.Compare( _L( "BTOFF" ) ) )
        {    
        iLog -> Log( _L("Init test option: bt off") );
        }
    else if ( !initOptionName.Compare( _L( "PAIRED" ) ) )
        {
        iLog -> Log( _L("Init test option: paired test device") );
        iTestDevice -> SetDeviceAddress( 0x12345678 );
        iTestDevice -> SetPaired( ELinkKeyAuthenticated );
        TBTDeviceClass testDeviceClass( EMajorServiceLimitedDiscoverableMode, EMajorDeviceComputer, EMinorDeviceComputerUnclassified );
        iTestDevice -> SetDeviceClass( testDeviceClass );
        iTestDevice -> SetDeviceNameL( _L8( "TestedDevice" ) );
        
        res = iBtEngDevManObserver -> AddDevice( *iTestDevice );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Adding device to register failed with value: %d"), res );
            iNotifier -> Close();
            return res;
            }
        }
    else if ( !initOptionName.Compare( _L( "NONPAIRED" ) ) )
        {
        iLog -> Log( _L("Init test option: test device not paired") );
        }
    else
        {
        iLog -> Log( _L("Option unavailable") );
        iNotifier -> Close();
        return KErrNotFound;
        }
    
    if ( initOptionName.Compare( _L( "BTOFF" ) ) )
        {
        res = iBtSettings -> SetPowerState( EBTPowerOn );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Failed to set BT on with value: %d"), res );
            iNotifier -> Close();
            return res;
            } 
        }

    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KDeviceSelectionNotifierUid, emptyNotifierParamsPckg, iBtDeviceResponseParamsPckg );
    iLog -> Log( _L("Notifier started") );
    
    User::After( 5000000 );

    return KErrNone;
    }


TInt CBtNotifApiTest::FinishInquiry( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TPtrC subOptionName( KNullDesC );
    TBTPowerStateValue btPowerState;
    TTestOption option;
    
    res = aItem.GetString( _L( "FinishInquiry" ), optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    res = aItem.GetNextString( subOptionName );   
    iLog -> Log( _L("GetString  value: %d"), res );
    
    if ( !subOptionName.Compare( _L( "BTOFF" ) ) )
        {
        iLog -> Log( _L("Test suboption: set bt off") );
        iBtSettings -> SetPowerState( EBTPowerOff );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Failed to set BT off with value: %d"), res );
            iNotifier -> Close();
            return res;
            } 
        }
   
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    User::WaitForRequest( iReqStatus );
    
    switch( option )
        {
        case EQueryAccepted:
            {
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            
            HBufC16* inputDeviceNameBuf = HBufC16::New( iTestDevice -> DeviceName().Length() );
            TPtr16 inputDeviceName = inputDeviceNameBuf -> Des();
            inputDeviceName.Copy( iTestDevice -> DeviceName() );
            
            HBufC16* outputDeviceNameBuf = HBufC16::New( iBtDeviceResponseParamsPckg().DeviceName().Length() );
            TPtr16 outputDeviceName = outputDeviceNameBuf -> Des();
            outputDeviceName.Copy( iBtDeviceResponseParamsPckg().DeviceName() );
              
            iLog -> Log( _L( "Equal device addresses expected" ) );
            STIF_ASSERT_EQUALS( iTestDevice -> BDAddr(), iBtDeviceResponseParamsPckg().BDAddr() );
            iLog -> Log( _L( "Equal device names expected" ) );
            STIF_ASSERT_EQUALS( inputDeviceName, outputDeviceName );
            iLog -> Log( _L( "Equal device classes expected" ) );
            STIF_ASSERT_EQUALS( iTestDevice -> DeviceClass(), iBtDeviceResponseParamsPckg().DeviceClass() );
            
            delete inputDeviceNameBuf;
            delete outputDeviceNameBuf;
            }
            break;
        case EQueryCanceled:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
            STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }
    
    res = iBtSettings -> GetPowerState( btPowerState );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to get BT power state with value: %d"), res );
        iNotifier -> Close();
        return res;
        }
    
    if ( btPowerState == EBTPowerOn )
        {
        res = iBtSettings -> SetPowerState( EBTPowerOff );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Failed to set BT off with value: %d"), res );
            iNotifier -> Close();
            return res;
            } 
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::ObexPasskeyQueryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::ObexPasskeyQueryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TUint8 passCode[] = { "0000" };
    TBTObexPasskey passKey = passCode;
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    iObexPasskeyBuf = passKey;  
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTObexPasskeyQueryNotifierUid, iObexPasskeyBuf, iEnteredPasskeyBuf );
    iLog -> Log( _L("Notifier started") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishObexPasskeyQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TTestOption option;
    
    res = aItem.GetNextString( optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetNextString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    User::WaitForRequest( iReqStatus );    
    switch( option )
        {
        case ECheckPasskey:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Equal passkeys expected") );
            STIF_ASSERT_EQUALS( iObexPasskeyBuf(), iEnteredPasskeyBuf() );
            break;
        case EQueryCanceled:
            iLog -> Log( _L("Request status value: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
            STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::PowerModeQueryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::PowerModeQueryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TBTPowerStateValue state;
    TBuf<KMaxBCBluetoothNameLen> testDeviceName = _L( "Device Test Name" );
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    
    res = iBtSettings -> GetPowerState( state );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to get bt power state with value: %d"), res );
        return res;
        }
    
    if ( state != EBTPowerOff )
        {
        res = iBtSettings -> SetPowerState( EBTPowerOff );
        if ( res != KErrNone )
            {
            iLog -> Log( _L("Failed to set bt power state with value: %d"), res );
            return res;
            }
        }
    
    res = iBtSettings -> GetLocalName( iLocalDeviceName );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed backup device name: %d"), res );
        return res;
        }
    
    res = iBtSettings -> SetLocalName( testDeviceName );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed set device name: %d"), res );
        return res;
        }
    
    TPckgBuf<TBool> emptyInput;   
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KPowerModeSettingNotifierUid, emptyInput, iPowerModeQueryReply );
    iLog -> Log( _L("Notifier started") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishPowerModeQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TTestOption option;    
    
    res = aItem.GetNextString( optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetNextString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }    
    
    User::WaitForRequest( iReqStatus );
    
    if ( iReqStatus.Int()!= KErrNone && iReqStatus.Int()!= KErrCancel )
        {
        iLog -> Log( _L("Failed to get notifier's response: %d"), iReqStatus.Int() );
        iNotifier -> Close();
        return iReqStatus.Int();
        }
    iLog -> Log( _L("Notifier responded") );
    
    iBtSettings -> SetLocalName( iLocalDeviceName );
    
    switch ( option )
        {
        case EQueryDiscarded:
            iLog -> Log( _L("Returned request status: %d, expected: %d"), iReqStatus.Int(), KErrCancel );
            STIF_ASSERT_EQUALS( KErrCancel, iReqStatus.Int() );
            iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iPowerModeQueryReply(), EFalse );  
            STIF_ASSERT_FALSE( iPowerModeQueryReply() );
            break;
        case EQueryAccepted:
            res = iBtSettings -> SetPowerState( EBTPowerOff );
            if ( res != KErrNone )
                {
                iLog -> Log( _L("Failed to set bt power state off with value: %d"), res );
                return res;
                }
            iLog -> Log( _L("Returned request status: %d, expected: %d"), iReqStatus.Int(), KErrNone );
            STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
            iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iPowerModeQueryReply(), ETrue );
            STIF_ASSERT_TRUE( iPowerModeQueryReply() );
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }
  
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::GenericInfoNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::GenericInfoNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TBTGenericInfoNotifierParams notifierParams;
    TBTGenericInfoNoteType genericInfoNoteType;
    TPtrC genericInfoNoteTypeString( KNullDesC );
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    res = aItem.GetNextString( genericInfoNoteTypeString );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetNextString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    res = GetGenericInfoNoteType( genericInfoNoteTypeString, genericInfoNoteType );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Generic info note type not found") );
        iNotifier -> Close();
        return res;
        }
    
    notifierParams.iMessageType = genericInfoNoteType;
    notifierParams.iRemoteAddr = _L8( "012345" );
    TBTGenericInfoNotiferParamsPckg messageInfo( notifierParams );
    TPckgBuf<TBool> emptyReply;
    
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTGenericInfoNotifierUid, messageInfo, emptyReply );
    iLog -> Log( _L("Notifier started") );
    User::WaitForRequest( iReqStatus );
    iLog -> Log( _L("Returned request status: %d, expected: %d"), iReqStatus.Int(), KErrNone );
    STIF_ASSERT_EQUALS( KErrNone, iReqStatus.Int() );
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    
    return KErrNone;
    }

TInt CBtNotifApiTest::GetGenericInfoNoteType( TPtrC aTypeString, TBTGenericInfoNoteType& aGenericInfoNoteType )
    {
    if ( !aTypeString.Compare( _L( "EBTConnected" ) ) )
        {
        aGenericInfoNoteType = EBTConnected;
        iLog -> Log( _L("Generic info message type: EBTConnected") );
        }
    else if ( !aTypeString.Compare( _L( "EBTDisconnected" ) ) )
        {
        aGenericInfoNoteType = EBTDisconnected;
        iLog -> Log( _L("Generic info message type: EBTDisconnected") );
        }
    else if ( !aTypeString.Compare( _L( "EBTAudioAccessory" ) ) )
        {
        aGenericInfoNoteType = EBTAudioAccessory;
        iLog -> Log( _L("Generic info message type: EBTAudioAccessory") );
        }
    else if ( !aTypeString.Compare( _L( "EBTAudioHandset" ) ) )
        {
        aGenericInfoNoteType = EBTAudioHandset;
        iLog -> Log( _L("Generic info message type: EBTAudioHandset") );
        }
    else if ( !aTypeString.Compare( _L( "EBTClosed" ) ) )
        {
        aGenericInfoNoteType = EBTClosed;
        iLog -> Log( _L("Generic info message type: EBTClosed") );
        }
    else if ( !aTypeString.Compare( _L( "EBTDeviceNotAvailable" ) ) )
        {
        aGenericInfoNoteType = EBTDeviceNotAvailable;
        iLog -> Log( _L("Generic info message type: EBTDeviceNotAvailable") );
        }
    else if ( !aTypeString.Compare( _L( "EBTOfflineDisabled" ) ) )
        {
        aGenericInfoNoteType = EBTOfflineDisabled;
        iLog -> Log( _L("Generic info message type: EBTOfflineDisabled") );
        }
    else if ( !aTypeString.Compare( _L( "EBTVisibilityTimeout" ) ) )
        {
        aGenericInfoNoteType = EBTVisibilityTimeout;
        iLog -> Log( _L("Generic info message type: EBTVisibilityTimeout") );
        }
    else if ( !aTypeString.Compare( _L( "EBTEnterSap" ) ) )
        {
        aGenericInfoNoteType = EBTEnterSap;
        iLog -> Log( _L("Generic info message type: EBTEnterSap") );
        }
    else if ( !aTypeString.Compare( _L( "EBTSapOk" ) ) )
        {
        aGenericInfoNoteType = EBTSapOk;
        iLog -> Log( _L("Generic info message type: EBTSapOk") );
        }
    else if ( !aTypeString.Compare( _L( "EBTSapFailed" ) ) )
        {
        aGenericInfoNoteType = EBTSapFailed;
        iLog -> Log( _L("Generic info message type: EBTSapFailed") );
        }
    else if ( !aTypeString.Compare( _L( "EBTSapNoSim" ) ) )
        {
        aGenericInfoNoteType = EBTSapNoSim;
        iLog -> Log( _L("Generic info message type: EBTSapNoSim") );
        }
    else if ( !aTypeString.Compare( _L( "EBTDeviceBusy" ) ) )
        {
        aGenericInfoNoteType = EBTDeviceBusy;
        iLog -> Log( _L("Generic info message type: EBTDeviceBusy") );
        }
    else if ( !aTypeString.Compare( _L( "EIRNotSupported" ) ) )
        {
        aGenericInfoNoteType = EIRNotSupported;
        iLog -> Log( _L("Generic info message type: EIRNotSupported") );
        }
    else if ( !aTypeString.Compare( _L( "ECmdShowBtBatteryLow" ) ) )
        {
        aGenericInfoNoteType = ECmdShowBtBatteryLow;
        iLog -> Log( _L("Generic info message type: ECmdShowBtBatteryLow") );
        }
    else if ( !aTypeString.Compare( _L( "ECmdShowBtBatteryCritical" ) ) )
        {
        aGenericInfoNoteType = ECmdShowBtBatteryCritical;
        iLog -> Log( _L("Generic info message type: ECmdShowBtBatteryCritical") );
        }
    else if ( !aTypeString.Compare( _L( "EBTStayPowerOn" ) ) )
        {
        aGenericInfoNoteType = EBTStayPowerOn;
        iLog -> Log( _L("Generic info message type: EBTStayPowerOn") );
        }
    else
        {
        iLog -> Log( _L("Generic info message type: No type matched") );
        return KErrNotFound;
        }
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtNotifApiTest::GenericQueryNotifierTests
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::GenericQueryNotifierTest( CStifItemParser& aItem )
    {
    TInt res;
    TBTGenericQueryNotifierParams notifierParams;
    TBTGenericQueryNoteType genericQueryNoteType;
    TPtrC genericQueryNoteTypeString( KNullDesC );
    
    res = iNotifier -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Failed to connect RNotifier with value: %d"), res );
        return res;
        }   
    iLog -> Log( _L("RNotifier connected") );
    
    res = aItem.GetNextString( genericQueryNoteTypeString );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetNextString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        } 
    
    res = GetGenericQueryNoteType( genericQueryNoteTypeString, genericQueryNoteType );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("Generic query note type not found") );
        iNotifier -> Close();
        return res;
        }
    
    notifierParams.iMessageType = genericQueryNoteType;
    notifierParams.iNameExists = EFalse;   
    TBTGenericQueryNotiferParamsPckg messageInfo( notifierParams );   
    iNotifier -> StartNotifierAndGetResponse( iReqStatus, KBTGenericQueryNotifierUid, messageInfo, iGenericQueryReply );
    iLog -> Log( _L("Notifier started") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::FinishGenericQuery( CStifItemParser& aItem )
    {
    TInt res;
    TPtrC optionName( KNullDesC );
    TTestOption option;
    
    res = aItem.GetNextString( optionName );   
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetNextString failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }  
    
    res = GetTestOption( optionName, option );
    if ( res != KErrNone )
        {
        iLog -> Log( _L("GetTestOption failed with value: %d"), res );
        iNotifier -> Close();
        return res;
        }   
    
    User::WaitForRequest( iReqStatus );
    if ( iReqStatus.Int()!= KErrNone )
        {
        iLog -> Log( _L("Failed to get notifier's response: %d"), iReqStatus.Int() );
        iNotifier -> Close();
        return iReqStatus.Int();
        }
    iLog -> Log( _L("Notifier responded") );
    
    switch ( option )
        {
        case EQueryDiscarded:
            iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iGenericQueryReply(), EFalse );  
            STIF_ASSERT_FALSE( iGenericQueryReply() );
            break;
        case EQueryAccepted:
            iLog -> Log( _L("Notifier's response value: %b, expected: %b"), iGenericQueryReply(), ETrue );
            STIF_ASSERT_TRUE( iGenericQueryReply() );
            break;
        default:
            iLog -> Log( _L("Option unavailable") );
            return KErrNotFound;
        }
    
    iNotifier -> Close();
    iLog -> Log( _L("Test case passed") );
    
    return KErrNone;
    }


TInt CBtNotifApiTest::GetGenericQueryNoteType( TPtrC aTypeString, TBTGenericQueryNoteType& aGenericQueryNoteType )
    {   
    if ( !aTypeString.Compare( _L( "EBTReceiveMessageQuery" ) ) )
        {
        aGenericQueryNoteType = EBTReceiveMessageQuery;
        iLog -> Log( _L("Generic query message type: EBTReceiveMessageQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTReceiveMessagePairedQuery" ) ) )
        {
        aGenericQueryNoteType = EBTReceiveMessagePairedQuery;
        iLog -> Log( _L("Generic query message type: EBTReceiveMessagePairedQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTIsOffQuery" ) ) )
        {
        aGenericQueryNoteType = EBTIsOffQuery;
        iLog -> Log( _L("Generic query message type: EBTIsOffQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTActivateOffLineQuery" ) ) )
        {
        aGenericQueryNoteType = EBTActivateOffLineQuery;
        iLog -> Log( _L("Generic query message type: EBTActivateOffLineQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTNoDevicesFoundQuery" ) ) )
        {
        aGenericQueryNoteType = EBTNoDevicesFoundQuery;
        iLog -> Log( _L("Generic query message type: EBTNoDevicesFoundQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTAcceptRequestQuery" ) ) )
        {
        aGenericQueryNoteType = EBTAcceptRequestQuery;
        iLog -> Log( _L("Generic query message type: EBTAcceptRequestQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTBlockConnectionQuery" ) ) )
        {
        aGenericQueryNoteType = EBTBlockConnectionQuery;
        iLog -> Log( _L("Generic query message type: EBTBlockConnectionQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTBlockPairedConnectionQuery" ) ) )
        {
        aGenericQueryNoteType = EBTBlockPairedConnectionQuery;
        iLog -> Log( _L("Generic query message type: EBTBlockPairedConnectionQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTIsNotShownQuery" ) ) )
        {
        aGenericQueryNoteType = EBTIsNotShownQuery;
        iLog -> Log( _L("Generic query message type: EBTIsNotShownQuery") );
        }
    else if ( !aTypeString.Compare( _L( "EBTIsOffJavaQuery" ) ) )
        {
        aGenericQueryNoteType = EBTIsOffJavaQuery;
        iLog -> Log( _L("Generic query message type: EBTIsOffJavaQuery") );
        }
    else
        {
        iLog -> Log( _L("Generic query message type: No type matched") );
        return KErrNotFound;
        }
    
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CBtNotifApiTest::GetTestOption
// -----------------------------------------------------------------------------
//

TInt CBtNotifApiTest::GetTestOption( TPtrC aOptionString, TTestOption& aOption )
    {    
    if ( !aOptionString.Compare( _L( "DISCARD" ) ) )
        {
        aOption = EQueryDiscarded;
        iLog -> Log( _L("Test type: EQueryDiscarded") );
        }
    else if ( !aOptionString.Compare( _L( "ACCEPT" ) ) )
        {
        aOption = EQueryAccepted;
        iLog -> Log( _L("Test type: EQueryAccepted") );
        }
    else if ( !aOptionString.Compare( _L( "PASSKEY" ) ) )
        {
        aOption = ECheckPasskey;
        iLog -> Log( _L("Test type: ECheckPasskey") );
        }
    else if ( !aOptionString.Compare( _L( "CANCEL" ) ) )
        {
        aOption = EQueryCanceled;
        iLog -> Log( _L("Test type: EQueryCanceled") );
        }
    else
        return KErrNotFound;  
 
    return KErrNone;
    }

TInt CBtNotifApiTest::TurnLightsOn( CStifItemParser& aItem )
    {
    TInt res;
    res = RProperty::Set(KPSUidCoreApplicationUIs, KLightsControl, ELightsOn);
    return res;
    }


// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
