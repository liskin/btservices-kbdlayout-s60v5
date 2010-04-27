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
* Description:  BTEng server header definition
*
*/


#ifndef BTENGSERVER_H
#define BTENGSERVER_H

#include <bluetooth/btpowercontrol.h>
#include <bt_sock.h>
#include <bttypes.h>
#include <btserversdkcrkeys.h>

#include "btengconstants.h"

class CBTEngSrvState;
class CBTEngSrvPluginMgr;
class CBTEngPlugin;
class CBTEngSrvBBConnMgr;
class CBTEngSrvKeyWatcher;
class CBTEngSdpDbHandler;
class CBTEngPairMan;
class CPolicyServer;
class TEComResolverParams;
class CImplementationInformation;
class CDeltaTimer;
class CBTEngSrvSettingsMgr;

/**
 * Main function in which the server is running.
 *
 * @since S60 v3.2
 */
static void RunServerL();

/**
 * Utility to panic the client.
 *
 * @since S60 v3.2
 * @param aMessage Client message
 * @param aPanic Panic reason code.
 */
void PanicClient( const RMessage2& aMessage, TInt aPanic );

/**
 * Utility to panic the server.
 *
 * @since S60 v3.2
 * @param aPanic Panic reason code.
 */
void PanicServer( TInt aPanic );

/**  Panic category */
_LIT( KBTEngPanic, "BTEng panic" );

/**  Panic codes */
enum TBTEngServerPanic
    {
    EBTEngPanicMemoryLeak = 10000,
    EBTEngPanicArgumentIsNull,
    EBTEngPanicMemberVarIsNull,
    EBTEngPanicCorrupt,
    EBTEngPanicCorruptSettings,
    EBTEngPanicExpectSetPowerOpcode,
    };


/**
 *  Class CBTEngServer
 *
 *  Core server class of BTEng.
 *
 *  @lib bteng.lib
 *  @since S60 S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngServer ) : public CPolicyServer
    {

    friend class CBTEngSrvPluginMgr;
    friend class CBTEngPairMan;

public:

    /**  Enumeration of bitmask for keeping track of different timers. */
    enum TTimerQueued
        {
        ENone               = 0x00,
        EScanModeTimer      = 0x01,
        EIdleTimer          = 0x02,
        EAutoPowerOffTimer  = 0x04,
        ESspDebugModeTimer  = 0x08 
        };

    /**
     * Two-phased constructor.
     */
    static CBTEngServer* NewLC();

    /**
     * Destructor.
     */
    virtual ~CBTEngServer();

    /**
     * Getter for power change state machine instance.
     *
     * @since Symbian^3
     * @return Pointer to instance of state machine.
     */
    inline CBTEngSrvState* StateMachine() const
            { return iServerState; }

    /**
     * Getter for settings manager instance.
     *
     * @since Symbian^3
     * @return Pointer to instance of settings manager.
     */
    inline CBTEngSrvSettingsMgr* SettingsManager() const
            { return iSettingsMgr; }

    /**
     * Getter for plug-in manager instance.
     *
     * @since Symbian^3
     * @return Pointer to instance of plug-in manager.
     */
    inline CBTEngSrvPluginMgr* PluginManager() const
            { return iPluginMgr; }

    /**
     * Getter for baseband connection manager instance.
     *
     * @since Symbian^3
     * @return Pointer to instance of baseband connection manager.
     */
    inline CBTEngSrvBBConnMgr* BasebandConnectionManager() const
            { return iBBConnMgr; }

    /**
     * Getter for pairing manager. Ownership is not transferred.
     *
     * @since Symbian^3
     * @return Pointer to instance of baseband connection manager.
     */
    inline CBTEngPairMan* PairManager() const
            { return iPairMan; }

    /**
     * Getter for socket server session.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^3
     * @return Reference to session with the socket server.
     */
    inline RSocketServ& SocketServer()
            { return  iSocketServ; }

    /**
     * Getter for registry server session.
     * This handle can be used for creating subsessions.
     *
     * @since Symbian^3
     * @return Reference to session with the registry server.
     */
    inline RBTRegServ& RegistrServer()
            { return iBTRegServ; }

    /**
     * Set Bluetooth on or off
     *
     * @since S60 v3.2
     * @param aState The new power state.
     * @param aTemporary Turn BT off after use (ETrue) or not (EFalse).
     */
    void SetPowerStateL( TBTPowerState aState, TBool aTemporary );

    /**
     * Set Bluetooth on or off upon a client power management request.
     * This function is only for power management request from clients of bteng server.
     *
     * @since Symbian^3
     * @param aMessage The message containing the detail of power management request.
     */
    void SetPowerStateL( const RMessage2 aMessage );
    
    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void DisconnectAllForPowerOffL();
    
    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void DisconnectAllCompleted();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void DispatchPluginMessageL( const RMessage2& aMessage );

    /**
     * Increment the session count.
     *
     * @since S60 v3.2
     */
    void AddSession();

    /**
     * Decrement the session count.
     *
     * @since Symbian^3
     * @param aSession the session to be cloased.
     * @param aAutoOff Indicator if this session had requested BT temporary on.
     */
    void RemoveSession(CSession2* aSession, TBool aAutoOff );

    /**
     * Queue a new timer.
     *
     * @since Symbian^3
     * @param aTimer Identifier for the timer to be queued.
     * @param aInterval The interval for timer callback.
     */
    void QueueTimer( CBTEngServer::TTimerQueued aTimer, TInt64 aInterval );

    /**
     * Remove a queued timer.
     *
     * @since Symbian^3
     * @param aTimer Identifier for the timer to be removed.
     */
    void RemoveTimer( CBTEngServer::TTimerQueued aTimer );

    /**
     * Check if a specific timer is currently queued.
     *
     * @since Symbian^3
     * @param aTimer Identifier for the timer to check.
     */
    inline TBool IsTimerQueued( CBTEngServer::TTimerQueued aTimer ) const
            { return ( iTimerQueued & aTimer ); }

    /**
     * Returns the service (limited to services managed in bteng scope)
     * level connection status of the specified device.
     *
     * @param aAddr the address of the device
     * @return one of TBTEngConnectionStatus enums
     */
    TBTEngConnectionStatus IsDeviceConnected( const TBTDevAddr& aAddr );

    /**
     * Checks if power is off and no session are connected, and
     * starts a shutdown timer if so.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void CheckIdle();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void ManageDIServiceL( TBool aState );

    /**
     * Callback function for completion of disconnecting all 
     * Bluetoooth Baseband links.
     *
     * @since S60 v3.2
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt DisconnectAllCallBack( TAny* aPtr );

    /**
     * Callback function for turning BT off automatically after all
     * connections are gone.
     *
     * @since S60 v5.0
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt AutoPowerOffCallBack( TAny* aPtr );

// from base class CPolicyServer

    /**
     * From CPolicyServer.
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual CSession2* NewSessionL( const TVersion& aVersion, 
                                     const RMessage2& aMessage ) const;

private:

    CBTEngServer();

    void ConstructL();

    /**
     * Reads the product-specific IDs for this phone from central repository.
     * These values are used in the BT Device Information profile.
     *
     * @since S60 v3.2
     * @param aVendorId On return, holds the vendor ID for this phone.
     * @param aProductId On return, holds the product ID for this phone.
     */
    void GetProductIdsL( TInt& aVendorId, TInt& aProductId );

    /**
     * Callback function for expiry of CDeltaTimer timer of temporary 
     * visibility mode.
     *
     * @since S60 v3.2
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt ScanModeTimerCallBack( TAny* aPtr );
    
    /**
     * Callback function for expiriy CDeltaTimer timer of server idle timeout.
     *
     * @since S60 v3.2
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt IdleTimerCallBack( TAny* aPtr );
    
    /**
     * Callback function for expiriy CDeltaTimer timer of 
     * Simple Pairing Debug Mode.
     *
     * @since S60 v3.2
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt DebugModeTimerCallBack( TAny* aPtr );

private: // data

    /**
     * Counter of the number of open sessions, which is used to 
     * stop the server when no session exists and Bluetooth is off.
     */
    TInt iSessionCount;

    /**
     * Handle to BT Power Manager library; this handle must exist 
     * for the lifetime of the loaded library.
     */
    TUint32 iDiSdpRecHandle;

    /**
     * Remember which timer has been queued. (CDeltaTimer does not 
     * provide this information).
     */
    TUint32 iTimerQueued;

    /**
     * Callback for expiry of temporary visibility mode timer.
     */
    TDeltaTimerEntry iScanModeCallBack;

    /**
     * Callback for expiry of temporary power on mode timer.
     */
    TDeltaTimerEntry iPowerOffCallBack;

    /**
     * Callback for expiry of idle timer.
     */
    TDeltaTimerEntry iIdleCallBack;

    /**
     * Callback for expiry of Simple Pairing debug mode.
     */
    TDeltaTimerEntry iDebugModeCallBack;

    /**
     * Pairing manager.
     */
    CBTEngPairMan* iPairMan;
    
    /**
    * Socket Server instance for this and other classes to access Bluetooth socket APIs.
    */   
    RSocketServ iSocketServ;

    /**
    * Registry Server instance for bteng to access Bluetooth registry APIs.
    */      
    RBTRegServ iBTRegServ;

    /**
     * Our state machine for handling power on/off.
     * Own.
     */
    CBTEngSrvState* iServerState;

    /**
     * Hardware and stack settings manager.
     * Own.
     */
    CBTEngSrvSettingsMgr* iSettingsMgr;

    /**
     * ECOM plugin manager.
     * Own.
     */
    CBTEngSrvPluginMgr* iPluginMgr;

    /**
     * Bluetooth Baseband connection manager.
     * Own.
     */
    CBTEngSrvBBConnMgr* iBBConnMgr;

    /**
     * Watcher for the relevant CenRep and PubSub keys.
     * Own.
     */
    CBTEngSrvKeyWatcher* iWatcher;

    /**
     * SDP database handler.
     * Own.
     */
    CBTEngSdpDbHandler* iSdpDbHandler;

    /**
     * Timer for setting various timeouts.
     * Own.
     */
    CDeltaTimer* iTimer;

    };


#endif // BTENGSERVER
