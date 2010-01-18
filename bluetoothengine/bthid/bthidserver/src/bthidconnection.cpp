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
#include "bthidconnection.h"
#include "btconnectionobserver.h"
#include "sockets.pan"
#include "socketinitiator.h"
#include "socketreader.h"
#include "socketwriter.h"
#include "datasegmenter.h"
#include "timeouttimer.h"
#include "bthidtypes.h"
#include "bthiddevice.h"
#include "hidinterfaces.h"
#include "debug.h"

CBTHidConnection* CBTHidConnection::NewL(RSocketServ& aSocketServ,
        MBTConnectionObserver& aObserver, TBTConnectionState aConnectionState)
    {
    CBTHidConnection* self = CBTHidConnection::NewLC(aSocketServ, aObserver,
            aConnectionState);
    CleanupStack::Pop(self);
    return self;
    }

CBTHidConnection* CBTHidConnection::NewLC(RSocketServ& aSocketServ,
        MBTConnectionObserver& aObserver, TBTConnectionState aConnectionState)
    {
    CBTHidConnection* self = new (ELeave) CBTHidConnection(aSocketServ,
            aObserver, aConnectionState);
    // This is a CObject derived class so must be closed to destroy it.
    CleanupClosePushL(*self);
    self->ConstructL();
    return self;
    }

CBTHidConnection::CBTHidConnection(RSocketServ& aSocketServ,
        MBTConnectionObserver& aObserver, TBTConnectionState aConnectionState) :
    iSocketServ(aSocketServ), iObserver(aObserver)
    {
    ChangeState(aConnectionState);
    }

CBTHidConnection::~CBTHidConnection()
    {
    delete iControlSocketReader;

    delete iControlSocketWriter;

    delete iInterruptSocketReader;

    delete iInterruptSocketWriter;

    delete iSocketInitiator;

    delete iCommandSegmenter;

    delete iControlDataBuffer;

    delete iInterruptDataBuffer;

    delete iInactivityTimer;

    delete iDevice;

    // Close connections in the reverse order to when they were opened
    if (iInterruptSocket)
        {
        iInterruptSocket->Close();
        delete iInterruptSocket;
        }

    if (iControlSocket)
        {
        iControlSocket->Close();
        delete iControlSocket;
        }
    }

TInt CBTHidConnection::ConnID()
    {
    return iConnID;
    }

void CBTHidConnection::SetConnID(TInt aConnID)
    {
    // We shouldn't be trying to set the id for this connection
    // after it is connected.
    __ASSERT_DEBUG(iConnectionState == ENotConnected || iConnectionState == EConnecting,
            User::Panic(KPanicBTConnection, ESocketsBadState));

    iConnID = aConnID;
    }

void CBTHidConnection::ConstructL()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ConstructL"));
    //ChangeState(ENotConnected);

    // A new hid device information object.
    iDevice = CBTHidDevice::NewL();

    // Socket readers and writer objects to perform asynchronous requests
    iControlSocketReader = CSocketReader::NewL(EControlSocketID, *this,
            KL2CAPDefaultMTU);

    iControlSocketWriter = CSocketWriter::NewL(EControlSocketID, *this);

    iInterruptSocketReader = CSocketReader::NewL(EInterruptSocketID, *this,
            KL2CAPDefaultMTU);

    iInterruptSocketWriter = CSocketWriter::NewL(EInterruptSocketID, *this);

    // Socket initiator to perform socket connections
    iSocketInitiator = CSocketInitiator::NewL(iSocketServ, *this);

    // Create a timeout timer
    iInactivityTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
    }

void CBTHidConnection::ConnectL()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ConnectL"));
    // If we aren't in the correct stiDeviceate for an initial connection Panic.
    __ASSERT_DEBUG(iConnectionState == ENotConnected || iConnectionState == EConnecting,
            User::Panic(KPanicBTConnection, ESocketsBadState));

    // Create the two sockets.

    delete iControlSocket;
    iControlSocket = 0;
    iControlSocket = new (ELeave) RSocket;

    // If we leave after this iControlSocket will not be NULL.
    // This is ok since any place where it can be set will delete it first
    // or it will be finally deleted in the destructor
    delete iInterruptSocket;
    iInterruptSocket = 0;
    iInterruptSocket = new (ELeave) RSocket;

    // If we leave after this iControlSocket & iInterruptSocket will not
    // be NULL.
    // This is ok since any place where they can be set will delete first
    // or they will be finally deleted in the destructor

    // Ask the socket initiator to connect these sockets,
    // giving it the bluetooth address of the device and a security flag.
    iSocketInitiator->ConnectSocketsL(iDevice->iAddress,
            iDevice->iUseSecurity, iControlSocket, iInterruptSocket);
    ChangeState(EFirstConnection);
    }

void CBTHidConnection::ReconnectL()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ReconnectL"));
    // If we aren't in the correct state for an reconnection Panic.
    __ASSERT_DEBUG(iConnectionState == ENotConnected,
            User::Panic(KPanicBTConnection, ESocketsBadState));

    // Expect the device to reconnect for now
    ChangeState(ELinkLost);
    }

void CBTHidConnection::Disconnect() //Virtual Cable Unplug. (this is not Bluetooth disconnect)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::Disconnect (Virtual Cable Unplug)"));
    if (iConnectionState == EConnected)
        {
        //First Send VCUnplug
        iCommandBuffer.Zero();
        iCommandBuffer.Append(EHIDControlVCUnplug);

        TRequestStatus status;
        iControlSocket->Write(iCommandBuffer, status);
        User::WaitForRequest(status);

        //Then wait for a reply from Su-8W on L2CAP channel(s). If not waited, Su-8W will go mad.
        User::After(500000); //0.5 sec

        //but we never handle that acknowledgement (because it is not required by BT HID
        //specification, but Su-8W needs to send something.)
        iControlSocketWriter->Cancel();
        iControlSocketReader->Cancel();
        iInterruptSocketReader->Cancel();
        iInterruptSocketWriter->Cancel();
        }
    }

void CBTHidConnection::OfferControlSocket(const TBTDevAddr& aAddress,
        RSocket*& aSocket)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::OfferControlSocket"));
    if (aAddress == iDevice->iAddress)
        {
        __ASSERT_DEBUG((iConnectionState == ELinkLost) ||
                (iConnectionState == EHIDReconnecting),
                User::Panic(KPanicBTConnection, ESocketsBadState));

        // Take ownership of this socket
        delete iControlSocket;
        iControlSocket = aSocket;
        aSocket = 0;

        // Mark that the HID Device is reconnecting to us.
        ChangeState(EHIDReconnecting);
        }
    }

void CBTHidConnection::OfferInterruptSocket(const TBTDevAddr& aAddress,
        RSocket*& aSocket)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::OfferInterruptSocket"));

    if (aAddress == iDevice->iAddress)
        {
        __ASSERT_DEBUG((iConnectionState == EHIDReconnecting), //||(iConnectionState == ELinkLost) ,
                User::Panic(KPanicBTConnection, ESocketsBadState));

        // Take ownership of this socket
        delete iInterruptSocket;
        iInterruptSocket = aSocket;
        aSocket = 0;
        TRAPD(error, PrepareSocketsL());

        if (KErrNone == error)
            {
            // Mark that we are now reconnected.

            ChangeState(EConnected);

            // Inform the observer that the connection has been restored.
            iObserver.LinkRestored(iConnID);

            }
        else
            {
            // Close the sockets as they can't be used
            CloseChannels();
            ChangeState(ELinkLost);
            }
        }
    }

CBTHidDevice& CBTHidConnection::DeviceDetails()
    {
    return (*iDevice);
    }

TBool CBTHidConnection::IsConnected() const
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidConnection::IsConnected: connection state = %d"), iConnectionState) );
    return (EConnected == iConnectionState);
    }

TBTConnectionState CBTHidConnection::ConnectStatus() const
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidConnection::ConnectStatus: connection state = %d"), iConnectionState) );
    return (iConnectionState);
    }

void CBTHidConnection::StartMonitoringChannelsL()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::StartMonitoringChannelsL"));
    __ASSERT_DEBUG(iConnectionState == EConnected,
            User::Panic(KPanicBTConnection, ESocketsBadState));

    iControlSocketReader->StartReadingL(iControlSocket, iControlInMTU);
    iInterruptSocketReader->StartReadingL(iInterruptSocket, iInterruptInMTU);

    // If the device will reconnect to us we are ok to drop the link
    // after a period of inactivity.
    if (iDevice->iReconnectInit)
        {
        iInactivityTimer->Cancel();
        iInactivityTimer->After(KInactivityTimeout);
        }
    }

void CBTHidConnection::DropConnection()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::DropConnection"));
    // Close the Bluetooth Channels.
    CloseChannels();

    // Update the connection state.
    ChangeState(ELinkLost);

    // If a command is outstanding
    if (iCommandIssued)
        {
        // Generate an error to the parent.
        iObserver.HandleCommandAck(iConnID, KErrNotReady);
        // Reset the command flag.
        iCommandIssued = EFalse;

        // Reset this, so we don't leave it in a bad state.
        if (iCommandSegmenter)
            {
            iCommandSegmenter->Reset();
            }
        }
    }

// from MSocketObserver
void CBTHidConnection::HandleSocketError(TUint /*aSocketID*/,
        TBool aConnectionLost, TInt aErrorCode)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidConnection::HandleSocketError: connlost %d, error code %d"), aConnectionLost, aErrorCode) );
    if (aConnectionLost)
        {
        ConnectionLost();
        }
    else
        {
        // If we failed to write to the device then inform Generic HID
        // of the failure
        if (iCommandIssued)
            {
            iCommandIssued = EFalse;

            // Reset this, so we don't leave it in a bad state.
            if (iCommandSegmenter)
                {
                iCommandSegmenter->Reset();
                }

            iObserver.HandleCommandAck(iConnID, aErrorCode);
            }
        }
    }

// from MSocketObserver
TBool CBTHidConnection::HandleDataReceived(TUint aSocketID,
        const TDesC8& aBuffer)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::HandledataReceived"));
    TBool result = ETrue;

    // Cancel the inactivity timer.
    iInactivityTimer->Cancel();

    switch (aSocketID)
        {
        case EControlSocketID:
            result = ProcessControlData(aBuffer);
            break;
        case EInterruptSocketID:
            result = ProcessInterruptData(aBuffer);
            break;
        default:
            // Shouldn't have any other socket id
            User::Panic(KPanicBTConnection, ESocketsUnknownID);
            break;
        }

    // If the device will reconnect to us we are ok to drop the link
    // after a period of inactivity.
    if ((result) && (iDevice->iReconnectInit))
        {
        iInactivityTimer->After(KInactivityTimeout);
        }

    return result;
    }

// from MSocketObserver
void CBTHidConnection::HandleWriteComplete(TUint aSocketID)
    {
        //to handle DATA Output in interrupt channel (Host to Device DATA)
        // Check the ID of the socket
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::HandleWriteComplete"));
    if (aSocketID == EControlSocketID || aSocketID == EInterruptSocketID)
        {
        // Send any additional packets on the control channel
        if (iCommandSegmenter)
            {
            const HBufC8* data = iCommandSegmenter->NextPacket();
            if (data)
                {
                if (aSocketID == EControlSocketID)
                    {
                    TRAPD(err, iControlSocketWriter->IssueWriteL(*data));
                    if (KErrNone != err)
                        {
                        // Reset this, so we don't leave it in a bad state.
                        iCommandSegmenter->Reset();
                        iCommandIssued = EFalse;

                        iObserver.HandleCommandAck(iConnID, err);
                        }
                    }
                else //aSocketID == EInterruptSocketID
                    {
                    TRAPD(err, iInterruptSocketWriter->IssueWriteL(*data));
                    if (KErrNone != err)
                        {
                        // Reset this, so we don't leave it in a bad state.
                        iCommandSegmenter->Reset();
                        iCommandIssued = EFalse;

                        iObserver.HandleCommandAck(iConnID, err);
                        }
                    }
                }
            if (!data && aSocketID == EInterruptSocketID && iCommandIssued)
                { //We don't expect response from HID Device, However we'll notify GenericHID
                //that async write operation has been finished.
                iObserver.HandleCommandAck(iConnID, KErrNone); //Socket write complete!
                iCommandIssued = EFalse;
                }
            }
        }

    }

void CBTHidConnection::SocketsConnected()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::SocketsConnected"));
    TRAPD(error, PrepareSocketsL());

    if (KErrNone == error)
        {
        switch (iConnectionState)
            {
            case EFirstConnection:
                // We are now connected

                ChangeState(EConnected);

                // If this was an initial connection inform the observer
                iObserver.FirstTimeConnectionComplete(iConnID, KErrNone);
                break;

            case EHostReconnecting:
                // We are now connected
                ChangeState(EConnected);

                iObserver.LinkRestored(iConnID);
                break;

            default:
                User::Panic(KPanicBTConnection, ESocketsBadState);
                break;
            }
        }
    else
        {
        SocketsConnFailed(error);
        }
    }

void CBTHidConnection::SocketsConnFailed(TInt aStatus)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidConnection::SocketsConnFailed(%d)"),aStatus) );
    switch (iConnectionState)
        {
        case EFirstConnection:
            // We are not connected
            ChangeState(ENotConnected);
            // If this was an initial connection inform the observer
            iObserver.FirstTimeConnectionComplete(iConnID, aStatus);
            break;

        case EHostReconnecting:
            ChangeState(ELinkLost);
            break;

        default:
            // we don't expect to be connecting in any other state.
            User::Panic(KPanicBTConnection, ESocketsBadState);
            break;
        }
    }

void CBTHidConnection::TimerExpired()
    {
    // Inactivity timer has expired.
    // Drop the connection.
    DropConnection();
    // Inform the observer of the state change.
    iObserver.Disconnected(iConnID);
    }

void CBTHidConnection::GetProtocolL()
    {
    LeaveIfCommandNotReadyL();

    iCommandBuffer.Zero();
    iCommandBuffer.Append(EGetProtocol);
    iControlSocketWriter->IssueWriteL(iCommandBuffer);
    iCommandIssued = ETrue;
    }

void CBTHidConnection::SetProtocolL(TUint16 aValue)
    {
    LeaveIfCommandNotReadyL();

    iCommandBuffer.Zero();
    // Value should be
    // 0 for boot
    // 1 for report
    // ESetProtocol is set for Boot mode. Add aValue to get the correct
    // command
    iCommandBuffer.Append(ESetProtocol + aValue);
    iControlSocketWriter->IssueWriteL(iCommandBuffer);
    iCommandIssued = ETrue;
    }

void CBTHidConnection::GetReportL(TUint8 aReportType, TUint8 aReportID,
        TUint16 aLength)
    {
    LeaveIfCommandNotReadyL();

    iCommandBuffer.Zero();

    // Add the Transaction header byte
    // Report Type 0 = Reserved
    // Report Type 1 = Input
    // Report Type 2 = Output
    // Report Type 3 = Feature
    iCommandBuffer.Append(EGetReportFullBufReserved + aReportType);

    // If Report ID's are declared in the report descriptor then we
    // add this field
    if (aReportID != 0)
        {
        iCommandBuffer.Append(aReportID);
        }

    // We need to check we have a buffer large enough to hold the control
    // data we get back from the device
    if (!iControlDataBuffer)
        {
        // Allocate the buffer if it didn't exist
        iControlDataBuffer = HBufC8::NewL(aLength);
        }
    else
        {
        // Get a modifiable pointer to the buffer.
        TPtr8 dataPtr = iControlDataBuffer->Des();

        if (dataPtr.MaxLength() < aLength)
            {
            // Reallocate the buffer if its too small.
            delete iControlDataBuffer;
            iControlDataBuffer = 0;
            iControlDataBuffer = HBufC8::NewL(aLength);
            }
        else
            {
            // Existing buffer is large enough, just zero it.
            dataPtr.Zero();
            }
        }

    iControlSocketWriter->IssueWriteL(iCommandBuffer);
    iCommandIssued = ETrue;
    }

void CBTHidConnection::SetReportL(TUint8 aReportType, TUint8 aReportID,
        const TDesC8& aReport)
    {
    LeaveIfCommandNotReadyL();

    if (!iCommandSegmenter)
        {
        iCommandSegmenter = CDataSegmenter::NewL();
        }

    iCommandBuffer.Zero();

    // Add the Transaction header byte
    // Report Type 0 = Reserved
    // Report Type 1 = Input
    // Report Type 2 = Output
    // Report Type 3 = Feature
    iCommandBuffer.Append(ESetReportReserved + aReportType);

    // If Report ID's are declared in the report descriptor then we
    // add this field
    if (aReportID != 0)
        {
        iCommandBuffer.Append(aReportID);
        }

    iCommandSegmenter->SegmentDataL(iCommandBuffer, aReport, EDATCOutput,
            iControlOutMTU);

    iControlSocketWriter->IssueWriteL(*(iCommandSegmenter->FirstPacketL()));

    iCommandIssued = ETrue;
    }

void CBTHidConnection::DataOutL(TUint8 aReportID, const TDesC8& aReport)
    {
    //This implementation is similar to SetReportL, but instead of Control channel,
    //sends the data in Interrupt channel.

    LeaveIfCommandNotReadyL();
    //We'll use Command buffer instead of Data buffer to send.
    //could they happen at the same time?
    if (!iCommandSegmenter)
        {
        iCommandSegmenter = CDataSegmenter::NewL();
        }

    iCommandBuffer.Zero();

    // Add the Transaction header byte
    // Report Type 0 = Reserved
    // Report Type 1 = Input
    // Report Type 2 = Output
    // Report Type 3 = Feature
    iCommandBuffer.Append(EDATAOutput);

    // If Report ID's are declared in the report descriptor then we
    // add this field
    if (aReportID != 0)
        {
        iCommandBuffer.Append(aReportID);
        }

    iCommandSegmenter->SegmentDataL(iCommandBuffer, aReport, EDATCOutput,
            iInterruptOutMTU);

    iInterruptSocketWriter->IssueWriteL(*(iCommandSegmenter->FirstPacketL()));

    iCommandIssued = ETrue; //We have sent the data but we don't expect
    //HANDSHAKE SUCCESSFULL for DATA OUT from HID device.
    //We will notify Generic HID when data has been sent.
    }

void CBTHidConnection::GetIdleL()
    {
    LeaveIfCommandNotReadyL();

    iCommandBuffer.Zero();
    iCommandBuffer.Append(EGetIdle);
    iControlSocketWriter->IssueWriteL(iCommandBuffer);
    iCommandIssued = ETrue;
    }

void CBTHidConnection::SetIdleL(TUint8 aDuration)
    {
    LeaveIfCommandNotReadyL();

    iCommandBuffer.Zero();
    iCommandBuffer.Append(ESetIdle);
    iCommandBuffer.Append(aDuration);
    iControlSocketWriter->IssueWriteL(iCommandBuffer);
    iCommandIssued = ETrue;
    }

void CBTHidConnection::PrepareSocketsL()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::PrepareSockets"));
    // Retrieve the MTU values from the sockets
    User::LeaveIfError(iControlSocket->GetOpt(KL2CAPGetOutboundMTU,
            KSolBtL2CAP, iControlOutMTU));
    User::LeaveIfError(iControlSocket->GetOpt(KL2CAPInboundMTU, KSolBtL2CAP,
            iControlInMTU));
    User::LeaveIfError(iInterruptSocket->GetOpt(KL2CAPInboundMTU,
            KSolBtL2CAP, iInterruptInMTU));
    User::LeaveIfError(iInterruptSocket->GetOpt(KL2CAPGetOutboundMTU,
            KSolBtL2CAP, iInterruptOutMTU));

    // Initialise the control socket writer with the new socket.
    iControlSocketWriter->Initialise(iControlSocket);

    // Initialise the interrupt socket writer with the new socket.
    iInterruptSocketWriter->Initialise(iInterruptSocket);

    }

void CBTHidConnection::ChangeState(TBTConnectionState aNewStatus)
    {
    iConnectionState = aNewStatus;
    }

void CBTHidConnection::ConnectionLost()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ConnectionLost"));
    CloseChannels();

    // First go into link loss state
    ChangeState(ELinkLost);

    // If a command is outstanding
    if (iCommandIssued)
        {
            TRACE_INFO(_L("[BTHID]\tCBTHidConnection::ConnectionLost, command outstanding"));
        // Generate an error to the parent.
        iObserver.HandleCommandAck(iConnID, KErrNotReady);
        // Reset the command flag.
        iCommandIssued = EFalse;

        // Reset this, so we don't leave it in a bad state.
        if (iCommandSegmenter)
            {
            iCommandSegmenter->Reset();
            }
        }

    // Check if the device will reconnect to us.
    if (iDevice->iReconnectInit)
        {
            TRACE_INFO(_L("[BTHID]\tCBTHidConnection::ConnectionLost, device inits reconnect"));
        // Inform the parent of the link loss and the fact we are not
        // reconnecting
        iObserver.LinkLost(iConnID);
        }
    else
        {
            TRACE_INFO(_L("[BTHID]\tCBTHidConnection::ConnectionLost, host inits reconnect"));
        // Device won't reconnect, check if we are able to.
        if (iDevice->iNormallyConnectable)
            {
                TRACE_INFO(_L("[BTHID]\tCBTHidConnection::ConnectionLost, device is normally connectable"));
            // Attempt to initiate reconnection to the device.
            TRAPD(res, iSocketInitiator->ConnectSocketsL(iDevice->iAddress,
                            iDevice->iUseSecurity,
                            iControlSocket,
                            iInterruptSocket);)
            if (res == KErrNone)
                {
                // Reconnection is in progress, so record this and inform
                // the parent.
                ChangeState(EHostReconnecting);
                iObserver.LinkLost(iConnID);
                }
            else
                {
                // Inform the parent of the link loss and the fact we are not
                // reconnecting
                iObserver.LinkLost(iConnID);
                }
            }
        else
            {
                TRACE_INFO(_L("[BTHID]\tCBTHidConnection::ConnectionLost, device does not allow reconnection"));
            // Device won't allow reconnection, so this connection is
            // effectively dead.
            // Change to not-connected and inform the parent of the
            // disconnection.
            ChangeState(ENotConnected);
            iObserver.Unplugged(iConnID);
            }
        }
    }

void CBTHidConnection::ReceivedVirtualCableUnplug()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ReceivedVirtualCableUnplug"));
    CloseChannels();
    ChangeState(ENotConnected);

    iObserver.Unplugged(iConnID);
    }

void CBTHidConnection::LeaveIfCommandNotReadyL() const
    {
    if (iConnectionState != EConnected)
        {
        User::Leave(KErrNotReady);
        }

    if (iCommandIssued)
        {
        User::Leave(KErrInUse);
        }
    }

void CBTHidConnection::CloseChannels()
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::CloseChannels"));
    // Cancel the inactivity timer.
    iInactivityTimer->Cancel();

    iControlSocketReader->Cancel();
    iControlSocketWriter->Cancel();
    iInterruptSocketReader->Cancel();
    iInterruptSocketWriter->Cancel();

    // BT HID Spec. says the channels should be closed in the reverse
    // order to when they were opened
    if (iInterruptSocket)
        {
        if (iConnectionState == EConnected)
            {
            // Immediate shutdown
            TRequestStatus status;
            iInterruptSocket->Shutdown(RSocket::EImmediate, status);
            User::WaitForRequest(status);
            }

        iInterruptSocket->Close();
        }

    if (iControlSocket)
        {
        if (iConnectionState == EConnected)
            {
            // Immediate shutdown
            TRequestStatus status;
            iControlSocket->Shutdown(RSocket::EImmediate, status);
            User::WaitForRequest(status);
            }

        iControlSocket->Close();
        }

    }

TBool CBTHidConnection::ProcessControlData(const TDesC8& aBuffer)
    {
    TBool result = ETrue;

        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ProcessControlData"));
    // Reset this, so we don't leave it in a bad state;
    if (iCommandSegmenter)
        {
        iCommandSegmenter->Reset();
        }

    TInt dataLength = aBuffer.Length();

    if (dataLength > 0)
        {
        TBTTransaction transaction = static_cast<TBTTransaction> (aBuffer[0]);

        // The only unsolicited response should be a Virtual Cable Unplugged
        // notification.
        // All other responses should only be handled, if we have an
        // outstanding command
        if (transaction == EHIDControlVCUnplug)
            {
            ReceivedVirtualCableUnplug();
            // Don't want to read any more
            result = EFalse;
            }
        else if (iCommandIssued)
            {
            // Reset the command issued flag.
            iCommandIssued = EFalse;

                TRACE_INFO( (_L("[BTHID]\tCBTHidConnection::ProcessControlData: transaction %d"), transaction) );
            switch (transaction)
                {
                case EHandshakeSuccess:
                    iObserver.HandleCommandAck(iConnID, KErrNone);
                    break;
                case EHandshakeNotReady:
                    iObserver.HandleCommandAck(iConnID, KErrNotReady);
                    break;
                case EHandshakeInvalidRepID:
                    iObserver.HandleCommandAck(iConnID,
                            KErrAckInvalidReportID);
                    break;
                case EHandshakeUnsupported:
                    iObserver.HandleCommandAck(iConnID, KErrNotSupported);
                    break;
                case EHandshakeInvalidParam:
                    iObserver.HandleCommandAck(iConnID,
                            KErrAckInvalidParameter);
                    break;
                case EHandshakeUnknown:
                    iObserver.HandleCommandAck(iConnID, KErrAckUnknown);
                    break;
                case EHandshakeFatal:
                    iObserver.HandleCommandAck(iConnID, KErrAckFatal);
                    break;

                case EDATAInput:
                    // Fall through.
                case EDATAFeature:
                    // If this packet was smaller than the maximum transmission
                    // unit this is the only data we will get.
                    if (dataLength < iControlInMTU)
                        {
                        // Pass to the observer
                        iObserver.HandleControlData(iConnID, aBuffer.Mid(1));
                        }
                    else
                        {
                        // Data was the size of the MTU, so we will expect
                        // further DATC packets.

                        // Mark that the command hasn't finished
                        iCommandIssued = ETrue;

                        // Store as much data as we can in the buffer allocated
                        if (iControlDataBuffer)
                            {
                            TPtr8 dataPtr = iControlDataBuffer->Des();
                            dataPtr.Zero();
                            AppendData(dataPtr, aBuffer.Mid(1));
                            }
                        }
                    break;

                case EDATCInput:
                    // Fall through.
                case EDATCFeature:
                    // If this packet was smaller than the maximum transmission
                    // this is the final packet.
                    HandleEDATCFeature(aBuffer);
                    break;

                case EDATAOther:
                    iObserver.HandleControlData(iConnID, aBuffer.Mid(1));
                    break;

                default:
                    // This transaction was unknown or unexpected, but
                    // we must report the error.
                    iObserver.HandleCommandAck(iConnID, KErrAckUnknown);
                    break;
                }
            }
        }

    return result;
    }

void CBTHidConnection::HandleEDATCFeature(const TDesC8& aBuffer)
    {
    TInt dataLength = aBuffer.Length();
    if (dataLength < iControlInMTU)
        {
        // If there is a data buffer.
        if (iControlDataBuffer)
            {
            // Get a modifiable pointer to the data
            TPtr8 dataPtr = iControlDataBuffer->Des();

            // If there is already some data then we can
            // append.
            if (dataPtr.Length() > 0)
                {
                // Append as much as we can.
                AppendData(dataPtr, aBuffer.Mid(1));

                // Pass up what data we have.
                iObserver.HandleControlData(iConnID, *iControlDataBuffer);
                }
            else
                {
                // No initial data was stored, so report an
                // error.
                iObserver.HandleCommandAck(iConnID, KErrAckUnknown);
                }
            }
        else
            {
            // There is no buffer to handle the data so just
            // report an error.
            iObserver.HandleCommandAck(iConnID, KErrNoMemory);
            }
        }
    else
        {
        // This is an intermediate packet.

        // Mark that the command hasn't finished
        iCommandIssued = ETrue;

        if (iControlDataBuffer)
            {
            // Get a modifiable pointer to the data.
            TPtr8 dataPtr = iControlDataBuffer->Des();

            // If there is already some data then we can
            // append.
            if (dataPtr.Length() > 0)
                {
                // Append as much as we can.
                AppendData(dataPtr, aBuffer.Mid(1));
                }
            }
        }

    }

void CBTHidConnection::AppendData(TDes8& aDest, const TDesC8& aSource)
    {
    TInt remainingSpace = aDest.MaxLength() - aDest.Length();

    // If the control data buffer can handle this new packet
    if (aSource.Length() <= remainingSpace)
        {
        // Append the data
        aDest.Append(aSource);
        }
    }

TBool CBTHidConnection::ProcessInterruptData(const TDesC8& aBuffer)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ProcessInterruptData"));
    TInt dataLength = aBuffer.Length();

    if (dataLength > 0)
        {
        TBTTransaction transaction = static_cast<TBTTransaction> (aBuffer[0]);

        switch (transaction)
            {
            case EDATAInput:
                // If this packet was smaller than the maximum transmission
                // unit this is the only data we will get
                if (dataLength < iInterruptInMTU)
                    {
                    // Pass to the observer
                    iObserver.HandleInterruptData(iConnID, aBuffer.Mid(1));
                    }
                else
                    {
                    // Data was the size of the MTU, so we will expect further
                    // DATC packets.
                    // Try to handle the start of this sequence
                    TRAPD(res, StartSegmentedInterruptDataL(aBuffer.Mid(1));)
                    if (res != KErrNone)
                        {
                        // First segment wasn't handled, so just pass up
                        // what we have
                        iObserver.HandleInterruptData(iConnID, aBuffer.Mid(1));
                        }
                    }
                break;

            case EDATCInput:
                // Handle the next segmented packet
                // If this packet was smaller than the maximum transmission
                // unit this is the last packet in the sequence
                if (dataLength < iInterruptInMTU)
                    {
                    ContinueSegmentedInterruptData(aBuffer.Mid(1), ETrue);
                    }
                else
                    {
                    ContinueSegmentedInterruptData(aBuffer.Mid(1), EFalse);
                    }
                break;

            default:
                // Don't expect anything more here
                break;
            }
        }

    return ETrue;
    }

void CBTHidConnection::StartSegmentedInterruptDataL(const TDesC8& aBuffer)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::StartSegmentedInterruptDataL"));
    // First check to see if we already have a data buffer
    if (!iInterruptDataBuffer)
        {
        // Allocate a buffer based on the size of the data and copy it.
        iInterruptDataBuffer = aBuffer.AllocL();
        }
    else
        {
        // Get a modifiable pointer to the buffer
        TPtr8 dataPtr = iInterruptDataBuffer->Des();

        if (dataPtr.MaxLength() < aBuffer.Length())
            {
            // If the buffer isn't large enough recreate it
            delete iInterruptDataBuffer;
            iInterruptDataBuffer = 0;
            iInterruptDataBuffer = aBuffer.AllocL();
            }
        else
            {
            // We have a large enough buffer so copy the data
            dataPtr.Copy(aBuffer);
            }
        }
    }

void CBTHidConnection::ContinueSegmentedInterruptData(const TDesC8& aBuffer,
        TBool aFinalSegment)
    {
        TRACE_FUNC
    (_L("[BTHID]\tCBTHidConnection::ContinueSegmentedInterruptDataL"));
    // If there is no buffer, then we haven't received a start packet, so
    // just ignore this data
    if (iInterruptDataBuffer)
        {
        // Get a modifiable pointer to the buffer
        TPtr8 dataPtr = iInterruptDataBuffer->Des();

        // If there is no data already, then we can't handle this continuation
        // packet
        if (dataPtr.Length() > 0)
            {
            // Append the new data to the end of the buffer
            TBool dataWasAppended = ETrue;
            TInt newSize = (dataPtr.Length() + aBuffer.Length());

            if (dataPtr.MaxLength() < newSize)
                {
                // Reallocate the buffer if it isn't large enough
                HBufC8* newBuffer = iInterruptDataBuffer->ReAlloc(newSize);

                if (newBuffer)
                    {
                    iInterruptDataBuffer = newBuffer;
                    dataPtr.Set(iInterruptDataBuffer->Des());
                    dataPtr.Append(aBuffer);
                    }
                else
                    {
                    // We couldn't append the data, so reset this
                    dataWasAppended = EFalse;
                    }
                }
            else
                {
                dataPtr.Append(aBuffer);
                }

            // If this is the final segment, or we couldn't append this
            // segment, pass up what we have.
            if ((aFinalSegment) || (!dataWasAppended))
                {
                iObserver.HandleInterruptData(iConnID, *iInterruptDataBuffer);

                // Zero the buffer so any more continuation segments will
                // be ignored
                dataPtr.Zero();
                }
            }
        }
    }

//End of file
