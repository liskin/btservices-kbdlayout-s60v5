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


#ifndef __BTHIDSESSION_H__
#define __BTHIDSESSION_H__

#include <e32base.h>
#include "hidsdpobserver.h"
#include "bthidclient.h"

class CBTHidServer;
class CHidSdpClient;

/*!
 An instance of class CBTHidServerSession is created for each client
 */
class CBTHidServerSession : public CSession2, public MHidSdpObserver
    {

public:

    /*!
     Create a CBTHidServerSession object using two phase construction,
     and return a pointer to the created object.
     @param aClient client's thread.
     @param aServer the server.
     @result Pointer to new session.
     */
    static CBTHidServerSession* NewL(CBTHidServer& aServer);

    /*!
     Create a CBTHidServerSession object using two phase construction,
     and return a pointer to the created object
     (leaving a pointer to the object on the cleanup stack)
     @param aClient client's thread
     @param aServer the server
     @result Pointer to new session
     */
    static CBTHidServerSession* NewLC(CBTHidServer& aServer);

    /*!
     Destroy the object and release all memory objects
     */
    ~CBTHidServerSession();

    /*!
     Updates the client of this session with the new status of a 
     connection, if it has an outstanding update request.
     @param aUpdateParams details of the bluetooth device that has changed. 
     */
    void InformStatusChange(const THIDStateUpdateBuf& aUpdateParams);
    /*!
     Informs the session of the result of a first-time connection.
     @param aConnID id given to the connection when it was created
     @param aResult result of the connection attempt
     */
    void InformConnectionResult(TInt aConnID, TInt aResult);

public:
    // from CSession
    /*!
     Service request from client
     @param aMessage client message (containing requested operation and any data)
     */
    void ServiceL(const RMessage2& aMessage);

public:
    // from MHidSdpObserver

    void HidSdpSearchComplete(TInt aResult);

private:

    /*!
     Perform the first phase of two phase construction 
     @param aClient client's thread
     */
    CBTHidServerSession(CBTHidServer& aServer);

    /*!
     Perform the second phase construction of a CBTHidServerSession object
     */
    void ConstructL();

    void DispatchMessageL(const RMessage2& aMessage);

    /*!
     Handle the connection message.
     @param aMessage message for request
     */
    void RequestConnectionL(const RMessage2& aMessage);

    /*!
     Handle the disconnection message.
     @param aMessage message for request
     */
    void RequestDisconnectionL(const RMessage2& aMessage);

    void IsConnectedL(const RMessage2& aMessage);

    void GetConnections(const RMessage2& aMessage);

private:
    // Member variables

    /*! The hid server */
    CBTHidServer& iServer;

    /*! ID of the connection being established */
    TInt iConnectingID;

    /*! A hid sdp client */
    CHidSdpClient* iHidSdpClient;

    TBool iSearchingSDP;

    /*! An outstanding connect message */
    RMessage2 iConnectionMessage;

    RMessagePtr2 iDisconnectMsg;

    /*! Session has an outstanding update message */
    RMessage2 iUpdateMessage;

    };

#endif
