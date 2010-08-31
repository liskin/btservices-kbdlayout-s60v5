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
* Description: Server class for handling commands from clients, and the 
*                central class in btnotif thread.
*
*/

#ifndef BTNOTIFSERVER_H
#define BTNOTIFSERVER_H


#include <e32base.h>
#include <btengsettings.h>

#include "bluetoothtrace.h"

class CBTNotifConnectionTracker;
class CBTNotifSettingsTracker;
class CBTNotificationManager;
class CBTNotifDeviceSelector;
class CBtDevRepository;

class CbtnotifServerTest; // ToDo: shall be refactored under compile flag?

/**
 * Utility function for panicking the server.
 *
 * @since Symbian^4
 * @param aReason The panic reason code.
 */
void PanicServer( TInt aReason );

/**
 * Utility function for panicking the server.
 *
 * @since Symbian^4
 * @param aMessage The handle to the client side.
 * @param aReason The panic reason code.
 */
void PanicClient( const RMessage2& aMessage, TInt aReason );

/**  BTNotif panic codes */
enum TBTNotifServerPanic
    {
    EBTNotifPanicUnknown,
    EBTNotifPanicCorrupt,
    EBTNotifPanicMissing,
    EBTNotifPanicNullMember,
    EBTNotifPanicBadState,
    EBTNotifPanicBadArgument,
    EBTNotifPanicBadResult
    };

/**  Convenience macro for indicating unimplemented events */
#define NOTIF_NOTIMPL 
//{ __ASSERT_ALWAYS( 1, PanicServer( EBTNotifPanicNotImplemented ) ); }
/**  Convenience macro for indicating unhandled events */
#define NOTIF_NOTHANDLED( cond ) 
//{ __ASSERT_ALWAYS( cond, PanicServer( EBTNotifPanicNotHandled ) ); }


/**
 * CBTNotifServer handles notifications and also maintains state information related to 
 * the local device as well as remote devices.
 *
 *  @since Symbian^4
 */
NONSHARABLE_CLASS( CBTNotifServer ) : public CPolicyServer
    {

public:

    /**
     * Two-phased constructor.
     */
    static CBTNotifServer* NewLC();

    /**
    * Destructor.
    */
    virtual ~CBTNotifServer();

    /**
     * Called when the BT power state changes.
     *
     * @since Symbian^4
     * @param aState The new BT power state.
     */
    void HandlePowerStateChangeL( TBTPowerStateValue aState );

    /**
     * Called by a session during creation, to keep track of the number
     * of active sessions.
     *
     * @since Symbian^4
     */
    void AddSession();

    /**
     * Called by a session during destruction, to keep track of the number
     * of active sessions.
     *
     * @since Symbian^4
     */
    void RemoveSession();

    /**
     * Returns a handle to the settings tracker.
     *
     * @since Symbian^4
     * @param Pointer to the settings tracker.
     */
    inline CBTNotifSettingsTracker* SettingsTracker() const
        { return iSettingsTracker; }

    /**
     * Returns a handle to the connection tracker.
     *
     * @since Symbian^4
     * @param Pointer to the connection tracker.
     */
    inline CBTNotifConnectionTracker* ConnectionTracker() const
        { return iConnectionTracker; }

    /**
     * Returns a handle to the notification manager.
     *
     * @since Symbian^4
     * @param Pointer to the notification manager.
     */
    inline CBTNotificationManager* NotificationManager() const
        { return iNotificationMgr; }

    CBtDevRepository& DevRepository();
    
    CBTNotifDeviceSelector& DeviceSelectorL();
    
// from base class CPolicyServer

    /**
     * From CPolicyServer.
     * Creates and returns a server-side session object.
     *
     * @since Symbian^4
     * @param aVersion The version information supplied by the client.
     * @param aMessage Represents the details of the client request that 
     *                 is requesting the creation of the session.
     * @return A pointer to the newly created server-side session object.
     */
    virtual CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const;

private:

    CBTNotifServer();

    void ConstructL();

    /**
     * Asynchronous second-phase constructor. This function is called asynchronously
     * from ConstructL, to facilitate short initial construction, and allow construction
     * of e.g. session to BTEngine. So in fact this is a third-phase constructor.
     *
     * @since Symbian^4
     */
    void AsyncConstructL();

    void CheckIdle( TBTPowerStateValue aState );
    
    /**
     * Callback function for asynchronous construction.
     *
     * @since Symbian^4
     * @param aPtr Pointer to server instance.
     */
    static TInt AsyncConstructCb( TAny* aPtr );

    static TInt ShutdownTimeout( TAny* aPtr );
    
private: // data

    /**
     * Counter for the number of active sessions.
     */
    TInt iSessionCount;

    /**
     * Helper class for tracking settings.
     * Own.
     */
    CBTNotifSettingsTracker* iSettingsTracker;

    /**
     * Helper class for tracking connections.
     * Own.
     */
    CBTNotifConnectionTracker* iConnectionTracker;

    /**
     * Helper class for managing the actual notifications.
     * Own.
     */
    CBTNotificationManager* iNotificationMgr;

    /**
     * Remote device repository.
     * Singleton in btnotifsrv. This is useful
     * for getting the correct name for displaying in UI
     * without the need getting the device everytime
     * when a name is needed.
     * Pairing handling also needs this repository to drive
     * the logic flow.
     */
    CBtDevRepository* iDevRep;
    
    /**
     * Helper class for device searching and selection.
     * Own.
     */
    CBTNotifDeviceSelector* iDevSelector;
    
    /**
     * Callback for asynchronous processing.
     * Own.
     */
    CAsyncCallBack* iAsyncCb;

    /**
     * Timer for various timeouts.
     * Own.
     */
    CDeltaTimer* iTimer;
    
    /**
     * The function entry of shutdown timeout.
     */
    TDeltaTimerEntry iShutdownTimerEntry;
    
    BTUNITTESTHOOK

    };

#endif // BTNOTIFSERVER_H
