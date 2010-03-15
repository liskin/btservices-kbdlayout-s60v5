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
* Description:  Declares main application class.
 *
*/


#ifndef __BTCONNECTION_H__
#define __BTCONNECTION_H__

#include <es_sock.h>
#include <bt_sock.h>
#include "socketobserver.h"
#include "sockinitnotifier.h"
#include "timeoutnotifier.h"

const TUint KCommandBufferSize = 2;

enum
    {
    EControlSocketID, EInterruptSocketID
    };

const TUint KL2CAPDefaultMTU = 672;

/*! 5 Minute -10 sec Inactivity Timeout (-10 sec due to Nokia Su8W*/
const TUint KInactivityTimeout = (5 * 60 * 1000000) - 10000000;

class CSocketReader;
class CSocketWriter;
class CSocketInitiator;
class CDataSegmenter;
class CTimeOutTimer;
class CBTHidDevice;
class MBTConnectionObserver;

/*!
 Tracks the current state of the connection
 */
enum TBTConnectionState
    {
    ENotConnected, /*!< No connection has been established. */
    EFirstConnection, /*!< A first-time connection is in progress. */
    ELinkLost, /*!< The connection is in link loss. */
    EConnected, /*!< The connection is active */
    EHostReconnecting, /*!< Host is reconnecting to the device */
    EHIDReconnecting, /*!< The device is reconnection to the host */
    EDisconnecting, /*!< The connection is being closed */
    EConnecting,      /*!< The connection is being set up from phone side */
    EHIDInitConnecting /*!< The connection is being set up from remote HID */
    };

/*!
 This class represents a single connection to a bluetooth HID device.
 It creates instances of separate active objects to perform reading from,
 and writing to, the sockets.
 */
class CBTHidConnection : public CObject,
        public MSocketObserver,
        public MSockInitNotifier,
        public MTimeOutNotifier
    {
public:
    /*!
     Create a CBTHidConnection object.
     @param aSocketServ The socket server session.
     @param aObserver An observer of this object.
     @result A pointer to the created instance of CBTHidConnection.
     */
    static CBTHidConnection* NewL(RSocketServ& aSocketServ,
            MBTConnectionObserver& aObserver,
            TBTConnectionState aConnectionState);

    /*!
     Create a CBTHidConnection object.
     @param aSocketServ The socket server session.
     @param aObserver An observer of this object.
     @result A pointer to the created instance of CBTHidConnection.
     */
    static CBTHidConnection* NewLC(RSocketServ& aSocketServ,
            MBTConnectionObserver& aObserver,
            TBTConnectionState aConnectionState);

    /*!
     Destroy the object and release all memory objects.
     */
    ~CBTHidConnection();

    /*!
     Gets the id for this connection.
     @return A connection ID.
     */
    TInt ConnID();

    /*!
     Sets the id for this connection.
     @param aConnID The new id for this connection.
     */
    void SetConnID(TInt aConnID);

    /*!
     Starts a first time connection to the HID device.
     */
    void ConnectL();

    /*!
     The connection should restart. either by connecting to the device or waiting
     for a connection.
     */
    void ReconnectL();

    /*!
     Perform a deliberate disconnection from the device. 
     Includes sending the Virtual-Cable-Unplug message if needed.
     */
    void Disconnect();

    /*!
     Drops the Bluetooth connection to a device
     */
    void DropConnection();

    /*!
     Starts the reading process on the sockets.
     */
    void StartMonitoringChannelsL();

    /*!
     Get access to the connections device details object.
     @result A reference to a device details object.
     */
    CBTHidDevice& DeviceDetails();

    /*!
     Current status of a connection after initial connection has been performed.
     @result ETrue device is connected.
     @result EFalse device is in link loss.
     */
    TBool IsConnected() const;

    TBTConnectionState ConnectStatus() const;

    /*!
     Asks to accept an incoming connection on the control channel.
     @param aAddress address of the connecting device.
     @param aSocket the socket for the connection. This will be set to NULL
     if the connection accepts the socket.
     */
    void OfferControlSocket(const TBTDevAddr& aAddress, RSocket*& aSocket);

    /*!
     Asks to accept an incoming connection on the interrupt channel.
     @param aAddress address of the connecting device.
     @param aSocket the socket for the connection. This will be set to NULL
     if the connection accepts the socket.
     */
    void OfferInterruptSocket(const TBTDevAddr& aAddress, RSocket*& aSocket);

    /*!
     Issues the Get Protocol Command over the control channel..
     */
    void GetProtocolL();

    /*!
     Issues the Set Protocol Command over the control channel.
     @param aValue the protocol setting. 0 = Boot, 1 = Report.
     */
    void SetProtocolL(TUint16 aValue);

    /*!
     Issues the Get Report Command over the control channel.
     @param aReportType the type of the report.
     @param aReportID the report ID.
     @param aLength the length of the report we are asking for.
     */
    void GetReportL(TUint8 aReportType, TUint8 aReportID, TUint16 aLength);

    /*!
     Issues the Set Report Command over the control channel.
     @param aReportType the type of the report.
     @param aReportID the report ID.
     @param aReport the report.
     */
    void SetReportL(TUint8 aReportType, TUint8 aReportID,
            const TDesC8& aReport);

    /*!
     Issues the DATA Out command over the Interrupt channel. This is similar to
     SetReport which uses ReportType Output. Therefore you may use SetReport
     to do data transfers from Host to device as well. 
     @param aReportID
     @param aReport reference to the report buffer
     */
    void DataOutL(TUint8 aReportID, const TDesC8& aReport);

    /*!
     Issues the Get Idle Command over the control channel.
     */
    void GetIdleL();

    /*!
     Issues the Set Idle Command over the control channel.
     @param aDuration the idle duration.
     */
    void SetIdleL(TUint8 aDuration);

public:
    // from MSocketObserver

    void HandleSocketError(TUint aSocketID, TBool aConnectionLost,
            TInt aErrorCode);

    TBool HandleDataReceived(TUint aSocketID, const TDesC8& aBuffer);

    void HandleWriteComplete(TUint aSocketID);

public:
    // from MSockInitNotifier

    void SocketsConnected();

    void SocketsConnFailed(TInt aStatus);

public:
    // from MTimeOutNotifier
    void TimerExpired();

private:
    /*!
     Perform the first phase of two phase construction. 
     @param aSocketServ the socket server session.
     @param aObserver an observer of this object.
     */
    CBTHidConnection(RSocketServ& aSocketServ,
            MBTConnectionObserver& aObserver,
            TBTConnectionState aConnectionState);

    /*!
     Perform the second phase construction of a CBTHidConnection object.
     */
    void ConstructL();

    /*!
     The BT HID Transaction Header based upon Bluetooth HID Spec Sec. 7.3
     */
    enum TBTTransaction
        {
        // Handshake commands
        EHandshakeSuccess = 0,
        EHandshakeNotReady = 1,
        EHandshakeInvalidRepID = 2,
        EHandshakeUnsupported = 3,
        EHandshakeInvalidParam = 4,
        EHandshakeUnknown = 14,
        EHandshakeFatal = 15,
        // Control Commands
        EHIDControlNOP = 16,
        EHIDControlHardReset = 17,
        EHIDControlSoftReset = 18,
        EHIDControlSuspend = 19,
        EHIDControlExitSuspend = 20,
        EHIDControlVCUnplug = 21,
        // Get Report Commands
        EGetReportFullBufReserved = 64,
        // Set Report Commands
        ESetReportReserved = 80,
        // Get/Set Protocol Commands
        EGetProtocol = 96,
        ESetProtocol = 112,
        // Get/Set Idle Commands
        EGetIdle = 128,
        ESetIdle = 144,
        // DATA Commands
        EDATAOther = 160,
        EDATAInput = 161,
        EDATAOutput = 162,
        EDATAFeature = 163,
        // DATC Commands
        EDATCOther = 176,
        EDATCInput = 177,
        EDATCOutput = 178,
        EDATCFeature = 179
        };

    /**
     *  Prepare the newly-connected sockets for reading and writing.
     */
    void PrepareSocketsL();

    /*!
     Perform a change in the connection status
     @param aNewStatus new connection status
     */
    void ChangeState(TBTConnectionState aNewStatus);

    /*!
     Handle the situation when the link to a device is lost.
     */
    void ConnectionLost();

    /*!
     The HID device has sent the virtual cable unplug command.
     */
    void ReceivedVirtualCableUnplug();

    /*!
     Checks to see if the connections is in a state able to service a set/get
     command.
     */
    void LeaveIfCommandNotReadyL() const;

    /*!
     Closes the L2CAP Control and Interrupt channels
     */
    void CloseChannels();

    /*!
     Handle data received on the control channel.
     @param aBuffer the data buffer.
     @result ETrue to continue reading.
     @result EFalse to stop reading.
     */
    TBool ProcessControlData(const TDesC8& aBuffer);

    /*!
     Append as much data to a buffer as can be handled.
     @param aDest the destination buffer.
     @param aSource the source buffer.
     */
    void AppendData(TDes8& aDest, const TDesC8& aSource);

    /*!
     Handle data received on the interrupt channel.
     @param aBuffer the data buffer.
     @result ETrue to continue reading.
     @result EFalse to stop reading.
     */
    TBool ProcessInterruptData(const TDesC8& aBuffer);

    void HandleEDATCFeature(const TDesC8& aBuffer);

    /*!
     Begin handling a new multiple interrupt packet transaction
     @param aBuffer the data buffer
     */
    void StartSegmentedInterruptDataL(const TDesC8& aBuffer);

    /*!
     Handle a Continuation data packet on the interrupt channel
     @param aBuffer the data buffer
     @param aFinalSegment ETrue if this is the final segment, EFalse otherwise
     */
    void ContinueSegmentedInterruptData(const TDesC8& aBuffer,
            TBool aFinalSegment);

private:

    /*! The socket server */
    RSocketServ& iSocketServ;

    /*! An observer for status reporting */
    MBTConnectionObserver& iObserver;

    /*! The ID given to this connection */
    TInt iConnID;

    /*! The HID device details */
    CBTHidDevice* iDevice;

    /*! Current connection status */
    TBTConnectionState iConnectionState;

    /*! Control channel socket */
    RSocket* iControlSocket;

    /*! Interrupt channel socket */
    RSocket* iInterruptSocket;

    /*! Active object to control connecting sockets */
    CSocketInitiator* iSocketInitiator;

    /*! Active object to control reads from the socket */
    CSocketReader* iControlSocketReader;

    /*! Active object to control writes to the socket */
    CSocketWriter* iControlSocketWriter;

    /*! Active object to control reads from the socket */
    CSocketReader* iInterruptSocketReader;

    /*! Active object to control writes to the socket */
    CSocketWriter* iInterruptSocketWriter;

    /*! Set/Get command is outstanding with the device */
    TBool iCommandIssued;

    /*! Fixed buffer used for all Set/Get commands except SetReport*/
    TBuf8<KCommandBufferSize> iCommandBuffer;

    /*! Utility object to provide individual packets for large data */
    CDataSegmenter* iCommandSegmenter;

    /*! Dynamic buffer used for control data larger than the incoming MTU */
    HBufC8* iControlDataBuffer;

    /*! Dynamic buffer used for interrupt data larger than the incoming MTU */
    HBufC8* iInterruptDataBuffer;

    /*! Size of data host can send in a packet on the control channel*/
    TInt iControlOutMTU;

    /*! Size of data the device sends in a packet on the control channel*/
    TInt iControlInMTU;

    /*! Size of data host can send in a packet on the interrupt channel*/
    TInt iInterruptOutMTU;

    /*! Size of data the device sends in a packet on the interrupt channel*/
    TInt iInterruptInMTU;

    /*! A timer to track inactivity on the Bluetooth Connection */
    CTimeOutTimer* iInactivityTimer;
    };

#endif // __BTCONNECTION_H__
