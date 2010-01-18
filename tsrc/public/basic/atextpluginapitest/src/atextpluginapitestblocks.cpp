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

#include "atextpluginapitest.h"


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
#define TLFUNCLOG (TUint8*) __FUNCTION__

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
// CAtExtPluginApiTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CAtExtPluginApiTest::Delete() 
    {
    }

// -----------------------------------------------------------------------------
// CAtExtPluginApiTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CAtExtPluginApiTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {
        ENTRY( "TestPluginStub", TestPluginStub ),
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );
    }

TInt CAtExtPluginApiTest::SendUnsolicitedResult(CATExtPluginBase& aPlugin, const TDesC8& aAT)
    {
    iLogger->Log( CBtTestLogger::ETLInfo, _L8( "SendUnsolicitedResult %S" ),  &aAT);
    return KErrNone;
    }

void CAtExtPluginApiTest::HandleCommandCompleted(CATExtPluginBase& aPlugin, TInt aErr)
    {
    iLogger->Log( CBtTestLogger::ETLInfo, _L8( "HandleCommandCompleted %d" ),  aErr);
    }

void CAtExtPluginApiTest::ATExtPluginClosed(CATExtPluginBase& aPlugin)
    {
    iPlugin = NULL;
    iLogger->LogResult( (TPtrC8( TLFUNCLOG )), KNullDesC, KErrNone );
    Signal(KErrNone);
    }

TInt CAtExtPluginApiTest::TestPluginStub( CStifItemParser& /*aItem*/ )
    {
    iLogger->Log( CBtTestLogger::ETLInfo, _L8( "TestPluginStub" ));
    TInt ret(KErrNone);
    TRAP(ret, LoadPluginStubL());
    if (ret)
        {
        iLogger->LogResult( (TPtrC8( TLFUNCLOG )), KNullDesC, ret );
        Signal(ret);
        }
    else
        {
        iPlugin->SetObserver(*this);
        _LIT8(KTestCmd, "ApiTest");
        iPlugin->HandleCommand(KTestCmd, iCmdBuf);
        iPlugin->HandleCommandCancel();
        }
    return KErrNone;
    }

template <class T>
class CleanupResetDestroyClose
    {
public:
    inline static void PushL(T& aRef) 
        {
        CleanupStack::PushL(TCleanupItem(&ResetDestroyClose,&aRef));
        }
private:
    static void ResetDestroyClose(TAny *aPtr)
        {
        static_cast<T*>(aPtr)->ResetAndDestroy();
        static_cast<T*>(aPtr)->Close();
        }
    };

/**
 * Pushes an object into CleanupStack and specifies the cleanup 
 * function as ResetAndDestroy() and Close().
*/
template <class T>
inline void CleanupResetDestroyClosePushL(T& aRef)
    {CleanupResetDestroyClose<T>::PushL(aRef);}

void CAtExtPluginApiTest::LoadPluginStubL()
    {
    delete iPlugin;
    iPlugin = NULL;
    const TUid KUidPluginInterface = TUid::Uid(0x2000B17B);
    RImplInfoPtrArray implementations;
    REComSession::ListImplementationsL(KUidPluginInterface,
        implementations);
    CleanupResetDestroyClosePushL(implementations);
    const TUint count = implementations.Count();
    
    TInt ret;
    for ( TUint ii = 0 ; ii < count ; ++ii )
        {
        if (TUid::Uid(0x2000B185) == implementations[ii]->ImplementationUid())
            {
            iPlugin = CATExtPluginBase::NewL(implementations[ii]->ImplementationUid(), *this);
            break;
            }
        }
    CleanupStack::PopAndDestroy(&implementations);
    if (!iPlugin)
        {
        User::Leave(KErrNotFound);
        }
    }
    
// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  End of File
