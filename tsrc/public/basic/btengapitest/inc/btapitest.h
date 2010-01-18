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



#ifndef BTAPITEST_H
#define BTAPITEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include "testobserver.h"


// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
// Logging path
_LIT( KBtApiTestLogPath, "\\logs\\btengapitest\\" ); 
// Log file
_LIT( KBtApiTestLogFile, "btengapitest.txt" ); 

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
class CBtTestLogger;
class CBTApiSettings;
class CBTApiDevman;
class CBTApiConnman;
class CBTApiDiscovery;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CCBtApiTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CBtApiTest) : public CScriptBase, public MBTTestObserver
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBtApiTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CBtApiTest();

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

    private: // From MBTTestObserver
    
        void TestCompleted( TInt aErr, const TUint8* aFunc, const TDesC& aArg );

        CBtTestLogger& Logger();

    private:

        /**
        * C++ default constructor.
        */
        CBtApiTest( CTestModuleIf& aTestModuleIf );

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
        * Test methods.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt CBTEngSettings_SetAndGetPowerState ( CStifItemParser& aItem );
        virtual TInt CBTEngSettings_SetAndGetVisibility ( CStifItemParser& aItem );
        virtual TInt CBTEngSettings_SetAndGetLocalName ( CStifItemParser& aItem );
        virtual TInt CBTEngDevMan_AddDevices ( CStifItemParser& aItem );
        virtual TInt CBTEngDevMan_GetDevices ( CStifItemParser& aItem );
        virtual TInt CBTEngDevMan_ModifyDevice ( CStifItemParser& aItem );
        virtual TInt CBTEngDiscovery_SearchRemoteDevice ( CStifItemParser& aItem );
        virtual TInt CBTEngDiscovery_RegisterAndDeleteSdpRecord ( CStifItemParser& aItem );
        virtual TInt CBTEngDiscovery_RemoteSdpQuery ( CStifItemParser& aItem );
        virtual TInt CBTEngDiscovery_ParseSdpAttrValues ( CStifItemParser& aItem );
        virtual TInt CBTEngConnMan_ConnectDisconnectAndCancelConnect ( CStifItemParser& aItem );
        virtual TInt CBTEngConnMan_GetAddresses ( CStifItemParser& aItem );
        virtual TInt CBTEngConnMan_IsConnectable ( CStifItemParser& aItem );
        virtual TInt CBTEngConnMan_PairDevice ( CStifItemParser& aItem );
        virtual TInt CBTEngConnMan_PrepareDiscovery ( CStifItemParser& aItem );
        
        
    private:    // Data
    
    CBTApiSettings* iSettings;
    CBTApiDevman*   iDevMan;
    CBTApiConnman* iConnman;
    CBTApiDiscovery* iDiscovery;
    CBtTestLogger*  iTestLogger;

    };

#endif      // BTAPITEST_H

// End of File
