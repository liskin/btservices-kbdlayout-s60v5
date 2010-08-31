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

#include "obexsmirdaconnection.h"
#include "debug.h"
#include "obexutilsopaquedata.h"

const TUint16 KMaxMtuSize    = 0x7FFF;  // 32kB-1

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CObexSMIrDAConnection::CObexSMIrDAConnection()
    {
    }
// ---------------------------------------------------------    
//  Constructor
// ---------------------------------------------------------
//
CObexSMIrDAConnection::CObexSMIrDAConnection(TAny* aInitParams)        
    :iImplementationInfo((CImplementationInformation *)aInitParams)
    {    
    }

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CObexSMIrDAConnection::~CObexSMIrDAConnection()
    {
    // Cleanup
    FLOG(_L("[SRCS]\tserver\tCSrcsIrDAConnection::~CSrcsIrDAConnection"));
    if(iServer)
        {
        iServer->Stop();
	    FLOG(_L("[SRCS]\tserver\tCSrcsIrDAConnection::~CSrcsIrDAConnection: OBEX server stopped."));
        }
	
    // Delete OBEX Server
    if (iServer)
        {
        delete iServer;
        FLOG(_L("[SRCS]\tserver\tCSrcsBtConnection::~CSrcsBtConnection obex server deleted."));
        iServer = NULL;
        }

    // Delete Service Controller callback
    delete iController;
    FLOG(_L("[SRCS]\tserver\tCSrcsIrDAConnection::~CSrcsIrDAConnection: service controller deleted."));
    iController = NULL;

    FLOG(_L("[SRCS]\tserver\tCSrcsIrDAConnection::~CSrcsIrDAConnection finished."));
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CObexSMIrDAConnection* CObexSMIrDAConnection::NewL(TAny* aInitParams)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsIrDAConnection: NewL"));
    CObexSMIrDAConnection* self = new (ELeave) CObexSMIrDAConnection(aInitParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


// ---------------------------------------------------------
// ConstructL
// Method to create IrDA connections.
// ---------------------------------------------------------
//
void CObexSMIrDAConnection::ConstructL()
    {

    // Create service controller implementation object
    iController = CSrcsInterface::NewL(iImplementationInfo->ImplementationUid());
    FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: CSrcsInterface::NewL\t"));

	iController->SetMediaType( ESrcsMediaIrDA );

	//Start service controller state machine.
	iController->IsOBEXActive();

    // parsing opaque_data in iImplementationInfo
    TPtrC8 res_string = iImplementationInfo->OpaqueData();

    CObexUtilsOpaqueData res_data(res_string);
    TBuf8<KObexUtilsMaxOpaqueDataStringLen> op_StringDesc, op_LocalWho, op_SkipStr;
    TUint	op_SkipInt; //used to skip certain fields
	TUint	op_ReceiveMtu(KSRCSDefaultReceiveMtu), op_TransmitMtu(KSRCSDefaultTransmitMtu);

    // get 1st entry: USB interface string descriptor
    User::LeaveIfError ( res_data.GetString(op_StringDesc) );

    // get 2nd entry: OBEX local who
    User::LeaveIfError ( res_data.GetString(op_LocalWho) );

    // skip 3rd to 7th entry
	// All service ID should be strings !
    res_data.GetString(op_SkipStr);
    res_data.GetNumber(op_SkipInt);
    res_data.GetNumber(op_SkipInt);
    res_data.GetNumber(op_SkipInt);
    res_data.GetNumber(op_SkipInt);

	// Get OBEX packet size:
	if( res_data.GetNumber(op_ReceiveMtu) != KErrNone )
		{
		op_ReceiveMtu = KSRCSDefaultReceiveMtu; // restore default value in case it is modified;		
	  FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: op_ReceiveMtu not specified.\t"));
		}
	
	if( res_data.GetNumber(op_TransmitMtu) != KErrNone )
		{
		op_TransmitMtu = KSRCSDefaultTransmitMtu; // restore default value in case it is modified;
	  FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: op_TransmitMtu not specified.\t"));
		}
		//limit max mtu size to 32kB-1
		if (op_ReceiveMtu>KMaxMtuSize)
		  {
		  op_ReceiveMtu=KMaxMtuSize;
		  }
		if (op_TransmitMtu>KMaxMtuSize)
		  {
		  op_TransmitMtu=KMaxMtuSize;  
		  }  

    FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: Successfully get all data from opaque_data.\t"));

    // set up the protocol stack...
    TObexIrProtocolInfo aInfo;

    aInfo.iTransport = KObexIrTTPProtocol; //"IrTinyTP" from obex.h
    aInfo.iClassName.Copy(op_StringDesc);
	aInfo.iAttributeName = _L8("IrDA:TinyTP:LsapSel");;

	// Configure Obex packet size
	TObexProtocolPolicy aObexProtocolPolicy;
	aObexProtocolPolicy.SetReceiveMtu( static_cast<TUint16>(op_ReceiveMtu) );
	aObexProtocolPolicy.SetTransmitMtu( static_cast<TUint16>(op_TransmitMtu) );
	FTRACE(FPrint(_L("[SRCS] CSrcsIrDAConnection: ConstructL: op_ReceiveMtu[%x] op_TransmitMtu[%x]"), 
		aObexProtocolPolicy.ReceiveMtu(), aObexProtocolPolicy.TransmitMtu() ));

	// try to find a available port to start OBEX server.
    TUint port=KAutoBindLSAP;    
	// create obex server
		
	aInfo.iAddr.SetPort( port );
	TRAPD(err,iServer = CObexServer::NewL(aInfo, aObexProtocolPolicy);)

	// Try to start server on this port
	if ( err == KErrNone )
	    {    	
	    FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: SetObexServer\t"));
		err=iController->SetObexServer( iServer );
		FTRACE(FPrint(_L("[SRCS] CSrcsIrDAConnection: ConstructL: CObexServer started on port %d"), port));		
		
		if (err!=KErrNone)
	    	{
			// Failed to start Obex server. Delete server and try again
			delete iServer;
			iServer = NULL;
			}
		}
		

	if ( !iServer )
        {
        // Server did not start.
        FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: Failed to start Obex server\t"));
        User::Leave( KErrGeneral );
        }
	else
		{
		// if there is "local who" field
		if(op_LocalWho.Size())
			{
			User::LeaveIfError ( iServer->SetLocalWho( op_LocalWho ) );
			FLOG(_L("[SRCS] CSrcsIrDAConnection: ConstructL: SetLocalWho"));
			}
		}
    }

// ---------------------------------------------------------
// GetUserPasswordL(const TDesC& )
// Purpose: Get the Obex password from user with ObexPasskeyNotifier
// Parameters:
// Return value:
// ---------------------------------------------------------
//
void CObexSMIrDAConnection::GetUserPasswordL( const TDesC& )
    {
    FLOG( _L( "[SRCS] CSrcsIrDAConnection: GetUserPassword\t" ) );
    }

// ---------------------------------------------------------
// Check if OBEX service is still active.
// @return ETrue:  OBEX service is still active.
//  	   EFalse: OBEX service is inactive.
// ---------------------------------------------------------
//

TBool CObexSMIrDAConnection::IsOBEXActive()
	{
    FLOG( _L( "[SRCS] CSrcsIrDAConnection: IsOBEXActive\t" ) );
	return iController->IsOBEXActive();
	}

// End of file
