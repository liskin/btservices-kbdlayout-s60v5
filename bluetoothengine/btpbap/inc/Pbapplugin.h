/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the btpbap ecom plugin class declaration.
*
*/

#ifndef BTPBAPPLUGIN_H
#define BTPBAPPLUGIN_H

#include <e32base.h>
#include <btengplugin.h>
#include <pbapcli.h>
#include "DisconnectHelper.h"    

    
/**
 *  BTPBAP plugin class
 *
 *  This is the CBTEngPlugin implementation
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS ( CPBAPplugin ) : public CBTEngPlugin,  
                                    public MDisconnectionCallback 
    {
public:


    /**
     * Two-phased constructor.     
     */
    static CPBAPplugin* NewL();

    /**
     * Destructor.
     */    
    ~CPBAPplugin();
    
    
    /**
     * From CBTEngPlugin.
     * Sets the observer for receiving connection events through the 
     * MBTEngPluginObserver interface. This method MUST be called 
     * before any command is given to the plug-in
     *
     * @since S60 v3.2
     * @param aObserver The observer of connection events.
     */
     void SetObserver( MBTEngPluginObserver* aObserver );

    /**
     * From CBTEngPlugin.
     * Gets the list of profiles implemented by this plug-in.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * @since S60 v3.2
     * @param aProfiles Array of integers identifying the BT profiles 
     *                  implemented by this plug-in.
     */
     void GetSupportedProfiles( RProfileArray& aProfiles );

    /**
     * From CBTEngPlugin.
     * Indicates if this plug-in implements the specified profiles.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * @since S60 v3.2
     * @param aProfile Integer identifying the BT profile.
     * @return ETrue if the plug-in implements the requested profile, 
     *               otherwise EFalse.
     */
     TBool IsProfileSupported( const TBTProfile aProfile ) const ;

    /**
     * From CBTEngPlugin.
     * Instructs the plug-in to create a service-level connection 
     * with the specified Bluetooth address.
     *
     * This is an asynchronous operation; on completion, the plug-in informs 
     * the observer by calling MBTEngPluginObserver::ConnectComplete with 
     * the appropriate error code.
     *
     * @since S60 v3.2
     * @param aAddr The BT device address identifying a remote device.
     * @return KErrNone on success, otherwise an appropriate error code.
     */
     TInt Connect( const TBTDevAddr& aAddr );

    /**
     * From CBTEngPlugin.
     * Instructs the plug-in to cancel the creation a service-level 
     * connection with the specified Bluetooth address.
     *
     * In addition, as this is part of an asynchronous operation, the plug-in 
     * completes the outstanding asyncronous request status with KErrCancel.
     *
     * @since S60 v3.2
     * @param aAddr The BT device address identifying a remote device.
     */
     void CancelConnect( const TBTDevAddr& aAddr );

    /**
     * From CBTEngPlugin.
     * Instructs the plug-in to create a service-level connection 
     * with the specified Bluetooth address.
     *
     * This is an asynchronous operation; on completion, the plug-in informs 
     * the observer by calling MBTEngPluginObserver::DisconnectComplete with 
     * the appropriate error code.
     *
     *
     * @since S60 v3.2
     * @param aAddr The BT device address identifying a remote device.
     *              In case the address is the NULL address, the plug-in is
     *              expected to disconnect all its connections.
     * @param aDiscType The type of disconnection; 
     *                  EGraceful for graceful (normal) disconnection, 
     *                  EImmediate for immediate (forced) disconnection.
     * @return KErrNone on success, otherwise an appropriate error code.
     */
     TInt Disconnect( const TBTDevAddr& aAddr, 
                              TBTDisconnectType aDiscType );
   
   /**
    * From CBTEngPlugin.
    * Gets the addresses of the connected devices for the specified profile.
    *
    * @since S60 v3.2
    * @param aAddrArray On return, holds the Bluetooth device addresses 
    *                   of the connected Bluetooth devices for the 
    *                   requested profile.
    * @param aConnectedProfile The profile for which the existing 
    *                          connections are requested.
    * @return None.
    */
    void GetConnections( RBTDevAddrArray& aAddrArray, 
                                  TBTProfile aConnectedProfile );

    /**
     * From CBTEngPlugin.
     * Indicates if this plug-in has a service-level connection 
     * with the specified Bluetooth address.
     *
     * @since S60 v3.2
     * @param aAddr The BT device address identifying a remote device.
     * @return The status of the connection according to the TConnectionStatus
     *         enumeration. EConnecting is interpreted that at least one 
     *         incoming connection request has been completed, but that not 
     *         all service-level connections have been fully established.
     */
    TBTEngConnectionStatus IsConnected( const TBTDevAddr& aAddr );


private:
    
    
    /**
     * From MDisconnectionCallback.
     * Callback function for completing asynchronous disconnection request
     *
     * @since S60 v3.2    
     */ 
    void CompleteDisconnection();  
    
private:

    CPBAPplugin();
    
    void ConstructL();
    
private: // data 

    /*
     * Callback interface for completing asynchronous
     * disconnection event.
     * Not own.
     */
    MBTEngPluginObserver* iObserver;
    
    /*
     * Pbap server client session     
     */
    RPbapSession          iBPAP;
    
    /*
     * Helper class for asynchronous  disconnection
     * Own.
     */
    CDisconnectHelper*    iDisconnectHelper;
    
    /*
     * BT device address to be disconnected     
     */
    TBTDevAddr            iAddr;
    };
#endif // BTPBAPPLUGIN
