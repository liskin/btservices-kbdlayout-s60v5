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
#include <Stiftestinterface.h>
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
// CBtApiTest::CCBtApiTest
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBtApiTest::CBtApiTest( 
    CTestModuleIf& aTestModuleIf ):
        CScriptBase( aTestModuleIf )
    {
    }

// -----------------------------------------------------------------------------
// CBtApiTest::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBtApiTest::ConstructL()
    {
    iTestLogger = CBtTestLogger::NewL( &TestModuleIf() );     
    iLog = iTestLogger->GetLogger();
    iSettings = CBTApiSettings::NewL( *this );
    iDevMan = CBTApiDevman::NewL( *this );
    iConnman = CBTApiConnman::NewL( *this );
    iDiscovery = CBTApiDiscovery::NewL( *this );
    }

// -----------------------------------------------------------------------------
// CBtApiTest::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBtApiTest* CBtApiTest::NewL( 
    CTestModuleIf& aTestModuleIf )
    {
    CBtApiTest* self = new (ELeave) CBtApiTest( aTestModuleIf );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }

// Destructor
CBtApiTest::~CBtApiTest()
    { 
    // Delete resources allocated from test methods
    Delete();
    
    delete iSettings;
    delete iDevMan;
    delete iConnman;
    delete iDiscovery;
    delete iTestLogger;
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

// -----------------------------------------------------------------------------
// LibEntryL is a polymorphic Dll entry point.
// Returns: CScriptBase: New CScriptBase derived object
// -----------------------------------------------------------------------------
//
EXPORT_C CScriptBase* LibEntryL( 
    CTestModuleIf& aTestModuleIf ) // Backpointer to STIF Test Framework
    {

    return ( CScriptBase* ) CBtApiTest::NewL( aTestModuleIf );

    }


//  End of File
