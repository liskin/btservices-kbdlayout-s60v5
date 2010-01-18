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


#ifndef __SOCKETINITIATOR_H__
#define __SOCKETINITIATOR_H__

#include <e32base.h>
#include <es_sock.h>
#include <bt_sock.h>
#include "timeoutnotifier.h"

class CTimeOutTimer;
class MSockInitNotifier;

/*!
 Controls connection of the L2CAP control and interrupt channels to a BT
 HID device. Performs authorisation and encryption if required.
 */
class CSocketInitiator : public CActive, public MTimeOutNotifier
    {
public:

    /*!
     Create a CSocketInitiator object
     @param aSocketServ the socket server session
     @param aNotifier a notifier to report to.
     @result A pointer to the created instance of CSocketInitiator
     */
    static CSocketInitiator* NewL(RSocketServ& aSocketServ,
            MSockInitNotifier& aNotifier);

    /*!
     Create a CSocketInitiator object
     (object is left on the cleanup stack)
     @param aSocketServ the socket server session
     @param aNotifier a notifier to report to.
     @result A pointer to the created instance of CSocketInitiator
     */
    static CSocketInitiator* NewLC(RSocketServ& aSocketServ,
            MSockInitNotifier& aNotifier);

    /*!
     Destroy the object and release all memory objects
     */
    ~CSocketInitiator();

    /*!
     Perform a connection to a BT HID device
     @param aAddress the bluetooth address of the device to connect
     @param aUseSecurity whether to use security on the connection
     @param aControlSocket the socket to use to connect the control channel
     @param aInterruptSocket the socket to use to connect the interrupt channel
     */
    void ConnectSocketsL(const TBTDevAddr& aAddress, TBool aUseSecurity,
            RSocket* aControlSocket, RSocket* aInterruptSocket);

public:
    // from MTimeOutNotifier

    void TimerExpired();

protected:
    // from CActive

    /*!
     Cancel any outstanding requests
     */
    void DoCancel();

    /*!
     Respond to an event
     */
    void RunL();

private:

    /*!
     Constructs this object
     @param aSocketServ the socket server session
     @param aNotifier a notifier to report to.
     */
    CSocketInitiator(RSocketServ& aSocketServ, MSockInitNotifier& aNotifier);

    /*!
     Performs second phase construction of this object
     */
    void ConstructL();

    /*!
     Tracks the progress of host initiated connection
     */
    enum TConnectState
        {
        EIdle, /*!< Initial State */
        EConnectingControl, /*!< Control channel is being connected */
        EConnectingInterrupt, /*!< Interrupt channel is being connected */
        EAuthenticateLink, /*!< Forcing authentication of the device. */
        EEncryptLink
        /*!< Forcing encryption of the connection */
        };

    /*!
     Connect the L2CAP Interrupt Channel
     */
    void ConnectInterruptSocket();

private:
    /*! The maximum time allowed for connection requests to complete */
    static const TInt KTimeOut;

    /*! The socket server session */
    RSocketServ& iSocketServ;

    /*! A notifier to inform of connection results*/
    MSockInitNotifier& iNotifier;

    /*! Current connect process state */
    TConnectState iState;

    /*! Use security on the sockets */
    TBool iUseSecurity;

    /*! BT Socket address used to connect */
    TBTSockAddr iSockAddress;

    /*! Control channel socket */
    RSocket* iControlSocket;

    /*! Interrupt channel socket */
    RSocket* iInterruptSocket;

    /*! Timer active object */
    CTimeOutTimer* iTimer;

    /*! Encryption buffer to use with the encrypt IOCTL */
    THCISetEncryptionBuf iEncryptBuf;
    };

#endif

