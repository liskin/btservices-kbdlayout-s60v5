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
* Description: 
 *       This class handles CenRep notifications
 *
*/


#ifndef BTHIDCENREPWATCHER_H
#define BTHIDCENREPWATCHER_H

// INCLUDES
#include <e32std.h>
#include <cenrepnotifyhandler.h>

// CLASS DECLARATION

class MBtHidCenRepObserver
    {
public:
    virtual void CenRepDataChanged(TUid& aUid, TUint32 aKey) = 0;
    };

class CRepository;
class CTimeOutTimer;

/**
 * The class to handle CenRep notifications
 */
NONSHARABLE_CLASS(CBtHidCenRepWatcher) : public CBase,
        public MCenRepNotifyHandlerCallback
    {
public:
    // Constructors and destructor

    /**
     * Two-phased constructor.
     * @param aUid The Uid of the repository we want to use.
     * @param aObserver The parent who is interested in relevant CenRep changes
     */
    static CBtHidCenRepWatcher* NewL(TUid aUid,
            MBtHidCenRepObserver& aObserver);

    /**
     * Destructor.
     */
    virtual ~CBtHidCenRepWatcher();

protected:
    // From MCenRepNotifyHandlerCallback

    /**
     * This callback method is used to notify the client about
     * changes in keys when the whole repository is listened for.
     *
     * Note: It is not guaranteed that a notification will be received
     *       for all keys, if multiple keys are changed in rapid succession
     *       by multiple threads or when the whole repository is reset,
     *       therefore only listen for whole repository if this is not an issue.
     *
     * @param aId Id of the key that has changed. If multiple keys were changed by
     *            whole repository reset, value will be KInvalidNotificationId.
     */
    void HandleNotifyGeneric(TUint32 aId);

    /**
     * This callback method is used to notify the client about errors
     * in the handler. Any error in handling causes the handler to stop
     * handling any more notifications. Handling can be restarted with
     * a call to aHandler->StartListeningL(), if the error is non-fatal.
     * However, be careful to trap any errors from this call if this is done.
     *
     * @param aId Id of the key this instance listens for or if notifications for
     *            whole repository are listened, could also be KInvalidNotificationId.
     * @param aError Error code.
     * @param aHandler Pointer to the handler instance. 
     *                 This pointer can be used to identify the handler or restart the listening.
     */
    void HandleNotifyError(TUint32 aId, TInt aError,
            CCenRepNotifyHandler* aHandler);

public:
    // New functions

    /**
     * Disable notifications
     */
    void DisableNotifications();

    /**
     * Enable notifications
     */
    void EnableNotifications();

private:

    /**
     * Constructor
     * @param aUid The Uid of the repository.
     * @param aObserver The instance which receives interested CenRep changes
     */
    CBtHidCenRepWatcher(TUid aUid, MBtHidCenRepObserver& aObserver);

    /**
     * 2nd phase constructor
     * @param aUid The Uid of the repository.
     */
    void ConstructL();

private:
    // DATA
    CCenRepNotifyHandler* iNotifier;
    CRepository* iSession;
    TUid iUid;
    MBtHidCenRepObserver& iObserver;
    TBool iNotificationsEnabled;
    };

#endif      // BTHIDCENREPWATCHER_H
// End of File
