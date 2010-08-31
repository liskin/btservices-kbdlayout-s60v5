/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class handles service connections.
*
*/


// INCLUDE FILES
#include <obexbttransportinfo.h>
#include "obexsmbtconnection.h"
#include "debug.h"

_LIT( KBTProtocol, "RFCOMM" );


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CObexSMBtConnection::CObexSMBtConnection()
    {
    }
// ---------------------------------------------------------
// Constructor
// ---------------------------------------------------------
//
CObexSMBtConnection::CObexSMBtConnection(TAny* aInitParams)    
    :iImplementationInfo((CImplementationInformation *)aInitParams),
     iBtDiscovery(NULL)
    {    
    }
// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CObexSMBtConnection::~CObexSMBtConnection()
    {
    // Cleanup
    FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection begin"));

    if(iPasskey)
        {
        iPasskey->Cancel();
	    FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection passkey query cancelled."));
        }
            
    if ( iBtDiscovery )
        {
        iBtDiscovery->DeleteSdpRecord( iSDPHandle );
        delete iBtDiscovery;
        }
	    
    FTRACE( FPrint( _L( "[SRCS] CSrcsBtConnection::~CSrcsBtConnection: UnregisterSDPSettingsL" ) ) );
        

    if(iServer)
        {
        iServer->Stop();
	    FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection obex server stopped."));
        }
    // Delete OBEX Server
    if (iServer)
        {
        delete iServer;
        FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection obex server deleted."));
        iServer = NULL;
        }
    
    // Delete passkey
    delete iPasskey;
    FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection iPasskey deleted."));
    iPasskey = NULL;

    // Delete Service Controller callback
    delete iController;
    FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection service controller deleted."));
    iController = NULL;
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CObexSMBtConnection* CObexSMBtConnection::NewL(TAny* aInitParams)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection: NewL"));
    CObexSMBtConnection* self = new (ELeave) CObexSMBtConnection(aInitParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


// ---------------------------------------------------------
// ConstructL
// Method to create BT connections.
// ---------------------------------------------------------
//
void CObexSMBtConnection::ConstructL()
    {
    TLex8 lex;
    // Create service controller implementation object
    iController = CSrcsInterface::NewL(iImplementationInfo->ImplementationUid());

    FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: CSrcsInterface::NewL\t"));	
	  iController->SetMediaType( ESrcsMediaBT );	
    // parsing opaque_data in iImplementationInfo
    TPtrC8 res_string = iImplementationInfo->OpaqueData();

    CObexUtilsOpaqueData res_data(res_string);

    TBuf8<KObexUtilsMaxOpaqueDataStringLen> op_LocalWho;
    TUint	op_Protocol, op_Authenticate, op_Authorise, op_Encrypt;
	  TUint	op_ReceiveMtu(KSRCSDefaultReceiveMtu), op_TransmitMtu(KSRCSDefaultTransmitMtu);

    // skip 1st entry: USB interface string descriptor
    User::LeaveIfError ( res_data.GetString(op_LocalWho) );

    // get 2nd entry: OBEX local who
    User::LeaveIfError ( res_data.GetString(op_LocalWho) );

    // get 3rd to 7th entry

	  // All service ID should be strings !
	  TBuf8<KObexUtilsMaxOpaqueDataStringLen> serviceUUID;
    User::LeaveIfError ( res_data.GetString(serviceUUID) );    
    
    FTRACE(FPrint(_L("[SRCS] CObexSMBtConnection::ConstructL() serviceUUID Length %d"), serviceUUID.Length()));
    
    if (serviceUUID.Length() < 6)
        {
        User::Leave( KErrArgument );
        }
            
    lex.Assign(serviceUUID);
    
    User::LeaveIfError ( res_data.GetNumber(op_Protocol) );
    User::LeaveIfError ( res_data.GetNumber(op_Authenticate) );
    User::LeaveIfError ( res_data.GetNumber(op_Authorise) );
    User::LeaveIfError ( res_data.GetNumber(op_Encrypt) );

	// Get OBEX packet size:
	if( res_data.GetNumber(op_ReceiveMtu) != KErrNone )
		{
		op_ReceiveMtu = KSRCSDefaultReceiveMtu; // restore default value in case it is modified;
	    FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: op_ReceiveMtu not specified.\t"));
		}
	
	if( res_data.GetNumber(op_TransmitMtu) != KErrNone )
		{
		op_TransmitMtu = KSRCSDefaultTransmitMtu; // restore default value in case it is modified;
	    FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: op_TransmitMtu not specified.\t"));
		}

    FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: Successfully get all data from opaque_data.\t"));

    // Create CBTEngDiscovery
    
    FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: CBTConnection::NewL\t"));

    // For handling Obex Passkey note
    iPasskey = CObexSMPasskey::NewL();
    FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: CSrcsPasskey::NewL\t"));

    // For showing Error Notes
    // iErrorUi = CErrorUI::NewL();

    // set up the protocol stack...
    TObexBluetoothProtocolInfo aObexBluetoothProtocolInfo;
    aObexBluetoothProtocolInfo.iTransport = KBTProtocol ;

	// Configure security setting
	FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: Setting Security\t"));
	TBTServiceSecurity sec;
	sec.SetAuthentication(op_Authenticate);
	sec.SetAuthorisation(op_Authorise);
	sec.SetEncryption(op_Encrypt);
	
	TUint32 serviceUID;
	lex.Inc(2);
	lex.Val(serviceUID,EHex);
	sec.SetUid(TUid::Uid(serviceUID));
	aObexBluetoothProtocolInfo.iAddr.SetSecurity(sec);	
	if (serviceUUID.Length() == 34)
	    {
	    TUint32 HH; TUint32 HL; TUint32 LH; TUint32 LL;
	    lex = serviceUUID.Mid(2, 8);
	    lex.Val(HH, EHex);
	    lex = serviceUUID.Mid(10, 8);
	    lex.Val(HL, EHex);
	    lex = serviceUUID.Mid(18, 8);
	    lex.Val(LH, EHex);
	    lex = serviceUUID.Mid(26, 8);
	    lex.Val(LL, EHex);
        iServiceUUID = TUUID(HH, HL, LH, LL);
	    }
	else
	    {
	    iServiceUUID = TUUID(serviceUID);
	    }

	// Configure Obex packet size
	TObexProtocolPolicy aObexProtocolPolicy;
	aObexProtocolPolicy.SetReceiveMtu( static_cast<TUint16>(op_ReceiveMtu) );
	aObexProtocolPolicy.SetTransmitMtu( static_cast<TUint16>(op_TransmitMtu) );
	FTRACE(FPrint(_L("[SRCS] CSrcsBtConnection: ConstructL: op_ReceiveMtu[%x] op_TransmitMtu[%x]"), 
		aObexProtocolPolicy.ReceiveMtu(), aObexProtocolPolicy.TransmitMtu() ));

	// try to find a available channel to start OBEX server.
    TUint channel=KRfcommPassiveAutoBind;
    
	aObexBluetoothProtocolInfo.iAddr.SetPort( channel );	
    TRAPD(err,iServer = CObexServer::NewL(aObexBluetoothProtocolInfo, aObexProtocolPolicy);)
	// Try to start server on this channel
    if ( err == KErrNone )
	    {
	    err=iController->SetObexServer( iServer );   
		if ( err == KErrNone )
		    {			
			// Set authentication handler
			iServer->SetCallBack( *this );
			//If there is "local who" field, set it to OBEX server.
			if(op_LocalWho.Size())
			    {
				User::LeaveIfError ( iServer->SetLocalWho( op_LocalWho ) );
				FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: SetLocalWho\t"));
				}
			FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: SetObexServer\t"));
		
		    // Register SDP settings
		    
		    TObexBtTransportInfo *info=(TObexBtTransportInfo*)iServer->TransportInfo();
		    channel=info->iAddr.Port();  		    
		    FTRACE(FPrint(_L("[SRCS] CSrcsBtConnection: ConstructL: CObexServer started on channel %d"), channel));			
		    iBtDiscovery = CBTEngDiscovery::NewL();		    
			User::LeaveIfError ( iBtDiscovery->RegisterSdpRecord(iServiceUUID, channel, iSDPHandle ));				                
			}
		else
		    {
			// Failed to start Obex server. Delete server and try again
			FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: Failed to start Obex server. Delete server and try again.\t"));
			delete iServer;
			iServer = NULL;
			}
		}
    if ( !iServer )
        {
        // Server did not start.
        FLOG(_L("[SRCS] CSrcsBtConnection: ConstructL: Failed to start Obex server. Leave now! \t"));
        User::Leave( KErrGeneral );
        }
    
    }

// ---------------------------------------------------------
// GetUserPasswordL(const TDesC& )
// Purpose: Get the Obex password from user with ObexPasskeyNotifier
// Parameters:
// Return value:
// ---------------------------------------------------------
//
void CObexSMBtConnection::GetUserPasswordL( const TDesC& )
    {
    FLOG( _L( "[SRCS] CSrcsBtConnection: GetUserPassword\t" ) );
    iPasskey->StartPassKeyRequestL( iServer );
    }

// ---------------------------------------------------------
// Check if OBEX service is still active.
// @return ETrue:  OBEX service is still active.
//  	   EFalse: OBEX service is inactive.
// ---------------------------------------------------------
//
TBool CObexSMBtConnection::IsOBEXActive()
	{
    FLOG( _L( "[SRCS] CSrcsBtConnection: IsOBEXActive\t" ) );
	return ETrue;	
	}
	
// End of file
