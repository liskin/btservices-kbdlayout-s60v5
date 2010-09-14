/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT RFComm socket.
*
*/


#ifndef C_BTMRFCOMMSOCK_H
#define C_BTMRFCOMMSOCK_H

#include <bt_sock.h>
#include "btmdefs.h"

class MBtmSockObserver;
class CDesC8ArrayFlat;

/**
 * RFComm socket.
 *
 * A RFComm connection.
 *
 *  @since S60 v3.1
 */
class CBtmRfcommSock : public CBase, public MBluetoothSocketNotifier
    {
public:

    static CBtmRfcommSock* NewL(MBtmSockObserver& aObserver, RSocketServ& aServer);

    virtual ~CBtmRfcommSock();
    
public:
    
    /**
     * Makes a RFCOMM connection to a BT device
     *
     * @since S60 v3.1
     * @param aAddr the BD address of the BT device
     */ 
    void ConnectL(TBTSockAddr& aAddr, TUint aService);
    
    /**
     * Cancel the outstanding connect request
     *
     * @since S60 v3.1
     */
    void CancelConnect();

    /**
     * Socket accept operation.
     * Register a RFComm channel first then register SDP service according the
     * given service id then starts socket accept operation.
     *
     * @since S60 v3.1
     * @param aService The service (profile)
     * @param aSec the security requirement of the service
     */
    TUint ListenL(TUint aService, const TBTServiceSecurity& aSec, TUint aLastUsedPort);

    /**
     * cancels the outstanding accept operation.
     * Th reserved channel will be freed and the registered service will be removed.
     *
     * @since S60 v3.1
     */
    void CancelListen();

    /**
     * Disconnects the socket connection
     *
     * @since S60 v3.1
     * @param aHow Information about the reason of shutdown.
     */
    void Disconnect(RSocket::TShutdown aHow = RSocket::ENormal);
    
    /**
     * Writes a data packet to the connected BT device
     *
     * @since S60 v3.1
     * @param aDesc the data packet
     */
    void WriteL(const TDesC8& aData);
    
    /**
     * Cancel writing
     *
     * @since S60 v3.1
     */
    void CancelWrite();
    
    /**
     * Get the connected remote BD address 
     *
     * @since S60 v3.1
     * @return the BD address
     */
    const TBTDevAddr& Remote() const; 
    
    /**
     * Activates the notification of baseband events
     *
     * @since S60 v3.1
     * @param aNotification the interested baseband events
     * @return the completion error code
     */
    TInt ActivateBasebandEventNotification(TInt aNotification);
    
    /**
     * Sets the socket observer to the specified.
     *
     * @since S60 v3.1
     * @param aObserver the new observer.
     */
    void SetObserver(MBtmSockObserver& aObserver);

    /**
     * Gets the service (a RFComm profile) that this connection is for.
     *
     * @since S60 v3.1
     * @return the service id.
     */
    TUint Service() const;
    
    TInt ActivateSniffRequester();
    
    void RequestLinkToActiveMode();
    
    TBool IsInSniff() const;
    
    void SetService(TUint aService);
    
    TBool AccessoryInitiatedConnection();

private:  

    // from base class MBluetoothSocketNotifier

    /**
     * From MBluetoothSocketNotifier
     * Notification of a RFCOMM socket connect completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     * @param aType the type (HF or HS) of the accessory if the connection is successful
     */
    void HandleConnectCompleteL(TInt aErr);

    /**
     * From MBluetoothSocketNotifier
     * Notification of a RFCOMM socket accpet completion event.
     *
     * @since S60 v3.1
     * @param aType the type (HF or HS) of the accessory if the connection is successful
     * @param aErr the completion error code
     */
    void HandleAcceptCompleteL(TInt aErr);

    /**
     * From MBluetoothSocketNotifier
     * Notification of a RFCOMM socket shutdown completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleShutdownCompleteL(TInt aErr);

    /**
     * From MBluetoothSocketNotifier
     * Notification of a RFCOMM socket send completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleSendCompleteL(TInt aErr);

    /**
     * From MBluetoothSocketNotifier
     * Notification of a RFCOMM socket receive completion event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     */
    void HandleReceiveCompleteL(TInt aErr);

    /** 
     * From MBluetoothSocketNotifier
     * Notification of an ioctl complete event.
     * 
     * @since S60 v3.1
     * @param aErr the returned error
     */
    void HandleIoctlCompleteL(TInt aErr);

    /**
     * From MBluetoothSocketNotifier
     * Notification of a baseband event.
     *
     * @since S60 v3.1
     * @param aErr the completion error code
     * @param TBTBasebandEventNotification Bit(s) set indicate what event has taken place.
     */        
    void HandleActivateBasebandEventNotifierCompleteL(
        TInt aErr, TBTBasebandEventNotification& aEventNotification);

    // From base class CBtmActive

private:

    void ConstructL();

    CBtmRfcommSock(MBtmSockObserver& aObserver, RSocketServ& aServer);

    void SendPacketL();

    /**
     * Reads data from the connected BT device
     *
     * @since S60 v3.1
     * @param aData Contains the received data on completion
     */
    void ReceiveL();
    
    /**
     * Cancel reading
     *
     * @since S60 v3.1
     */
    void CancelReceive();

private:
    
    /**
     * socket event observer.
     * Not own.
     */
    MBtmSockObserver* iObserver;
    
    /**
     * The socket server session
     * Not own.
     */
    RSocketServ& iServer;
    
    /**
     * The RFComm connection socket
     */
    CBluetoothSocket* iDataSocket;

    /**
     * The listening socket 
     */
    CBluetoothSocket* iListenSocket;

    /**
     * the length of data read and written
     */
    TSockXfrLength iXfrLength;
    
    /**
     * the baseband event notification
     */
    TBTBasebandEvent iEventNotification;

    TInt iBBNotificationMode;
    
    TBool iInSniff;
    
    /**
     * The service for which the socket connection is served
     */
    TUint iService;
    
    /** 
     * The remote BD address
     */
    TBTDevAddr iRemote;
    
    TBool iRemoteHasConnected; // who initiated connection
    RBuf8 iInData; // owned
    RBuf8 iOutData; // owned
    CDesC8ArrayFlat* iOutDataQueue; // owned
    
    TBool iWriting;
    
    RBTPhysicalLinkAdapter iAda;
    };

#endif    // C_BTMRFCOMMSOCK_H
