/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "obexsmusbconnection.h"
#include "debug.h"
#include <d32usbc.h>
#include "obexutilsopaquedata.h"
#include <centralrepository.h>
#include "obexservicemanprivatecrkeys.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CObexSMUsbConnection::CObexSMUsbConnection()
    {
    }
// ---------------------------------------------------------
// Constructor
// ---------------------------------------------------------
//
CObexSMUsbConnection::CObexSMUsbConnection(TAny* aInitParams)        
    :iImplementationInfo((CImplementationInformation *)aInitParams)
    {    
    }

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CObexSMUsbConnection::~CObexSMUsbConnection()
    {
    // Cleanup       
    FLOG(_L("[SRCS] CSrcsUsbConnection: StopObexServer"));              
    if(iServer)
        {
        iServer->Stop();
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
    iController = NULL;    
    
    FLOG(_L("[SRCS] CSrcsUsbConnection: ~CSrcsUsbConnection: Completed\t"));    
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CObexSMUsbConnection* CObexSMUsbConnection::NewL(TAny* aInitParams)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsUsbConnection: NewL"));
    CObexSMUsbConnection* self = new (ELeave) CObexSMUsbConnection(aInitParams);
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
void CObexSMUsbConnection::ConstructL()
    {    
    // Save service controller implementation UID for deferred initialization
    iImplUid = iImplementationInfo->ImplementationUid();

    FLOG(_L("[SRCS] CSrcsUsbConnection: ConstructL: CSrcsInterface::NewL\t"));

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
	    FLOG(_L("[SRCS] CSrcsUsbConnection: ConstructL: op_ReceiveMtu not specified.\t"));
		}
	
	if( res_data.GetNumber(op_TransmitMtu) != KErrNone )
		{
		op_TransmitMtu = KSRCSDefaultTransmitMtu; // restore default value in case it is modified;
	    FLOG(_L("[SRCS] CSrcsUsbConnection: ConstructL: op_TransmitMtu not specified.\t"));
		}   

    FLOG(_L("[SRCS] CSrcsUsbConnection: ConstructL: Successfully get all data from opaque_data.\t"));

	// Get OBEX usb dma usage
    CRepository* cenrep = NULL;
    TRAP_IGNORE(cenrep = CRepository::NewL(KCRUidObexServiceMan));
    TInt dmaUsage = EObexUsbDmaInUse;
    if (cenrep)
        {
        cenrep->Get(KObexUsbDmaUsage, dmaUsage);
        delete cenrep;
        }

	// set up the protocol stack...    
    TObexUsbProtocolInfoV2 obexUsbProtocolInfo;
    obexUsbProtocolInfo.iTransport = KObexUsbProtocolV2;
    obexUsbProtocolInfo.iInterfaceStringDescriptor.Copy(op_StringDesc);
    if(dmaUsage == EObexUsbDmaInUse)
	    {
	    obexUsbProtocolInfo.iDmaOnInEndpoint = ETrue;
	    obexUsbProtocolInfo.iDmaOnOutEndpoint = ETrue;
	    }
	 else
	 	{
	 	obexUsbProtocolInfo.iDmaOnInEndpoint = EFalse;
	    obexUsbProtocolInfo.iDmaOnOutEndpoint = EFalse;
	 	}	
    obexUsbProtocolInfo.iBandwidthPriority = EUsbcBandwidthOUTPlus2 | EUsbcBandwidthINPlus2;



	// Configure Obex packet size
	TObexProtocolPolicy aObexProtocolPolicy;
	aObexProtocolPolicy.SetReceiveMtu( static_cast<TUint16>(op_ReceiveMtu) );
	aObexProtocolPolicy.SetTransmitMtu( static_cast<TUint16>(op_TransmitMtu) );
	FTRACE(FPrint(_L("[SRCS] CSrcsUsbConnection: ConstructL: op_ReceiveMtu[%x] op_TransmitMtu[%x]"), 
		aObexProtocolPolicy.ReceiveMtu(), aObexProtocolPolicy.TransmitMtu() ));

	// Try to create OBEX server
	iServer = CObexServer::NewL(obexUsbProtocolInfo, aObexProtocolPolicy);
    FLOG(_L("[SRCS] CSrcsUsbConnection: ConstructL: CObexServer::NewL"));
    // if there is "local who" field
    if(op_LocalWho.Size())
        {
        User::LeaveIfError ( iServer->SetLocalWho( op_LocalWho ) );
        FLOG(_L("[SRCS] CSrcsUsbConnection: ConstructL: SetLocalWho"));
        }
    
    // The rest of initialization procedure is executed in PostInitialzeL()
    }

// ---------------------------------------------------------
// GetUserPasswordL(const TDesC& )
// Purpose: Get the Obex password from user with ObexPasskeyNotifier
// Parameters:
// Return value:
// ---------------------------------------------------------
//
void CObexSMUsbConnection::GetUserPasswordL( const TDesC& )
    {
    FLOG( _L( "[SRCS] CSrcsUsbConnection: GetUserPassword\t" ) );
    }

// ---------------------------------------------------------
// Check if OBEX service is still active.
// @return ETrue:  OBEX service is still active.
//  	   EFalse: OBEX service is inactive.
// ---------------------------------------------------------
//

TBool CObexSMUsbConnection::IsOBEXActive()
	{
    FLOG( _L( "[SRCS] CSrcsUsbConnection: IsOBEXActive\t" ) );
	return ETrue;	
	}     

void CObexSMUsbConnection::PostInitializeL()
    {
    FTRACE(FPrint(_L("[SRCS] CObexSMUsbConnection: PostInitializeL(%08X)"), iImplUid.iUid));

    iController = CSrcsInterface::NewL(iImplUid);
    iController->SetMediaType( ESrcsMediaUSB );

    FLOG(_L("[SRCS] CObexSMUsbConnection::PostInitializeL(): SetObexServer\t"));
    // Try to start server 
    User::LeaveIfError ( iController->SetObexServer( iServer ));
    FLOG( _L( "[SRCS] CObexSMUsbConnection: PostInitializeL() exits\t" ) );
    }

// End of file
