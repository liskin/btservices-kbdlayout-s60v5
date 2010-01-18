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




#ifndef BTAUDIOSTREAMERAPITEST_H
#define BTAUDIOSTREAMERAPITEST_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <TestclassAssert.h>
#include <SbcEncoderIntfc.h>
#include <btserversdkcrkeys.h> 

#include "bt_sock.h"
#include "btaudiostreamer.h"
#include "btaudiostreaminputbase.h" 
#include "BtAudioStreamerObserver.h"

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

// Logging path
//_LIT( KBtAudioStreamerApiTestLogPath, "\\logs\\testframework\\bt\\" );
_LIT( KBtAudioStreamerApiTestLogPath, "e:\\testing\\stiflogs\\" );
// Log file
_LIT( KBtAudioStreamerApiTestLogFile, "BtAudioStreamerApiTest.txt" ); 
_LIT( KBtAudioStreamerApiTestLogFileWithTitle, "BtAudioStreamerApiTest_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CBtAudioStreamerApiTest;

// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CBtAudioStreamerApiTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CBtAudioStreamerApiTest) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBtAudioStreamerApiTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CBtAudioStreamerApiTest();

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
        CBtAudioStreamerApiTest( CTestModuleIf& aTestModuleIf );

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
        virtual TInt InitializationTest( CStifItemParser& aItem );
        virtual TInt StartStopTest( CStifItemParser& aItem );
        virtual TInt SetNewFrameLengthTest( CStifItemParser& aItem );
        
        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove
        
        TInt ConfigureEncoderInterface();

    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data
        
        CBTAudioStreamer* iBtAudioStreamer;
        CBTAudioStreamInputBase* iBtAudioStreamInputBase;
        CBtAudioStreamerObserver* iBtAudioStreamerObserver;
        CSbcEncoderIntfc* iSbcEncoderIntfc;
        
        RSocketServ iSocketServer;
        RSocket iSocket;

    };

#endif      // BTAUDIOSTREAMERAPITEST_H

// End of File
