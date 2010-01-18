/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*	  This class is for security checking of key length of both encryption and pass keys
*
*/



// INCLUDE FILES
#include <btdevice.h>
#include <btextnotifiers.h> // Needed to check the NOTIFIERS_SUPPORT_PASSKEY_MIN_LENGTH flag
#include <hciproxy.h>
#include <btengdomainpskeys.h>

#include "BTSapSecurityHandler.h"
#include "BTSapHciExtensionMan.h"
#include "debug.h"

const TInt KRequiredEncryptionKeyLen = 128;

CBTSapSecurityHandler::CBTSapSecurityHandler()
	: CActive(CActive::EPriorityStandard)
	{
	CActiveScheduler::Add(this);
	}

CBTSapSecurityHandler::~CBTSapSecurityHandler()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSecurityHandler")));
	Cancel();
	delete iBtHci;
	if(iBtDeviceArray)
	    {
        iBtDeviceArray->ResetAndDestroy();
        delete iBtDeviceArray;
	    }
	delete iBtDevMan;
	}

// ---------------------------------------------------------
// CBTSapSecurityHandler::NewL()
// ---------------------------------------------------------
//
CBTSapSecurityHandler* CBTSapSecurityHandler::NewL()
	{
	CBTSapSecurityHandler* self = new (ELeave) CBTSapSecurityHandler();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

// ---------------------------------------------------------
// CBTSapSecurityHandler::ConstructL
// ---------------------------------------------------------
//
void CBTSapSecurityHandler::ConstructL()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: ConstructL")));
	iBtDevMan = CBTEngDevMan::NewL(this);
	iBtDeviceArray = new (ELeave) CBTDeviceArray(1);
	iBtHci = CBTHciExtensionMan::NewL();
	}

// ---------------------------------------------------------
// CBTSapSecurityHandler::DoCancel
// ---------------------------------------------------------
//
void CBTSapSecurityHandler::DoCancel()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: DoCancel")));
	if (iSecurityStatus && iSecurityStatus->Int() == KRequestPending)
	    {
	    User::RequestComplete(iSecurityStatus, KErrCancel);
	    }
	}
	
// ---------------------------------------------------------
// CBTSapSecurityHandler::RunL
// ---------------------------------------------------------
//
TInt CBTSapSecurityHandler::RunError(TInt aError)
    {
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: RunError: %d"), aError));    
   
    switch(iState)
        {
        case EEncryptionKeyLength:
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: RunError: Couldn't get EncryptionKeyLength: %d"), aError));
            User::RequestComplete(iSecurityStatus, static_cast <TInt> (EGetEncryptionKeyFail));
            }
            break;
        case EPassKeyLength:
            {
            User::RequestComplete(iSecurityStatus, static_cast <TInt> (EPassKeyTooShort));
            }
            break;
        default:
            {
            User::RequestComplete(iSecurityStatus, aError);
            }
        }
    return KErrNone;
    }


// ---------------------------------------------------------
// CBTSapSecurityHandler::RunL
// ---------------------------------------------------------
//
void CBTSapSecurityHandler::RunL()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: RunL: %d"), iStatus.Int()));
    TBTSapSecurityCheckResult result = ESecurityOK;
    TBool complete = EFalse;
    
    User::LeaveIfError(iStatus.Int()); // handle errors in RunError
    
    switch(iState)
        {
        case EEncryptionKeyLength:
            {
            iBtHci->GetResultL(iEncryptionKeyLength);
            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: RunL: EncryptionKeyLength: %d"), iEncryptionKeyLength));
            
            if (iEncryptionKeyLength < KRequiredEncryptionKeyLen)
                {
                result = EEncryptionKeyTooShort;
                complete = ETrue;
                }
            else
                {
                ASSERT(iBtDeviceArray);
                TBTSockAddr sockAddr;
                iSocket->RemoteName(sockAddr);
                TBTRegistrySearch criteria;
                criteria.FindAddress(sockAddr.BTAddr());
                iBtDeviceArray->ResetAndDestroy();
                iBtDevMan->GetDevices(criteria, iBtDeviceArray);
                iState = EPassKeyLength;
                iStatus = KRequestPending;
                SetActive();
                }
            }
            break;
        case EPassKeyLength:
            {
            if (!iBtDeviceArray || !iBtDeviceArray->Count())
                {
                User::Leave(KErrNotFound);
                }

            CBTDevice* device = iBtDeviceArray->At( 0 );
            // When SSP is used, the link key needs to be authenticated,
            // otherwise, the passkey needs to be 16 digits.
            if( device->LinkKeyType() != ELinkKeyAuthenticated )
                {
                BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: unauthenticated link key")));
                if( !( device->LinkKeyType() == ELinkKeyCombination &&
                       device->PassKeyLength() >= KRequiredPassKeyLen ) )
                    {
                    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: unacceptable link key")));
                    result = EPassKeyTooShort;
                    }
                else if( device->LinkKeyType() == ELinkKeyDebug )
                    {
                    // For SAP, we do an extra check for debug mode, to be really sure.
                    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: Debug link key, checking debug mode")));
                    TBTSspDebugModeValue debugMode = EBTSspDebugModeOff;
                    TInt err = RProperty::Get( KPSUidBluetoothTestingMode, KBTSspDebugmode, (TInt&) debugMode );
                    if( err || debugMode == EBTSspDebugModeOff )
                        {
                        BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler: debug key not allowed")));
                        result = EPassKeyTooShort;
                        }
                    }
                }
            complete = ETrue;
            }
            break;
        default:
            {
            User::Leave(KErrNotSupported);
            }
        }
    if (complete)
        {
        User::RequestComplete(iSecurityStatus, static_cast <TInt> (result));
        }
	}

// ---------------------------------------------------------
// CBTSapSecurityHandler::CheckSapSecurity
// ---------------------------------------------------------
void CBTSapSecurityHandler::CheckSapSecurity(RSocket& aSocket, TRequestStatus& aStatus)
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler::CheckSapSecurity")));

	iSocket = &aSocket;
	iSecurityStatus = &aStatus;
	aStatus = KRequestPending;

    TBTSockAddr sockAddr;
    aSocket.RemoteName(sockAddr);

    TBTDevAddr btAddr = sockAddr.BTAddr();
    TRAPD(err, iBtHci->GetEncryptionKeyLengthL(btAddr, iStatus));
    if (err)
        {
        User::RequestComplete(iSecurityStatus, err);
        }
    else
        {
        iState = EEncryptionKeyLength;
        SetActive();
        }
	}
	

// ---------------------------------------------------------
// CBTSapSecurityHandler::HandleGetDevicesComplete, from MBTEngDevManObserver
// ---------------------------------------------------------	
void CBTSapSecurityHandler::HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* /*aDeviceArray*/ )
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSecurityHandler::HandleGetDevicesComplete")));
    // Complete our own request -> RunL
    TRequestStatus* ownStatus = &iStatus;
    User::RequestComplete(ownStatus, aErr);
    }

//	End of File
