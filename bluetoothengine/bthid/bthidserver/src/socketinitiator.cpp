/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of application class
 *
*/

#include <e32debug.h>
#include "hiddebug.h"
#include "socketinitiator.h"
#include "sockinitnotifier.h"
#include "timeouttimer.h"
#include "bthidtypes.h"
#include "sockets.pan"
#include "debug.h"
// The transport protocol of the sockets used by this object.
_LIT(KTransportProtocol, "L2CAP");

// 60*2 seconds socket connect time-out (initially this was 10 seconds, but was changed due to end-user feedback)
const TInt CSocketInitiator::KTimeOut = 120000000;

CSocketInitiator* CSocketInitiator::NewL(RSocketServ& aSocketServ,
        MSockInitNotifier& aNotifier)
    {
    CSocketInitiator* self = NewLC(aSocketServ, aNotifier);
    CleanupStack::Pop(self);
    return self;
    }

CSocketInitiator* CSocketInitiator::NewLC(RSocketServ& aSocketServ,
        MSockInitNotifier& aNotifier)
    {
    CSocketInitiator* self = new (ELeave) CSocketInitiator(aSocketServ,
            aNotifier);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CSocketInitiator::CSocketInitiator(RSocketServ& aSocketServ,
        MSockInitNotifier& aNotifier) :
    CActive(CActive::EPriorityStandard), iSocketServ(aSocketServ), iNotifier(
            aNotifier), iState(EIdle)
    {
    CActiveScheduler::Add(this);
    }

CSocketInitiator::~CSocketInitiator()
    {
        TRACE_INFO((_L("[BTHID]\tCSocketInitiator::~CSocketInitiator()")));
    Cancel(); // Causes DoCancel

    delete iTimer;
    }

void CSocketInitiator::ConnectSocketsL(const TBTDevAddr& aAddress,
        TBool aUseSecurity, RSocket* aControlSocket,
        RSocket* aInterruptSocket)
    {
        TRACE_INFO((_L("[BTHID]\tCSocketInitiator::ConnectSocketsL")));
    // Store the params for later
    iControlSocket = aControlSocket;
    iInterruptSocket = aInterruptSocket;
    iUseSecurity = aUseSecurity;

    // First close the sockets
    iControlSocket->Close();
    iInterruptSocket->Close();

    // Try to open the sockets as L2CAP
    User::LeaveIfError(iControlSocket->Open(iSocketServ, KTransportProtocol));
    User::LeaveIfError(
            iInterruptSocket->Open(iSocketServ, KTransportProtocol));

    iSockAddress.SetBTAddr(aAddress);
    // First connect the control channel
    iSockAddress.SetPort(KL2CAPHidControl);

    // Set security requirements for the Control channel.
    TBTServiceSecurity sec;
    sec.SetAuthentication(aUseSecurity); //Require authentication
    sec.SetEncryption(aUseSecurity); //and encryption.
    sec.SetAuthorisation(EFalse); //and authorisation (This not needed for out-going connections)
    iSockAddress.SetSecurity(sec);

    // Start a timer to timeout the connect request
    if (!iTimer->IsActive())
        iTimer->After(KTimeOut);
    // Issue the connect request
    iControlSocket->Connect(iSockAddress, iStatus);
    // Update the connect state
    iState = EConnectingControl;

#ifdef __WINS__

    User::After(1); // Fix to allow emulator client to connect to server
#endif

    // Start this active object
    SetActive();
    }

void CSocketInitiator::TimerExpired()
    {
        TRACE_INFO((_L("[BTHID]\tCSocketInitiator::TimerExpired()")));
    // Cancel the operation in progress
    Cancel();
    // Inform the observer of this object that a failure occurred
    // due to a timeout
    iNotifier.SocketsConnFailed(KErrTimedOut);
    }

void CSocketInitiator::ConstructL()
    {
    // Create a timer
    iTimer = CTimeOutTimer::NewL(EPriorityStandard, *this);
    }

void CSocketInitiator::DoCancel()
    {
        TRACE_INFO((_L("[BTHID]\tCSocketInitiator::DoCancel()")));

    // Cancel appropriate request
    switch (iState)
        {
        case EConnectingControl:
            {
            iControlSocket->CancelConnect();
            break;
            }
        case EConnectingInterrupt:
            {
            iInterruptSocket->CancelConnect();
            break;
            }
        default:
            {
            User::Panic(KPanicBTConnection, ESocketsBadStatus);
            break;
            }
        }

    //Close the sockets
    iInterruptSocket->Close();
    iControlSocket->Close();

    // Set the connect state back to idle
    iState = EIdle;
    }

void CSocketInitiator::RunL()
    {
    // Cancel any outstanding timer
    iTimer->Cancel();

    // Any error stops us dead.
    if (iStatus != KErrNone)
        {
        //Close the sockets
        iControlSocket->Close();
        iInterruptSocket->Close();

        // Set the connect state back to idle
        iState = EIdle;
        // Inform the observer that a failure occurred
        iNotifier.SocketsConnFailed(iStatus.Int());
        }
    else
        {
        switch (iState)
            {
            case EConnectingControl:
                {
                    TRACE_INFO((_L("[BTHID]\tCSocketInitiator::RunL(): Control channel connection request")));
                ConnectInterruptSocket();
                }
                break;

            case EConnectingInterrupt:
                // L2CAP Interrupt channel connection request
                // We are connected
                // Set the connect state back to idle
                {
                    TRACE_INFO((_L("[BTHID]\tCSocketInitiator::RunL(): Interrupt channel connection request")));
                iState = EIdle;
                // Inform the observer that connection is complete
                iNotifier.SocketsConnected();
                break;
                }

            default:
                {
                User::Panic(KPanicBTConnection, ESocketsBadState);
                break;
                }

            };
        }
    }

void CSocketInitiator::ConnectInterruptSocket()
    {
        // Connect the interrupt channel.
        TRACE_INFO((_L("[BTHID]\tCSocketInitiator::ConnectInterruptSocket()")));
    ///for BT Stack v2.0, use authentication and encryption
    TBTServiceSecurity sec;
    sec.SetAuthentication(iUseSecurity); // 2nd L2cap channel should need no authentication.
    sec.SetEncryption(iUseSecurity); // but we need encryption.
    iSockAddress.SetSecurity(sec);
    ////
    iSockAddress.SetPort(KL2CAPHidInterrupt);

    // Start a timer to timeout the connect request
    if (!iTimer->IsActive())
        iTimer->After(KTimeOut);
    // Issue the connect request
    iInterruptSocket->Connect(iSockAddress, iStatus);
    // Update the connect state
    iState = EConnectingInterrupt;

#ifdef __WINS__

    User::After(1); // Fix to allow emulator client to connect to server
#endif

    SetActive();
    }
