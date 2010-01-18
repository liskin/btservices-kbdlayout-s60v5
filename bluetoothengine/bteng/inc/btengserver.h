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
#ifndef __WINS__
#include <bluetooth/dutmode.h>
#endif  //__WINS__  
#include <bt_sock.h>
#include <bttypes.h>
#include <btfeaturescfg.h>
#include "btengdomaincrkeys.h"
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
    EBTEngPanicCorruptSettings
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

    friend class CBTEngSrvSession;
    friend class CBTEngSrvState;
    friend class CBTEngSrvPluginMgr;
    friend class CBTEngSrvKeyWatcher;
    friend class CBTEngPairMan;

public:

    static CBTEngServer* NewLC();

    virtual ~CBTEngServer();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aState The new power state.
     * @param aTemp Turn BT off after use (ETrue) or not (EFalse).
     */
    void SetPowerStateL( TBTPowerStateValue aState, TBool aTemporary );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void SetVisibilityModeL( TBTVisibilityMode aMode, TInt aTime );
    
     /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void UpdateVisibilityModeL( TInt aStackScanMode );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void DisconnectAllL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void SetDutMode( TInt aDutMode );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void ScanModeTimerCompletedL();

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
     * @since S60 v3.2
     * @param aAutoOff Indicator if this session had requested BT temporary on.
     */
     void RemoveSession( TBool aAutoOff );
     
     /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
     void SetUiIndicatorsL();
     
     /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
     void SetIndicatorStateL( const TInt aIndicator, const TInt aState );

    /**
     * Check whether BT should be turned off automatically.
     *
     * @since S60 v5.0
     */
     void CheckAutoPowerOffL();

    /**
     * Utility to get HW power state.
     *
     * @since S60 v5.1
     * @param aState On return, this will contain the current HW power state.
     * @return KErrNone if successful, otherwise one of the system-wide error codes.
     */
    TInt GetHwPowerState( TBTPowerStateValue& aState );

     /**
     * queue a timer if Simple Pairing debug mode has been enabled.
     *
     * @since S60 v3.2
     * @param aDebugMode State of Simple Pairing debug mode.
     */
     void CheckSspDebugModeL( TBool aDebugMode );
     
     /**
      * Gets the access to pairing manager. Ownership is not transferred.
      * @return the pairing manager
      */
     CBTEngPairMan& PairManager();
     
     /**
      * Be informed when registry remote device table is changed
      */
     void RemoteRegistryChangeDetected();

     /**
      * gets the reference of socket server session
      */
     RSocketServ& SocketServ();
     
     /**
      * gets the reference of registry session.
      */
     RBTRegServ& BTRegServ();
     
     /**
      * Returns the service (limited to services managed in bteng scope)
      * level connection status of the specified device.
      *
      * @param aAddr the address of the device
      * @return one of TBTEngConnectionStatus enums
      */
     TBTEngConnectionStatus IsDeviceConnected( const TBTDevAddr& aAddr );
     
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

// from base class MBTPowerManagerObserver

    BluetoothFeatures::TEnterpriseEnablementMode EnterpriseEnablementMode() const;
    
private:

    CBTEngServer();

    void ConstructL();

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
    TInt SetPowerState( TBool aState );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    TInt SetLocalNameL();
    
    TInt GetLocalNameFromRegistryL(TDes& aName);

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void SetClassOfDeviceL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void InitBTStackL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void StopBTStackL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void UpdateCenRepPowerKeyL( TBTPowerStateValue aValue );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void LoadBTPowerManagerL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void ManageDIServiceL( TBool aState );
    
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
     * Checks if a client requests temporary power on/off, and keeps track 
     * of the number of clients requesting that.
     *
     * @since S60 v5.0
     * @param aCurrentState On return, will contain the current power state.
     * @param aNewState The requested power state.
     * @param aTemporary Indicates if this is about a tempororary state change.
     */
    void CheckTemporaryPowerStateL( TBTPowerStateValue& aCurrentState, 
                                     TBTPowerStateValue aNewState, 
                                     TBool aTemporary );

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
     * Callback function for completion of disconnecting all 
     * Bluetoooth Baseband links.
     *
     * @since S60 v3.2
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt DisconnectAllCallBack( TAny* aPtr );
    
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
    
    /**
     * Callback function for turning BT off automatically after all
     * connections are gone.
     *
     * @since S60 v5.0
     * @param aPtr Pointer to ourselves.
     * @return Result
     */
    static TInt AutoPowerOffCallBack( TAny* aPtr );
	
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
     * Flag indicating if BT is going to be switched off automatically.
     */
    TBool iAutoSwitchOff;

    /**
     * Number of clients that are using BT temporarily.
     * Note that this is equal or less than the active number of links.
     */
    TInt iAutoOffClients;

    /**
     * Flag indicating hidden mode has been set for temporary power on.
     */
    TBool iRestoreVisibility;

    /**
    * To decide wether DUT mode should be enabled in power mode
    * change callback.
    */   
    TBool iEnableDutMode;

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
     * Handle to BT Power Manager library; this handle must exist 
     * for the lifetime of the loaded library.
     */
    RLibrary iPowerMgrLib;
#ifndef __WINS__
    /**
     *  Handle for DUT mode API
     */
    RBluetoothDutMode iDutMode;
#endif  //__WINS__  

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
     * BT Power Manager.
     * Own.
     */
    CBTEngSrvState* iServerState;

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
     * BT Power Manager (HCIv2 version).
     */
    RBTPowerControl iPowerMgr;

#ifdef __WINS__
    /**
     * Current BT power state (power manager is not used in emulator).
     */
    TBTPowerState iPowerState;
#endif  //__WINS__

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

    BluetoothFeatures::TEnterpriseEnablementMode iEnterpriseEnablementMode;
    };


#endif // BTENGSERVER
