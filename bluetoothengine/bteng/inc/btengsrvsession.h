/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BTEng server session header definition.
*
*/



#ifndef BTENGSRVSESSION_H
#define BTENGSRVSESSION_H

#include <badesca.h>
#include "btengclientserver.h"

class CSession2;
class RMessage;
class CBTEngServer;
typedef RArray<TBTDevAddr> RBTDevAddrArray;


/**
 *  Class CBTEngSrvSession, server-side session class.
 *
 *  An instance of this class is created for every client session, 
 *  and handles client messages (commands). It also handles informing 
 *  clients of connection events.
 *
 *  @lib bteng.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSrvSession ): public CSession2
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 v3.2
     * @return Pointer to the constructed CBTEngSrvSession object.
     */
    static CBTEngSrvSession* NewL();

    /**
     * Destructor
     */
    virtual ~CBTEngSrvSession();

    /**
     * Notify the client of the connection event.
     *
     * @since S60 v3.2
     * @param aAddr The address of the remote Bluetooth device
     * @param aConnStatus Status of the connection.
     * @param aArray Contains the bluetooth device addresses of the remote 
     *               devices for which profile connections exist for an 
     *               outgoing connection request.
     * @param aErr The error code indicating the result of the connection 
     *             request.
     */
    void NotifyConnectionEvent( const TBTDevAddr aAddr, 
                                 TBTEngConnectionStatus aConnStatus, 
                                 RBTDevAddrArray* aArray, TInt aErr );

private: // from base class CSession2

    /**
     * From CSession2.
     * Processes a message from a client.
     *
     * @since S60 v3.2
     * @param aMessage The message containing the details of the client request.
     */
    virtual void ServiceL( const RMessage2& aMessage );

    /**
     * From CSession2.
     * Completes construction of the session.
     *
     * @since S60 v3.2
     */
    virtual void CreateL();

private:

    /**
     * C++ default constructor
     *
     * @since S60 v3.2
     */
    CBTEngSrvSession();

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v3.2
     */
    void ConstructL();

    /**
     * Returns a handle to the server object.
     *
     * @since S60 v3.2
     * @return Pointer to the server object.
     */
    CBTEngServer* Server();

    /**
     * Processes a message from a client.
     *
     * @since S60 v3.2
     * @param aMessage The message containing the details of the client request.
     */
    void DispatchMessageL( const RMessage2& aMessage );

    /**
     * Cancels outstanding request.
     */
    void CancelRequest();

    /**
     * Checks if power is on, and leaves with error KErrNotReady if not.
     */
    void CheckPowerOnL();

private: // data

    /**
     * Client-server message containing the thread handle of the client.
     * For connection notifications.
     */
    RMessage2 iNotifyConnMessage;
    
    /**
     * Flag indicating if this client requested BT to be switched on temporarily.
     */
    TBool iAutoSwitchOff;
    
    /**
     * Required for queuing connection events to send to the client
     */
    CDesC8ArraySeg* iConnectionEventQueue;
    };

#endif // BTENGSRVSESSION
