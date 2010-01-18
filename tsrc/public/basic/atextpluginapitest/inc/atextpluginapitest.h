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
#include <atextpluginbase.h>
#include "atextpluginobserver.h"
#include "bttestlogger.h"

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
// Logging path
// Log file

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CCAtExtPluginApiTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CAtExtPluginApiTest) : public CScriptBase, public MATExtPluginObserver
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CAtExtPluginApiTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CAtExtPluginApiTest();

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

    private: // From MATExtPluginObserver
    
        TInt SendUnsolicitedResult(CATExtPluginBase& aPlugin, const TDesC8& aAT);

        void HandleCommandCompleted(CATExtPluginBase& aPlugin, TInt aErr);

        void ATExtPluginClosed(CATExtPluginBase& aPlugin);

    private:

        /**
        * C++ default constructor.
        */
        CAtExtPluginApiTest( CTestModuleIf& aTestModuleIf );

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
        TInt TestPluginStub ( CStifItemParser& aItem );
        
        void LoadPluginStubL();
        
    private:    // Data
    
        CATExtPluginBase* iPlugin;
        RBuf8 iCmdBuf;
        CBtTestLogger* iLogger;
    };

#endif      // BTAPITEST_H

// End of File
