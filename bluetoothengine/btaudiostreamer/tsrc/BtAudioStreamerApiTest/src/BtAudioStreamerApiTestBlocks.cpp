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
#include "BtAudioStreamerApiTest.h"


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
// CBtAudioStreamerApiTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CBtAudioStreamerApiTest::Delete() 
    {      
        delete iBtAudioStreamInputBase;
        iBtAudioStreamInputBase = NULL;
        delete iBtAudioStreamerObserver;
        iBtAudioStreamerObserver = NULL;
        delete iBtAudioStreamer;
        iBtAudioStreamer = NULL;
    }

// -----------------------------------------------------------------------------
// CBtAudioStreamerApiTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CBtAudioStreamerApiTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "InitializationTest", CBtAudioStreamerApiTest::InitializationTest ),
        ENTRY( "StartStopTest", CBtAudioStreamerApiTest::StartStopTest ),
        ENTRY( "SetNewFrameLengthTest", CBtAudioStreamerApiTest::SetNewFrameLengthTest ),
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove

        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CBtAudioStreamerApiTest::InitializationTest
// -----------------------------------------------------------------------------
//
TInt CBtAudioStreamerApiTest::InitializationTest( CStifItemParser& aItem )
    {
    
    _LIT(KL2CAPDesC,"L2CAP");
    TInt res;
    TProtocolDesc protocolDesc;
    TProtocolName protocolName( KL2CAPDesC );
    
    iLog -> Log( _L( "BtAudioStreamer pointer value: %d" ), iBtAudioStreamer );
    STIF_ASSERT_NOT_NULL( iBtAudioStreamer );
    iLog -> Log( _L( "BtAudioStreamer pointer value: %d" ), iBtAudioStreamInputBase );
    STIF_ASSERT_NOT_NULL( iBtAudioStreamInputBase );
    
    res = iSocketServer.Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Server connection failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Connected to socket server" ) );
    
    res = iSocketServer.FindProtocol( TProtocolName( KL2CAPDesC ), protocolDesc );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Searching protocol failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Protocol info found" ) );
    
    res = iSocket.Open( iSocketServer, protocolDesc.iAddrFamily, protocolDesc.iSockType, protocolDesc.iProtocol );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Socket opening failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Socket opened" ) );
    
    iSocket.Close();
    iSocketServer.Close();
    iLog -> Log( _L( "Test case passed" ) );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtAudioStreamerApiTest::StartStopTest
// -----------------------------------------------------------------------------
//

TInt CBtAudioStreamerApiTest::StartStopTest( CStifItemParser& aItem )
    {
    _LIT(KL2CAPDesC,"L2CAP");
    TInt res;
    TProtocolDesc protocolDesc;
    TProtocolName protocolName( KL2CAPDesC );
    
    iBtAudioStreamer = CBTAudioStreamer::NewL();
    iLog -> Log( _L( "BtAudioStreamer pointer value: %d" ), iBtAudioStreamer );
    STIF_ASSERT_NOT_NULL( iBtAudioStreamer );
    
    iBtAudioStreamerObserver = CBtAudioStreamerObserver::NewL( iLog );
    
    iBtAudioStreamInputBase = CBTAudioStreamInputBase::NewL( *iBtAudioStreamer, *iBtAudioStreamerObserver ); 
    iLog -> Log( _L( "BtAudioStreamer pointer value: %d" ), iBtAudioStreamInputBase );
    STIF_ASSERT_NOT_NULL( iBtAudioStreamInputBase );
    
    res = iBtAudioStreamInputBase -> SetFormat(KMMFFourCCCodeSBC);
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Audio input format setting failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Audio input format set" ) );
    
    res = iSocketServer.Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Server connection failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Connected to socket server" ) );
    
    res = iSocketServer.FindProtocol( TProtocolName( KL2CAPDesC ), protocolDesc );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Searching protocol failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Protocol info found" ) );
    
    res = iSocket.Open( iSocketServer, protocolDesc.iAddrFamily, protocolDesc.iSockType, protocolDesc.iProtocol );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Socket opening failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Socket opened" ) );
    
    TUint frameLength = 8;
    TUint bitRate = 44100;
    
    TRAP( res, iBtAudioStreamer -> StartL( iSocket, frameLength, iBtAudioStreamInputBase, bitRate ));
    
    iLog -> Log( _L( "Start result: %d" ), res );
    
    STIF_ASSERT_EQUALS( KErrNone, res );
    
    iBtAudioStreamer -> Stop();
    
    iSocket.Close();
    iSocketServer.Close();
    iLog -> Log( _L( "Test case passed" ) );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBtAudioStreamerApiTest::SetNewFrameLengthTest
// -----------------------------------------------------------------------------
//

TInt CBtAudioStreamerApiTest::SetNewFrameLengthTest( CStifItemParser& aItem )
    {
    _LIT(KL2CAPDesC,"L2CAP");
    TInt res;
    TProtocolDesc protocolDesc;
    TProtocolName protocolName( KL2CAPDesC );
    
    iBtAudioStreamer = CBTAudioStreamer::NewL();
    iLog -> Log( _L( "BtAudioStreamer pointer value: %d" ), iBtAudioStreamer );
    STIF_ASSERT_NOT_NULL( iBtAudioStreamer );
    
    iBtAudioStreamerObserver = CBtAudioStreamerObserver::NewL( iLog );
    
    iBtAudioStreamInputBase = CBTAudioStreamInputBase::NewL( *iBtAudioStreamer, *iBtAudioStreamerObserver ); 
    iLog -> Log( _L( "BtAudioStreamer pointer value: %d" ), iBtAudioStreamInputBase );
    STIF_ASSERT_NOT_NULL( iBtAudioStreamInputBase );
    
    res = iBtAudioStreamInputBase -> Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Audio input connection failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Connected to audio input" ) );
    
    res = iBtAudioStreamInputBase -> SetFormat( KMMFFourCCCodeSBC );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Audio input set format failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Audio input format set" ) );
    
    iSbcEncoderIntfc = reinterpret_cast< CSbcEncoderIntfc* > ( iBtAudioStreamInputBase -> EncoderInterface( KUidSbcEncoderIntfc ) );
    
    res = iSocketServer.Connect();
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Server connection failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Connected to socket server" ) );
    
    res = iSocketServer.FindProtocol( TProtocolName( KL2CAPDesC ), protocolDesc );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Searching protocol failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Protocol info found" ) );
    
    res = iSocket.Open( iSocketServer, protocolDesc.iAddrFamily, protocolDesc.iSockType, protocolDesc.iProtocol );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Socket opening failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Socket opened" ) );
    
//    TInt option = 0;
//    res = iSocket.SetOpt( KL2CAPInboundMTU, KSolBtL2CAP, option );
//    if ( res != KErrNone )
//        {
//        iLog -> Log( _L( "Socket option setting with value: %d" ), res );
//        return res;
//        }
//    iLog -> Log( _L( "Socket option set" ) );
    
    TUint frameLength = 8;
    TUint newFrameLength = 10;
    TUint bitRate = 44100;
   
    
    TRAP( res, iBtAudioStreamer -> StartL( iSocket, frameLength, iBtAudioStreamInputBase, bitRate ));
    
    iLog -> Log( _L( "Start result: %d" ), res );
    
    STIF_ASSERT_EQUALS( KErrNone, res );
    
    res = iBtAudioStreamInputBase -> Start();
//    if ( res != KErrNone )
//        {
//        iLog -> Log( _L( "Starting audio input failed with value: %d" ), res );
//        return res;
//        }
//    iLog -> Log( _L( "Audio input started" ) );
    
    res = iBtAudioStreamer -> SetNewFrameLength( newFrameLength, bitRate );
    if ( res != KErrNone )
        {
        iLog -> Log( _L( "Setting new frame length failed with value: %d" ), res );
        return res;
        }
    iLog -> Log( _L( "Frame length changed" ) );
    
    iBtAudioStreamer -> Stop();
    iBtAudioStreamInputBase -> Stop();
    
    iSocket.Close();
    iSocketServer.Close();
    iLog -> Log( _L( "Test case passed" ) );
    return KErrNone;
    }


TInt CBtAudioStreamerApiTest::ConfigureEncoderInterface()
    {
    return KErrNone;
    }
// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
