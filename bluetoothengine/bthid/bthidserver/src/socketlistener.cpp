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

#include <bt_sock.h>
#include "socketlistener.h"
#include "listenerobserver.h"
#include "sockets.pan"
#include "bthidclientsrv.h" //for BT Stack v2 security
#include "bthidtypes.h"
#include <e32std.h>
#include <e32svr.h>
#include "debug.h"
// The transport protocol of the sockets used by this object.
_LIT(KTransportProtocol, "L2CAP");

const TInt KListenQueueSize = 2;

CSocketListener* CSocketListener::NewL(RSocketServ& aSocketServ, TUint aPort,
        MListenerObserver& aObserver, TBool aAuthorisation)
    {
    CSocketListener* self = NewLC(aSocketServ, aPort, aObserver,
            aAuthorisation);
    CleanupStack::Pop(self);
    return self;
    }

CSocketListener* CSocketListener::NewLC(RSocketServ& aSocketServ,
        TUint aPort, MListenerObserver& aObserver, TBool aAuthorisation)
    {
    CSocketListener* self = new (ELeave) CSocketListener(aSocketServ, aPort,
            aObserver, aAuthorisation);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CSocketListener::CSocketListener(RSocketServ& aSocketServ, TUint aPort,
        MListenerObserver& aObserver, TBool aAuthorisation) :
    CActive(CActive::EPriorityStandard), iSocketServer(aSocketServ), iPort(
            aPort), iObserver(aObserver), iAuthorisationAskedFlag(
            aAuthorisation)
    {
    CActiveScheduler::Add(this);
    }

CSocketListener::~CSocketListener()
    {
        TRACE_INFO((_L("[BTHID]\tCSocketListener::~CSocketListener()")));
    Cancel(); // Causes DoCancel

    // Close the listening socket
    iListeningSocket.Close();
    }

void CSocketListener::ConstructL()
    {
        TRACE_INFO((_L("[BTHID]\tCSocketListener::ConstructL()")));
    // Open the listening socket
    User::LeaveIfError(iListeningSocket.Open(iSocketServer,
            KTransportProtocol));
    TL2CAPSockAddr addr;
    addr.SetPort(iPort);
    TBTServiceSecurity serverSecurity;
    serverSecurity.SetUid(KUidBTHidServer);
    serverSecurity.SetAuthentication(ETrue); //Microsoft KB reconnect needs this!?? --> This line has caused IOP problems before.
    serverSecurity.SetAuthorisation(iAuthorisationAskedFlag); //This should be according to Authorisation setting
    serverSecurity.SetDenied(EFalse);

    //No need for Interrupt channel to use encryption
    if (KL2CAPHidInterrupt == iPort)
        serverSecurity.SetEncryption(EFalse); //Logitech Mx5000 needs this false.
    else
        serverSecurity.SetEncryption(ETrue);

    addr.SetSecurity(serverSecurity);
    //Bind
    User::LeaveIfError(iListeningSocket.Bind(addr));
    User::LeaveIfError(iListeningSocket.Listen(KListenQueueSize));

    }

void CSocketListener::DoCancel()
    {
    // Cancel any outstanding request
    iListeningSocket.CancelAccept();
    }

void CSocketListener::RunL()
    {
    TInt err = iStatus.Int();

        TRACE_INFO((_L("[BTHID]\tCSocketListener::RunL(), iStatus.Int()=%d"),err));

    if (err == KErrCancel)
        return; //Active task was cancelled.

    // In case of KErrHardwareNotAvailable,  cancel and reset the listening
    // socket and reissue the Accept is needed.
    if (err == KErrHardwareNotAvailable)
        {
        iListeningSocket.CancelAll();

        err = iListeningSocket.Listen(KListenQueueSize);

        if (err == KErrNone)
            {
            iListeningSocket.Accept(*iAcceptSocket, iStatus);
            SetActive();
            }
        else
            {
            // Report the result to the observer.
            iObserver.SocketAccepted(iPort, err);
            }
        }
    else
        {
        // Report the result to the observer.
        iObserver.SocketAccepted(iPort, err);
        }
    }

TInt CSocketListener::AcceptConnection(RSocket& aSocket)
    {

        TRACE_INFO((_L("[BTHID]\tCSocketListener::AcceptConnection()")));

    // If we aren't in the correct state to accept, panic.
    __ASSERT_DEBUG(!IsActive(),
            User::Panic(KPanicBTConnection, ESocketsBadState));

    // close old connection - if any
    aSocket.Close();

    // Open abstract socket
    TInt err = aSocket.Open(iSocketServer);

    if (err == KErrNone)
        {

            TRACE_INFO((_L("[BTHID]\tCSocketListener::AcceptConnection(), opening new socket succeed")));
        // Accept onto the new socket
        iListeningSocket.Accept(aSocket, iStatus);

        // Keep a handle on the accept socket.
        iAcceptSocket = &aSocket;

        SetActive();
        }
    else
        TRACE_INFO((_L("[BTHID]\tCSocketListener::AcceptConnection(), opening new socket failed")));

    return err;
    }

//End of file

