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
* Description:  Implements the actual connection management, and receives 
*                notifications of Bluetooth connection events.
*
*/



#ifndef BTENGCONNHANDLER_H
#define BTENGCONNHANDLER_H


//#include <?include_file>

#include "btengactive.h"
#include "btengclient.h"
#include "btengclientserver.h"

class MBTEngConnObserver;


/**
 *  Class CBTEngConnHandler
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngConnHandler ) : public CBase, public MBTEngActiveObserver
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngConnHandler* NewL( MBTEngConnObserver* aObserver );

    /**
     * Destructor
     */
    virtual ~CBTEngConnHandler();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    inline TInt ConnectDevice( const TBTDevAddr& aAddr, 
                                const TBTDeviceClass& aDeviceClass  );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    inline TInt CancelConnectDevice( const TBTDevAddr& aAddr );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    inline TInt DisconnectDevice( const TBTDevAddr& aAddr, 
                                   TBTDisconnectType aDiscType );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    inline TInt IsDeviceConnected( const TBTDevAddr& aAddr, 
                                    TBTEngConnectionStatus& aConnected );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    inline TInt IsDeviceConnectable( const TBTDeviceClass& aDeviceClass, 
                                      TBool& aConnectable );

    /**
     * ?description
     *
     * @since S60 v5.1
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    inline TInt IsDeviceConnectable( const TBTDevAddr& aAddr, 
            const TBTDeviceClass& aDeviceClass, TBool& aConnectable );
    
    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void GetConnectedAddressesL( RBTDevAddrArray& aAddrArray, 
                                  TBTProfile aConnectedProfile );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    TInt NotifyConnectionEvents( MBTEngConnObserver* aObserver );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @return ?description
     */
    TInt CancelNotifyConnectionEvents();

    /**
     * Set a pairing observer in BTEngine.
     *
     * @since S60 v3.2
     * @param aAddr The address of the remote device that is being paired.
     * @param aActivate If ETrue, the observer will be activated, otherwise
     *                  the observer will be removed.
     * @return KErrNone on success, otherwise a system-wide error code.
     */
    static TInt SetPairingObserver( const TBTDevAddr& aAddr, TBool aActivate );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @return ?description
     */
    inline TInt PrepareDiscovery();


    /**
     * Pair a device
     *
     * @since S60 v5.1
     * @param aAddr the address of the device to pair with
     * @param aDeviceClass the device class of the device to pair with
     * @return KErrNone if this request has been accepted; an error situation otherwise.
     */
    TInt StartPairing( const TBTDevAddr& aAddr, const TBTDeviceClass& aDeviceClass );

    /**
     * cancel the outstanding pairing request.
     */
    void CancelPairing();
    
private:
// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Handles the notification of a change in the connection status.
     *
     * @since S60 v3.2
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aId, TInt aStatus );

    /**
     * From MBTEngActiveObserver.
     * Handles an error during processing of connection status event.
     *
     * @since S60 v3.2
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aId, TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngConnHandler( MBTEngConnObserver* aObserver );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    void HandleConnectionEvent( TInt aStatus );
    
private: // data

    /**
     * Client-server package to which the event result is copied.
     */
    TBTEngEventPkg iEventPkg;

    /**
     * Address of the remote device to pair with.
     */
    TBTDevAddrPckgBuf iPairAddr;
    
    /**
     * The CoD of the remote device to pair with.
     */
    TUint32 iPairDevCod;
    
    /**
     * Session with BTEng server side.
     */
    RBTEng iBTEng;

    /**
     * The actual active object for receiving event notifications.
     * Own.
     */
    CBTEngActive* iConnEventActive;

    /**
     * The actual active object for pairing a device.
     * Own.
     */
    CBTEngActive* iPairActive;
    
    /**
     * Reference to receiver of connection events.
     * Not own.
     */
    MBTEngConnObserver* iObserver;

    };


#include "btengconnhandler.inl"


#endif // BTENGCONNHANDLER_H
