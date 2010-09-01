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




#ifndef BTNOTIFAPITEST_H
#define BTNOTIFAPITEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <TestclassAssert.h>
#include <btextnotifierspartner.h>

#include "btnotif.h"
#include "btmanclient.h"
#include "btnotifierapi.h"
#include "btextnotifiers.h"
#include "coreapplicationuisdomainpskeys.h"

#include "BtEngDevManObserver.h"
#include "BtSettingsObserver.h"


// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

// Logging path
//_LIT( KBtNotifApiTestLogPath, "\\logs\\testframework\\bt\\" );
_LIT( KBtNotifApiTestLogPath, "e:\\testing\\stiflogs\\" );
// Log file
_LIT( KBtNotifApiTestLogFile, "BtNotifApiTest.txt" ); 
_LIT( KBtNotifApiTestLogFileWithTitle, "BtNotifApiTest_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CBtNotifApiTest;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

enum TTestOption
    {
    EQueryDiscarded = 0,
    EQueryAccepted,
    ECheckPasskey,
    EQueryCanceled
    };

// CLASS DECLARATION

/**
*  CBtNotifApiTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CBtNotifApiTest) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBtNotifApiTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CBtNotifApiTest();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

        /**
        * From ?base_class ?member_description
        */
        //?type ?member_function();

    private:

        /**
        * C++ default constructor.
        */
        CBtNotifApiTest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below. 
        */

        /**
        * Example test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt PairedDeviceSettingNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishPairedDeviceSettingQuery( CStifItemParser& aItem );
        
        virtual TInt PbapAuthNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishPbapAuthQuery( CStifItemParser& aItem );
        
        virtual TInt PasskeyDisplayNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishPasskeyDisplayQuery( CStifItemParser& aItem );
        
        virtual TInt NumericComparisonQueryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishNumericComparisonQuery( CStifItemParser& aItem );
        
        virtual TInt AuthQueryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishAuthQuery( CStifItemParser& aItem );
        
        virtual TInt PinPasskeyQueryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishPinPasskeyQuery( CStifItemParser& aItem );
        
        virtual TInt InquiryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishInquiry( CStifItemParser& aItem );
        
        virtual TInt ObexPasskeyQueryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishObexPasskeyQuery( CStifItemParser& aItem );
        
        virtual TInt PowerModeQueryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishPowerModeQuery( CStifItemParser& aItem );
        
        virtual TInt GenericInfoNotifierTest( CStifItemParser& aItem );
        
        virtual TInt GenericQueryNotifierTest( CStifItemParser& aItem );
        virtual TInt FinishGenericQuery( CStifItemParser& aItem );
        
        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();
        
        TInt GetGenericInfoNoteType( TPtrC aTypeString, TBTGenericInfoNoteType& aGenericInfoNoteType );        
        TInt GetGenericQueryNoteType( TPtrC aTypeString, TBTGenericQueryNoteType& aGenericQueryNoteType );
        
        TInt GetTestOption( TPtrC optionString, TTestOption& aOption );
        TInt TurnLightsOn( CStifItemParser& aItem );

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove

    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data
        
        RNotifier* iNotifier;
              
        TPbapAuthNotifierResponsePckg iPbapAuthNotifierResponsePckg;
        TBTDeviceResponseParamsPckg iBtDeviceResponseParamsPckg;
        TPckgBuf<TInt> iNumericComparisonQueryReply;
        TBTPinCode iEnteredPinPasskey;
        TBTPinCode iPinPasskey;
        TObexPasskeyBuf iObexPasskeyBuf;
        TObexPasskeyBuf iEnteredPasskeyBuf;
        TPckgBuf<TBool> iAuthQueryReply;
        TPckgBuf<TBool> iPowerModeQueryReply;
        TPckgBuf<TBool> iGenericQueryReply;
        
        
        TRequestStatus iReqStatus;
        TBuf<KMaxBCBluetoothNameLen> iLocalDeviceName;
        
        CBtSettingsObserver* iBtSettings;
        CBtEngDevManObserver* iBtEngDevManObserver;
        CBTDeviceArray* iBtDeviceArray;
        CBTDevice* iTestDevice;
        

    public:     // Friend classes
        //?friend_class_declaration;
    protected:  // Friend classes
        //?friend_class_declaration;
    private:    // Friend classes
        //?friend_class_declaration;

    };

#endif      // BTNOTIFAPITEST_H

// End of File
