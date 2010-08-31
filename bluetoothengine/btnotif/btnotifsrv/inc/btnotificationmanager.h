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
* Description: Class for managing user notification and query objects, 
* and for serializing access to the notification server.
*
*/

#ifndef BTNOTIFICATIONMANAGER_H
#define BTNOTIFICATIONMANAGER_H


#include <e32base.h>

class CBTNotifServer;
class CBluetoothNotification;

/**
 *  CBTNotificationManager manages Bt Notifications
 *  
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotificationManager ) : public CBase
    {

public:

    /**  Enumeration for the priority of the notification. */
    enum TNotificationPriority
        {
        EPriorityLow,
        EPriorityStandard,
        EPriorityHigh
        };

    /**
     * Two-phased constructor.
     * @param aServer Pointer to our parent
     */
    static CBTNotificationManager* NewL( const CBTNotifServer* aServer );

    /**
    * Destructor.
    */
    virtual ~CBTNotificationManager();

    CBluetoothNotification* GetNotification();

    void ReleaseNotification( CBluetoothNotification* aNotification );

    /**
     * Add a notification to the queue of notifications scheduled to be 
     * shown. Notifications are serialized, to avoid overlapping notes.
     * The queue is managed on priority of each notification.
     *
     * @since Symbian^4
     * @param aNotification The notification to be added.
     * @param aPriority The priority of the notification. EPriorityHigh means 
     *                  that the note is put to the front of the queue.
     *                  EPriorityStandard means that it is appended to the end,
     *                  and should be used for most notes. EPriorityLow is not 
     *                  used yet.
     */
    void QueueNotificationL( CBluetoothNotification* aNotification, 
                TNotificationPriority aPriority = EPriorityStandard );

private:

    CBTNotificationManager( const CBTNotifServer* aServer );

    void ConstructL();

    /**
     * Process the notification queue and launch the next notification.
     *
     * @since Symbian^4
     */
    void ProcessNotificationQueueL();

    /**
     * Process the notification queue and launch the next notification.
     *
     * @since Symbian^4
     */
    void CleanupUnusedQueueL();

    /**
     * Create and queue an idle timer if there are no outstanding notifications,
     * otherwise cancel the idle timer.
     *
     * @since Symbian^4
     */
    void CheckIdle();

private: // data

    /**
     * The queue of notifications to be shown to the user.
     */
    RPointerArray<CBluetoothNotification> iNotificationQ;
    
    /**
     * Pointer to our parent.
     * Not own.
     */
    const CBTNotifServer* iServer;

    };

#endif // BTNOTIFICATIONMANAGER_H
