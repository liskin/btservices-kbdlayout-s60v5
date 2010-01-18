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


#ifndef __BTHIDSERVER_H__
#define __BTHIDSERVER_H__

#include <e32base.h>
#include <es_sock.h>
#include <btmanclient.h>
#include <f32file.h>
#include <ecom/ecom.h>
#include <btdevice.h>
#include <e32std.h>
#include <bttypes.h> //linkkey
#include <btnotif.h>


#include "hidinterfaces.h"
#include "bthidclient.h"
#include "btconnectionobserver.h"
#include "listenerobserver.h"
#include "genericserver.h"
#include "hidlayoutids.h"
#include "genericactive.h"

class CBTConnection;
class CSocketListener;
class CBTHidConnection;
class CBTHidDevice;
class CGenericHid;
class CBTHidNotifierHelper;

/**
 *  An instance of class CBTHidServer is the main server class.
 *
 *  @since Series 60 2.6
 */
class CBTHidServer : public CGenericServer,
        public MBTConnectionObserver,
        public MListenerObserver,
        public MTransportLayer
    {
public:

    /*!
     Create a CBTHidServer object using two phase construction,
     and return a pointer to the created object
     @result Pointer to created CBTHidServer object
     */
    static CBTHidServer* NewL();

    /*!
     Create a CBTHidServer object using two phase construction,
     and return a pointer to the created object, leaving a pointer to the
     object on the cleanup stack.
     @result Pointer to created CBTHidServer object.
     */
    static CBTHidServer* NewLC();

    /*!
     Destroy the object and release all memory objects.
     */
    ~CBTHidServer();
    
    /*!
     Panic the client.
     @param aMessage the message channel to the client
     @param aReason the reason code for the panic.
     */
    static void PanicClient(const RMessage2& aMessage, TInt aReason);

    void StartShutdownTimerIfNoSession();

    void CancelShutdownTimer();

    /*!
     Increments the count of the active sessions for this server.
     */
    void IncrementSessions();

    /*!
     Decrements the count of the active sessions for this server. .
     If no more sessions are in use the server terminates.
     */
    void DecrementSessions();

    /*!
     Returns a count of the BT HID connections the server is managing.
     @result Number of connections
     */
    TUint ConnectionCount();

    /*!
     Gives access to the device details for a numbered connection.
     @param aConnID connection ID.
     @result Reference to a device details object
     */
    CBTHidDevice& ConnectionDetailsL(TInt aConnID);

    void CleanOldConnection(TInt aConnID);
    /*!
     Asks the server to create a new connection object.
     @result ID for the connection
     */
    TInt NewConnectionL();

    /*!
     Asks the server to do a first-time connection to a device.
     @param aConnID ID of the connection returned from NewConnectionL
     */
    void DoFirstConnectionL(TInt aConnID);

    /*!
     Asks the server to delete a new connection object
     @param aConnID ID of the connection returned from NewConnectionL
     */
    void DeleteNewConnection(TInt aConnID);

    /*
     * Keep the connection alive, initial reconnect from HID device available... 
     */
    void DisconnectDeviceL(const TBTDevAddr& aAddress);
    void DisconnectAllDeviceL();

    /**
     * Asks the server to disconnect (virtual cable unplug) a device totally, 
     * remove the connection entry from the connection container. 
     @param aAddress bluetooth address of the device to disconnect.
     */
    void CloseBluetoothConnection(const TBTDevAddr& aAddress);
    void CloseAllBluetoothConnection();

    /*!
     Finds out the connection status for a given Bluetooth address connected with HID profile
     @param aBDAddr reference to TBTAddr that is checked from Bluetooth registry
     @result TBTEngConnectionStatus
     */
    TBTEngConnectionStatus ConnectStatus(const TBTDevAddr& aAddress);

    /*!
     Panic the server.
     @param aPanic panic code
     */
    static void PanicServer(TInt aPanic);
    
    TBool GetConnectionAddress(TDes8& aAddressBuf);

    TBool IsAllowToConnectFromServerSide(TUint aDeviceSubClass);
    TBool IsAllowToConnectFromClientSide(TBTDevAddr devAddr);
    TUint GetDeviceSubClass(TBTDevAddr aDevAddr);
    TBool IsKeyboard(TUint aDeviceSubClass);
    TBool IsPointer(TUint aDeviceSubClass);

    TBTDevAddr ConflictAddress();

public:
    // from MBTConnectionObserver
    void HandleControlData(TInt aConnID, const TDesC8& aBuffer);

    void HandleCommandAck(TInt aConnID, TInt aStatus);

    void HandleInterruptData(TInt aConnID, const TDesC8& aBuffer);

    void FirstTimeConnectionComplete(TInt aConnID, TInt aStatus);

    void LinkLost(TInt aConnID);

    void LinkRestored(TInt aConnID);

    void Disconnected(TInt aConnID);

    void Unplugged(TInt aConnID);

public:
    //from MListenerObserver
    void SocketAccepted(TUint aPort, TInt aErrorCode);

public:
    //from MTransportLayer
    TUint CountryCodeL(TInt aConnID);

    TUint VendorIdL(TInt aConnID);

    TUint ProductIdL(TInt aConnID);

    void GetProtocolL(TInt aConnID, TUint16 aInterface);

    void SetProtocolL(TInt aConnID, TUint16 aValue, TUint16 aInterface);

    void GetReportL(TInt aConnID, TUint8 aReportType, TUint8 aReportID,
            TUint16 aInterface, TUint16 aLength);

    void SetReportL(TInt aConnID, TUint8 aReportType, TUint8 aReportID,
            TUint16 aInterface, const TDesC8& aReport);

    void DataOutL(TInt aConnID, TUint8 aReportID, TUint16 aInterface,
            const TDesC8& aReport);

    void GetIdleL(TInt aConnID, TUint8 aReportID, TUint16 aInterface);

    void SetIdleL(TInt aConnID, TUint8 aDuration, TUint8 aReportID,
            TUint16 aInterface);

    void NotifierRequestCompleted();
    
protected:
    // From CActive
    /*!
     Process any errors
     @param aError the leave code reported.
     */
    TInt RunError(TInt aError);
    static TInt TimerFired(TAny* aThis);

private:

    /*!
     Perform the first phase of two phase construction 
     */
    CBTHidServer();

    /*!
     Perform the second phase construction of a CBTHidServer object
     */
    void ConstructL();

     /*!
     Shuts down socket listeners on error and inform user
     @param aError The error which caused the shutdown.
     */
    void ShutdownListeners(TInt aError);

    /*!
     Retrieve the Connection object for a given ID.
     @param aConnID The connection ID.
     @return A pointer to a CBTHidConnection object.
     */
    CBTHidConnection* IdentifyConnectionL(TInt aConnID) const;

    /*!
     Loads virtually-cabled device details from the filestore.
     @param aStoreName file to load from.
     */
    void LoadVirtuallyCabledDevicesL(const TDesC& aStoreName);

    /*!
     Stores virtually-cabled device details from the filestore.
     @param aStoreName file to write to.
     */
    void StoreVirtuallyCabledDevicesL(const TDesC& aStoreName);

    /*!
     Stores virtually-cabled device details from the filestore.
     (Non-Leaving Version)
     @param aStoreName file to write to.
     */
    void StoreVirtuallyCabledDevices(const TDesC& aStoreName);

    /*!
     Updates all current sessions with the current status of a connection
     @param aDeviceDetails details of the device whose status has changed 
     @param aState new state of the connection
     */
    void InformClientsOfStatusChange(const CBTHidDevice& aDeviceDetails,
            TBTHidConnState aState);

    void GlobalNotify(const TBTDevAddr& aDeviceAddr, TBTHidConnState aState);

    TInt HandleAsyncRequest(const TBTDevAddr& aDeviceAddr, TInt note);

    void HandleAsyncRequestL(const TBTDevAddr& aDeviceAddr, TInt aNote);
 
    void InformStatusChange(const TBTDevAddr& aAddress,
            TBTHidConnState aState);
    /*!
     Inform the Generic HID of a new connection (and start the driver if required)
     @param aConnection a CBTHidConnection object for the device connected.
     @param aStartDriver ETrue to start the driver, EFalse otherwise.
     */
    void
            GenericHIDConnectL(CBTHidConnection* aConnection,
                    TBool aStartDriver);

private:
    // From CServer2

    /*!
     Create a time server session, and return a pointer to the created object
     @param aVersion version 
     @result Pointer to new session
     */
    CSession2* NewSessionL(const TVersion &aVersion,
            const RMessage2& aMessage) const;

private:
    /* Internal function to override the security settings of Control channel
     * listener. Used if we are listening for unauthenticated/unauthorised device.
     * For example: Microsoft keyboard with Nokia 6630 PR#1 needs this.
     */
    void CheckAndSetControlListenerSecurityL(TBool aSec);

    /*! The active session count */
    TInt iSessionCount;

    /*! The file server session */
    RFs iFs;

    /*! The socket server session */
    RSocketServ iSocketServ;

    /*! A temporary socket to accept a connection on the control channel */
    RSocket* iTempControl;

    /*! A temporary socket to accept a connection on the interrupt channel */
    RSocket* iTempInterrupt;

    /*! Flag indicating the Autorisation setting on incoming connections */
    TBool iAuthorisationFlag;

    /*! Socket listener for the control channel */
    CSocketListener* iControlListener;

    /*! Socket listener for the interrupt channel */
    CSocketListener* iInterruptListener;

    /*! The Generic HID object */
    CGenericHid* iGenHID;

    /*! Main container index for the server */
    CObjectConIx* iMasterContIndex;

    /*! Container for BT Connection Objects */
    CObjectCon* iBTConnContainer;

    /*! Index for the BT Connection container */
    CObjectIx* iBTConnIndex;

    /*! BT address of a Bluetooth device currently connected in conflict*/
    TBTDevAddr iConflictAddr;

    /*! BT address of a Bluetooth device last used */
    TBTDevAddr iLastUsedAddr;
    TBool iActiveState;

    //Shutdown Timer
    CPeriodic* iShutDownTimer;

    RNotifier iNotifier;

    RPointerArray<CBTHidNotifierHelper> iReqs;

    };


/** 
 *  This wraps up a UI Notification for the BTHidServer
 */
   
class CBTHidNotifierHelper : public CActive
    {
public:

    static CBTHidNotifierHelper* NewL(CBTHidServer& aHidServer, TInt aNote, const TBTDevAddr& aDeviceAddr);
    
    ~CBTHidNotifierHelper();
    
    void Start();
    
private:

    CBTHidNotifierHelper(CBTHidServer& aHidServer, TInt aNote, const TBTDevAddr& aDeviceAddr);
    
    void ConstructL();
    void RunL();
    void DoCancel();

private:
    
    CBTHidServer& iHidServer;
    
    RNotifier iNotifier;
    
    TPckgBuf<TInt> iNoResult;
    
    TBTGenericInfoNotiferParamsPckg iGenericInfoNotifierType;
    };  

#endif
