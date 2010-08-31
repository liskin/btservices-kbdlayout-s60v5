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
* Description:  Watcher for BTEng server PubSub and CenRep keys.
*
*/



#ifndef BTENGSRVKEYWATCHER_H
#define BTENGSRVKEYWATCHER_H


#include <e32property.h>

#include "btengactive.h"

class CBTEngServer;
class CRepository;

/**
 *  ?one_line_short_description
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
class CBTEngSrvKeyWatcher : public CBase, public MBTEngActiveObserver
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngSrvKeyWatcher* NewL( CBTEngServer* aServer );

    /**
     * Destructor
     */
    virtual ~CBTEngSrvKeyWatcher();

// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Notification that a key value has changed.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, 
                                     TInt aStatus );

    /**
     * Callback for handling cancelation of an outstanding request.
     *
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );
    
    /**
     * From MBTEngActiveObserver.
     * Notification that a key value has changed.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngSrvKeyWatcher( CBTEngServer* aServer );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * Check if we checked the BD_ADDR before.
     *
     * @since S60 v3.2
     */
    void CheckBDAddrL();

private: // data

    /**
     * Property contaning the BT Device Under Test (DUT) status.
     * When enabled, we need inform the server.
     */
    RProperty iDutModeKey;

    /**
     * Property containing the phone lock status.
     * We turn off BT when the phone is remotely locked.
     */
    RProperty iPhoneLockKey;

    /**
     * Property containing the system status.
     * We disconnect BT links when the phone is shutting down.
     */
    RProperty iSystemStateKey;
    
     /**
     * Property containing the BT connecting status.
     * When connecting, specific BT UI indicator is shown.
     */
    RProperty iBtConnectionKey;
    
     /**
     * Property containing the BT stack scanning status.
     * When changed, the UI indicators are updated
     */
    RProperty iBtScanningKey;

    /**
     * Property containing the emergency call status.
     * We disconnect BT SAP when an emergency call is created.
     */
    RProperty iEmergencyCallKey;

    /**
     * Property containing the BT device address.
     * We read it once to store it in CenRep.
     */
    RProperty iBdaddrKey;

    /**
     * Property containing the Simple Pairing debug mode status.
     * We deactivate debug mode after 30 mins or BT power off.
     */
    RProperty iSspDebugModeKey;
    
    /**
     * Session with the central repository for BT SAP mode setting.
     * We load/unload BT SAP plugin.
     * Own.
     */
    CRepository* iSapKeyCenRep;

    /**
     * Active object for subscribing to DUT mode property changes.
     * Own.
     */
    CBTEngActive* iDutModeWatcher;

    /**
     * Active object for subscribing to phone lock property changes.
     * Own.
     */
    CBTEngActive* iPhoneLockWatcher;

    /**
     * Active object for subscribing to system status property changes.
     * Own.
     */
    CBTEngActive* iSystemStateWatcher;
    
    /**
     * Active object for subscribing to BT connection property changes.
     * Own.
     */    
    CBTEngActive* iBtConnectionWatcher;
    
     /**
     * Active object for subscribing to BT scanning state property changes.
     * Own.
     */    
    CBTEngActive* iBtScanningWatcher;

     /**
     * Active object for subscribing to emergency call property changes.
     * Own.
     */
    CBTEngActive* iEmergencyCallWatcher;

    /**
     * Active object for subscribing to BT SAP mode setting changes.
     * Own.
     */
    CBTEngActive* iSapModeWatcher;

    /**
     * Active object for subscribing to BD_Addr property changes.
     * Own.
     */
    CBTEngActive* iBdaddrWatcher;

     /**
     * Active object for subscribing to SSP debug mode property changes.
     * Own.
     */
    CBTEngActive* iSspDebugModeWatcher;

    /**
     * Pointer to our parent.
     * Not own.
     */
    CBTEngServer* iServer;

    };


#endif // BTENGSRVKEYWATCHER_H
