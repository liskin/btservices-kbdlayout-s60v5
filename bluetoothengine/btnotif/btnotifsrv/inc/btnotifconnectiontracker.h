/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Bluetooth connection tracker and manager.
*
*/

#ifndef BTNOTIFCONNECTIONTRACKER_H
#define BTNOTIFCONNECTIONTRACKER_H


#include <e32property.h>
#include <btmanclient.h>
#include <bluetooth/pairing.h>
#include "btnotifserver.h"

class CBTNotificationManager;
class CbtnotifConnectionTrackerTest;
class CBTNotifSecurityManager;



/**
 *  CBTNotifConnectionTracker keeps track of remote device connections
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifConnectionTracker ) : public CBase
    {

public:

    /**
     * Two-phased constructor.
     * @param aServer Pointer to our parent
     */
    static CBTNotifConnectionTracker* NewL( CBTNotifServer* aServer );

    /**
     * Destructor.
     */
    virtual ~CBTNotifConnectionTracker();

    /**
     * Get a pointer to the btnotif server object.
     *
     * @since Symbian^4
     * @return The server.
     */
    inline CBTNotifServer* Server() const
        { return iServer; }

    /**
     * Get a pointer to the notification manager.
     * This handle can be used for queueing notifications.
     *
     * @since Symbian^4
     * @return The notification manager.
     */
    inline CBTNotificationManager* NotificationManager() const
        { return iServer->NotificationManager(); }

    /**
     * Get the shared handle to BT registry server.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^4
     * @return Session with BT registry server.
     */
    inline RBTRegServ& RegistryServerSession()
        { return iBTRegistrySession; }

    /**
     * Get the handle to the socket server.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^4
     * @return Session with the socket server.
     */
    inline RSocketServ& SocketServerSession()
        { return iSockServ; }
  
    /**
     * Processes a message for notifiers related to remote devices.
     * ( These are usually issued by BT stack. However, any application
     * is not restricted to do so (e.g., for testing purpose).
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    void HandleNotifierRequestL( const RMessage2& aMessage );
    
    /**
     * Handle a request related to pairing.
     *
     * @since Symbian^4
     * @param aMessage The message containing the details of the client request.
     */
    void HandleBondingRequestL( const RMessage2& aMessage );

    /**
     * Check repeated connection attempts, and record rejected/accepted queries.
     *
     * @since Symbian^4
     * @param aDevice The details of the remote device for this query.
     * @param aAccepted ETrue if the user accepted the request, EFalse if rejected.
     * @return ETrue if the user should be queried for blocking this device,
     *         EFalse if no query should be launched by the caller.
     */
    TBool UpdateBlockingHistoryL( const CBTDevice* aDevice, TBool aAccepted );

private:

    CBTNotifConnectionTracker( CBTNotifServer* aServer );

    void ConstructL();

    /**
     * Record and check the time between connection attempts.
     *
     * @since Symbian^4
     * @param aAccepted ETrue if the user accepted the request, EFalse if rejected.
     * @return EFalse if the attempt followed the previous attempt too fast,
     *         otherwise ETrue.
     */
    TBool RecordConnectionAttempts( TBool aAccepted );

private: // data

    /**
     * Time of the last denied connection attempt.
     */
    TInt64 iLastReject;

    /**
     * Array of device addresses that the user has denied access.
     */
    RArray<TBTDevAddr> iDeniedRequests;

    /**
     * Single session with BTRegistry, to be used for subsessions.
     */
    RBTRegServ iBTRegistrySession;

    /**
     * Single session with the socket server, to be used for subsessions.
     */
    RSocketServ iSockServ;

    /**
     * Reference to our parent the server class.
     * Not own.
     */
    CBTNotifServer* iServer;
    
    /**
     * Object for managing the application pairing.
     * Own.
     */
    CBTNotifSecurityManager* iPairingManager;

    BTUNITTESTHOOK

    };

#endif // BTNOTIFCONNECTIONTRACKER_H
