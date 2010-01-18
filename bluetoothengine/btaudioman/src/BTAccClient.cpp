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
*        BT Accessory Server's client API Implementation. Starts server 
*        and creates a session with server if necessary. 
*
*
*/


// INCLUDE FILES
#include "BTAccClient.h"
#include "BTAccClientSrv.h"
#include "debug.h"

// CONSTANTS

_LIT(KBTAudioManImg, "btaudioman.exe");

const TUid KBTAudioManUid3={0x10208971};

static TInt StartServer()
    {
    const TUidType serverUid(KNullUid,KNullUid,KBTAudioManUid3);
    
    RProcess server;
    TInt r = server.Create(KBTAudioManImg, KNullDesC, serverUid);
    server.SetPriority(EPriorityHigh); 
    if (r!=KErrNone)
        return r;
    TRequestStatus stat;
    server.Rendezvous(stat);
    if (stat!=KRequestPending)
        {
        server.Kill(0);        // abort startup
        }
    else
        {
        server.Resume();    // logon OK - start the server
        }
        
    User::WaitForRequest(stat);        // wait for start or death
    TRACE_INFO((_L("Server started, code %d"), stat.Int()))
    r=(server.ExitType()==EExitPanic) ? KErrGeneral : stat.Int();
    server.Close(); 
    return r;
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// Connect
// Handles connection to server(creates session)
// ---------------------------------------------------------
//
TInt RBTAccClient::Connect()
    {
    TRACE_FUNC
    TInt retVal = CreateSession(KBTAudioManName, Version());
    if (retVal != KErrNone && retVal != KErrAlreadyExists)
        {
        retVal = StartServer();
        if (retVal == KErrNone || retVal == KErrAlreadyExists)
            {
            retVal = CreateSession(KBTAudioManName, Version());
            }
        }
    TRACE_INFO((_L("ret %d "), retVal))
    return retVal;
    }

// ---------------------------------------------------------
// Version
// Defines server version number
// ---------------------------------------------------------
//
TVersion RBTAccClient::Version() const
    {
    return(TVersion(KBTAccServerMajorVersionNumber,
                    KBTAccServerMinorVersionNumber,
                    KBTAccServerBuildVersionNumber));
    }

// ---------------------------------------------------------
// ConnectToAccessory
// Establishes a Bluetooth Handsfree connection
// ---------------------------------------------------------
//
void RBTAccClient::ConnectToAccessory(TRequestStatus& aStatus, 
    const TBTDevAddrPckgBuf& aAddrBuf, TDes8& aDiagnostic)
    {
    SendReceive(EBTAccSrvConnectToAccessory, TIpcArgs(&aAddrBuf, &aDiagnostic), aStatus);
    }
    
// ---------------------------------------------------------
// CancelConnectToAccessory
// Cancel Connection attempt
// ---------------------------------------------------------
//
void RBTAccClient::CancelConnectToAccessory()
    {
    TRACE_FUNC
    SendReceive(EBTAccSrvCancelConnectToAccessory);
    }

// ---------------------------------------------------------
// DisconnectAccessory
// Disconnect Bluetooth Handfree device
// ---------------------------------------------------------
//
void RBTAccClient::DisconnectAccessory(TRequestStatus& aStatus, const TBTDevAddrPckgBuf& aAddrBuf, TDes8& aDiagnostic)
    {
    TRACE_FUNC
    SendReceive(EBTAccSrvDisconnectAccessory, TIpcArgs(&aAddrBuf, &aDiagnostic), aStatus);
    }

TInt RBTAccClient::GetConnections(TDes8& aAddrs, TInt aProfile)
    {
    TRACE_FUNC
    return SendReceive(EBTAccSrvGetConnections, TIpcArgs(&aAddrs, aProfile));
    }

void RBTAccClient::DisconnectAllGracefully(TRequestStatus& aStatus)
    {
    TRACE_FUNC
    SendReceive(EBTAccSrvDisconnectAllGracefully, aStatus);
    }

void RBTAccClient::NotifyConnectionStatus(TProfileStatusPckg& aPckg, TRequestStatus& aStatus)
    {
    SendReceive(EBTAccSrvNotifyConnectionStatus, TIpcArgs(&aPckg), aStatus);
    }

void RBTAccClient::CancelNotifyConnectionStatus()
    {
    SendReceive(EBTAccSrvCancelNotifyConnectionStatus);
    }

TInt RBTAccClient::IsConnected(const TBTDevAddr& aAddr) const
    {
    TRACE_FUNC
    TBTDevAddrPckgBuf pckg(aAddr);
    return SendReceive(EBTAccSrvConnectionStatus, TIpcArgs(&pckg));
    }
    
// ---------------------------------------------------------
// GetBTAccInfo
// ---------------------------------------------------------
//
void RBTAccClient::GetInfoOfConnectedAcc(TPckg<TAccInfo>& aInfoPckg, TRequestStatus& aStatus )
    {
    TRACE_FUNC
    SendReceive(EBTAccSrvGetInfoOfConnectedAcc, TIpcArgs(&aInfoPckg),  aStatus);
    }

// ---------------------------------------------------------
// AudioToPhone
// Dos Related functionality only. 
// ---------------------------------------------------------
//
TInt RBTAccClient::AudioToPhone()
    {
    TRACE_FUNC
    return SendReceive(EBTAccSrvAudioToPhone);
    }

// ---------------------------------------------------------
// AudioToAccessory
// Dos Related functionality only.
// ---------------------------------------------------------
//
void RBTAccClient::AudioToAccessory( TRequestStatus& aStatus )
    {
    TRACE_FUNC
    SendReceive(EBTAccSrvAudioToAccessory, aStatus);
    }

void RBTAccClient::CancelAudioToAccessory()
    {
    SendReceive(EBTAccSrvCancelAudioToAccessory);
    }

    
// End of File
