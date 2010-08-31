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
    void CancelNotifyConnectionEvents();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @return ?description
     */
    inline TInt PrepareDiscovery();
    
private:
// from base class MBTEngActiveObserver

    /**
     * From MBTEngActiveObserver.
     * Handles the notification of a change in the connection status.
     *
     * @since S60 v3.2
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aStatus );

    /**
     * Callback for handling cancelation of an outstanding request.
     *
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void CancelRequest( TInt aRequestId );
    
    /**
     * From MBTEngActiveObserver.
     * Handles an error during processing of connection status event.
     *
     * @since S60 v3.2
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aError );

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
     * Session with BTEng server side.
     */
    RBTEng iBTEng;

    /**
     * The actual active object for receiving event notifications.
     * Own.
     */
    CBTEngActive* iConnEventActive;
    
    /**
     * Reference to receiver of connection events.
     * Not own.
     */
    MBTEngConnObserver* iObserver;

    };


#include "btengconnhandler.inl"


#endif // BTENGCONNHANDLER_H
