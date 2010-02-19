/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*        BT Accessory Server's client API declaration. Starts server 
*        and creates a session with server if necessary. 
*
*
*/


#ifndef BTACCCLIENT_H
#define BTACCCLIENT_H

#include <bt_sock.h>
#include "BTAccInfo.h"
#include "btaccTypes.h"


// CLASS DECLARATION

/**
* RBTAccClient, the client class. 
* An RSessionBase sends messages to the server with the function
* RSessionBase::SendReceive(); specifying an opcode and and array of argument 
* pointers. Functions in derived classes, such as RBTAccServer::ManageServices(), 
* are wrappers for different calls to SendReceive().
*/
class RBTAccClient : public RSessionBase
    {
public:
    
    /**
    * Connects to Bluetooth audio accessory
    * @param    aStatus the request status, KErrNone if the connection is successful, a Symbian 
    *           error code otherwise, specifically:
    *           KErrNotSupported if the device doesn't support any BT audio profile;
    *           KErrAlreadyExists if the request is rejected due to other existing connections.
    * @param    aAddrBuf the package buf of the device address
    * @param    aDiagnostic will contain the conflict connections if connecting fails due to existing connections.
    *           Otherwise will contain the connected profile (type TProfiles)
    */
    void ConnectToAccessory(TRequestStatus& aStatus, const TBTDevAddrPckgBuf& aAddrBuf, TDes8& aDiagnostic);

    /**
    * Cancel Connection attempt to Bluetooth audio accessory
    * @param    aDevice Bluetooth address of the device
    * @return   TInt indicating the success of call.      
    */
    void CancelConnectToAccessory();

    /**
    * Disconnects Bluetooth audio accessory
    * @param    aStatus the request status, KErrNone if the operation is successful, a Symbian 
    *           error code otherwise
    * @param    aAddrBuf the package buf of the device address
    * @param    aDiagnostic will contain the disconnected profile (type TProfiles)
    */
    void DisconnectAccessory(TRequestStatus& aStatus, const TBTDevAddrPckgBuf& aAddrBuf, TDes8& aDiagnostic);


    TInt GetConnections(TDes8& aAddrs, TInt aProfile);

    /**
    * Disconnects all connection gracefully.
    * This is called internally when BTAudioMan is going to be destroyed.
    */
    void DisconnectAllGracefully(TRequestStatus& aStatus);
    
    void NotifyConnectionStatus(TProfileStatusPckg& aPckg, TRequestStatus& aStatus);
    
    void CancelNotifyConnectionStatus();
    
    /**
    * Checks if this is connecting or has been connected to the specified device
    * 
    * @param aAddr the specified BD address
    */
    TInt IsConnected(const TBTDevAddr& aAddr) const;

    /**
    * 
    */
    void GetInfoOfConnectedAcc(TPckg<TAccInfo>& aInfoPckg, TRequestStatus& aStatus ); 
    
public: // Functions from base classes

    /**
    * From RSessionBase informs current version
    * @param    None
    * @return   Version information about the server
    */
    TVersion Version() const;

    /**
    * From RSessionBase connect to the server
    * @param    None
    * @return   Error/success code
    */
    TInt Connect();

private:    // Data
    };

#endif  // BTACCCLIENT_H
// End of File
