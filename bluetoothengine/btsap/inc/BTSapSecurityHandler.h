/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
*     This class handles SAP security checking
*
*/

#ifndef BT_SAP_SECURITY_HANDLER_H
#define BT_SAP_SECURITY_HANDLER_H

//  INCLUDES
#include <bt_sock.h>
#include <hciproxy.h>
#include <btengdevman.h>

// Vendor specific
const TUint8 KCmdInfo[] = {0xFC, 0x00, 0x00, 0x00, 0xFC, 0x04, 0xF0, 0x07, 0x00, 0x00};
const TInt KExtCmdLen = 10;

// Security settings
const TInt KRequiredPassKeyLen       = 16;

// FORWARD DECLARATIONS
class CBTEngDevMan;
class CBTHciExtensionMan;
    
enum TBTSapSecurityCheckResult
    {
    ESecurityOK,
    EGetEncryptionKeyFail,
    EEncryptionKeyTooShort,
    EPassKeyTooShort
    };

class CBTSapSecurityHandler : public CActive, MBTEngDevManObserver
    {
public:
    // Constructors
    static CBTSapSecurityHandler* NewL();

    // Destructor
	~CBTSapSecurityHandler();

private:     // From CActive
   
    /**
    * DoCancel() has to be implemented by all the classes deriving CActive.
    * Request cancellation routine.
    * @param none
    * @return none 
    */
    void DoCancel();
    
    /**
    * RunL is called by framework after request is being completed.
    * @param nones
    * @return none 
    */
	void RunL();
	
	/**
	* RunError is used for handling leaves that occure inside RunL
	*/
	TInt RunError(TInt aError);

public:

    void CheckSapSecurity(RSocket& aSocket, TRequestStatus& aStatus);

private:

    // Two-phase constructor
    void ConstructL();

    // Default constructor
    CBTSapSecurityHandler();
    
    // from MBTEngDevManObserver
    void HandleGetDevicesComplete(TInt aErr,CBTDeviceArray* aDeviceArray);
    
private:

    enum TState
        {
        EEncryptionKeyLength,
        EPassKeyLength
        };

private:

    TRequestStatus *iSecurityStatus;
    RSocket        *iSocket;
    CBTHciExtensionMan* iBtHci;
    
    CBTEngDevMan* iBtDevMan;
    CBTDeviceArray* iBtDeviceArray;
    TState iState;

    THCIConnHandle iHCIConnHandle;
    TPckgBuf<THCIConnHandle> iHCIHandleBuf;
    TBuf8<KExtCmdLen> iExtCmd;

    TUint8 iEncryptionKeyLength;
    };

#endif // BT_SAP_SECURITY_HANDLER_H
