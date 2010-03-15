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


#include <e32svr.h>
#include <bttypes.h>
#include "bthidsession.h"
#include "bthidconnection.h"
#include "bthidserver.h"
#include "bthidclientsrv.h"
#include "hidsdpclient.h"
#include "bthiddevice.h"
#include "bthidserver.pan"
#include "debug.h"

CBTHidServerSession::CBTHidServerSession(CBTHidServer& aServer) :
    iServer(aServer)
    {
    // Implementation not required
    }

CBTHidServerSession::~CBTHidServerSession()
    {
    iServer.DecrementSessions();
    delete iHidSdpClient;
    }

void CBTHidServerSession::ServiceL(const RMessage2& aMessage)
    {
    TRAPD( err, DispatchMessageL( aMessage ) );

    if (aMessage.Function() == EBTHIDGetConnections && !aMessage.IsNull())
        {
        // Handle the error code, what to to?
        err = err;
        }
    }

void CBTHidServerSession::DispatchMessageL(const RMessage2& aMessage)
    {
    switch (aMessage.Function())
        {
        case EBTHIDServConnectDevice:
            RequestConnectionL(aMessage);
            break;

        case EBTHIDServCancelConnect:
            if (iConnectionMessage.Handle())
                {
                delete iHidSdpClient;
                iHidSdpClient = 0;

                iServer.DeleteNewConnection(iConnectingID);

                iConnectionMessage.Complete(KErrCancel);
                }
            aMessage.Complete(KErrNone);
            break;

        case EBTHIDServNotifyConnectionChange:
            if (iUpdateMessage.Handle())
                {
                // We're already busy
                CBTHidServer::PanicClient(aMessage, EReqAlreadyPending);
                }
            else
                {
                iUpdateMessage = aMessage;
                }
            break;

        case EBTHIDServCancelNotify:
            if (iUpdateMessage.Handle())
                {
                iUpdateMessage.Complete(KErrCancel);
                }
            aMessage.Complete(KErrNone);
            break;

        case EBTHIDServDisconnectDevice:
            RequestDisconnectionL(aMessage);
            break;

        case EBTHIDServIsConnected:
            IsConnectedL(aMessage);
            break;

        case EBTHidSrvDisconnectAllGracefully:
            if (iDisconnectMsg.Handle())
                {
                aMessage.Complete(KErrServerBusy);
                }
            else
                {
                iDisconnectMsg = aMessage;
                iServer.DisconnectAllDeviceL();
                iDisconnectMsg.Complete(KErrNone);
                }
            break;

        case EBTHIDGetConnections:
            GetConnections(aMessage);
            break;

        default:
            {
            TRACE_INFO( ( _L( "[BTHID]\t DispatchMessageL: bad request (%d)" ), 
                                aMessage.Function() ) )
            CBTHidServer::PanicClient(aMessage, EBadRequest);
            }
            break;
        }
    }

void CBTHidServerSession::HidSdpSearchComplete(TInt aResult)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidServerSession::HidSdpSearchComplete"));
    // This is a callback for the Hid SDP client so we can't delete it here
    // Get it to destroy itself when its convenient.
    iHidSdpClient->Kill();
    // Deleted outside destructor.
    iHidSdpClient = 0;

    TInt err = aResult;

    // If the SDP search was a success
    if (err == KErrNone)
        {
            // Try to connect to the device as a HID
            TRAP( err, iServer.DoFirstConnectionL(iConnectingID); )

        if (err == KErrAlreadyExists)
            {
            //there is already an active connection with the same CoD
            TBuf8<KBTDevAddrSize> buf;
            TBTDevAddr dummyAddr = iServer.ConflictAddress();
            buf.Append(dummyAddr.Des());
            if (buf.Length())
                {
                iConnectionMessage.Write(1, buf);
                }

            //delete the connection object in the server
            iServer.DeleteNewConnection(iConnectingID);
            iConnectionMessage.Complete(err);
            return;
            }
        }

    // Any error causes clean up,
    if (err != KErrNone)
        {
        //delete the connection object in the server
        iServer.DeleteNewConnection(iConnectingID);

        //Inform the client of the result
        iConnectionMessage.Complete(aResult);
        }
    }

void CBTHidServerSession::RequestConnectionL(const RMessage2& aMessage)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidServerSession::RequestConnectionL"));

    if (iConnectionMessage.Handle())
        {
        // We're already busy
        CBTHidServer::PanicClient(aMessage, EReqAlreadyPending);
        }
    else
        {
            TRACE_INFO(_L("[BTHID]\tCBTHidServerSession::RequestConnectionL continue to connect"));
        // Keep a copy of message - for use later
        iConnectionMessage = aMessage;

        // Read the connection parameters
        TBTDevAddr addr;
        TPckg<TBTDevAddr> addrPack(addr);

        iConnectionMessage.ReadL(0, addrPack);
        
        // Ask the server to create a new connection object
        iConnectingID = iServer.NewConnectionL(EConnecting);
        
        TRAPD( res,
                // Retrieve the hid device object for this new connection
                CBTHidDevice &devDetails =
                iServer.ConnectionDetailsL(iConnectingID);

                // Fill in the information we got from the client
                devDetails.iAddress = addr;
                devDetails.iUseSecurity = ETrue;

                // Create a new HID Sdp Client
                // Its only used here so it doesn't matter if we leave.
                delete iHidSdpClient;
                iHidSdpClient = 0;
                //Create a new hid sdp client using the hid device object.
                iHidSdpClient = CHidSdpClient::NewL(devDetails, *this);

                // Start the hid sdp client
                iHidSdpClient->StartL();
        )

        // If there was any error, inform the client and finish
        if (res != KErrNone)
            {
            // Get the server to delete the new connection object
            iServer.DeleteNewConnection(iConnectingID);
            aMessage.Complete(res);
            }
        }

    }

void CBTHidServerSession::RequestDisconnectionL(const RMessage2& aMessage)
    {
    TBTDevAddr addr;
    TPckg<TBTDevAddr> addrPack(addr);

    // Read the address from the client thread.
    aMessage.ReadL(0, addrPack);

    // Since the user can select this option using the BT Keyboard
    // and the keydown event triggers it, depending on how long they
    // hold down the key, the connection could be restored to send
    // the key up event.
    // Add a small delay, sensible enough for a press of the enter
    // key before disconnection.
    // Currently 3/4 of a second.
    User::After(750000);

    //Keep the connection alive, initial reconnect from HID device available...
    iServer.CloseBluetoothConnection(addr);

    aMessage.Complete(KErrNone);
    }

void CBTHidServerSession::InformStatusChange(
        const THIDStateUpdateBuf& aUpdateParams)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidServerSession::InformStatusChange"));

    THIDStateUpdate update = aUpdateParams();
    TBTHidConnState state = update.iState;

    TBuf8<KBTDevAddrSize> buf;
    if (state == EBTDeviceAnotherExist)
        {
        TBTDevAddr iDumyAddr = iServer.ConflictAddress();
        buf.Append(iDumyAddr.Des());
        }

        TRACE_INFO( (_L("[BTHID]\tCBTHidServerSession::InformStatusChange, state=%d"), state) );

    if (iUpdateMessage.Handle())
        {
            TRACE_FUNC
        (_L("[BTHID]\tCBTHidServerSession::InformStatusChange, OK - send message"));
        // Trap any error
        TRAPD( res,
                iUpdateMessage.WriteL(0,aUpdateParams);
                if (buf.Length())
                    {
                    iUpdateMessage.WriteL(1,buf);
                    }
        )

        if (state == EBTDeviceAnotherExist)
            {
            iUpdateMessage.Complete(KErrAlreadyExists);
            }
        else
            {
            // Complete the message, with the error code from the trap.
            iUpdateMessage.Complete(res);
            }
        }
    }

void CBTHidServerSession::InformConnectionResult(TInt aConnID, TInt aResult)
    {
        TRACE_INFO( (_L("[BTHID new]\tCBTHidServerSession::InformConnectionResult, Ids: %d==%d aResult = %d"),
                        iConnectingID, aConnID, aResult) );
    if ((iConnectionMessage.Handle()) && (iConnectingID == aConnID))
        {
        iConnectionMessage.Complete(aResult);
        }
    }

void CBTHidServerSession::IsConnectedL(const RMessage2& aMessage)
    {
    TBTDevAddr addr;
    TPckg<TBTDevAddr> addrPck(addr);
    aMessage.ReadL(0, addrPck);
    TBTEngConnectionStatus ret = iServer.ConnectStatus(addr);

    aMessage.Complete(ret);
    }

void CBTHidServerSession::GetConnections(const RMessage2& aMessage)
    {
        TRACE_FUNC
    TBuf8<KBTDevAddrSize * 2> buf;
    buf.Zero();
    iServer.GetConnectionAddress(buf);
    if (buf.Length())
        {
        aMessage.Write(0, buf);
        }

    aMessage.Complete(KErrNone);
    }

CBTHidServerSession* CBTHidServerSession::NewL(CBTHidServer& aServer)
    {
    CBTHidServerSession* self = CBTHidServerSession::NewLC(aServer);
    CleanupStack::Pop(self);
    return self;
    }

CBTHidServerSession* CBTHidServerSession::NewLC(CBTHidServer& aServer)
    {
    CBTHidServerSession* self = new (ELeave) CBTHidServerSession(aServer);

    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

void CBTHidServerSession::ConstructL()
    {
    iServer.IncrementSessions();
    }
