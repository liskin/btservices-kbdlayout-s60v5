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
* Description:  Bluetooth Engine ECom plug-in base class declaration.
*
*/



#ifndef BTENGINEPLUGIN_H
#define BTENGINEPLUGIN_H


#include <ecom/ecom.h>
#include <bt_sock.h>
#include <btengconstants.h>


/**
 * Class MBTEngConnObserver
 *
 * Callback class for receiving CBTEngPlugin events
 * for connect/disconnect complete events.
 *
 *  @lib btengdevman.dll
 *  @since S60 V3.0
 */
class MBTEngPluginObserver
    {

public:

    /**
     * Indicates to the caller that a service-level connection has completed.
     * This function is called for both incoming and outgoing connections. 
     * This function is also called when an outgoing connection request fails, 
     * e.g. with error code KErrCouldNotConnect.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * The caller of this function must be ready to receive new commands on the 
     * CBTEngPlugin interface during execution of this function i.e. it should 
     * have released all resources relating to execution of such commands. 
     *
     * @since  S60 v3.2
     * @param  aAddr The address of the remote device.
     * @param  aProfile Identifier for the service-level connection.
     * @param  aErr Status information, if there is an error.
     * @param  aConflicts If there already is a connection for the selected 
     *                    profile(s) of an outgoing connection request, then 
     *                    this array contains the bluetooth device addresses 
     *                    of the remote devices for those connections.
     */
    virtual void ConnectComplete( const TBTDevAddr& aAddr, 
                                   TBTProfile aProfile, TInt aErr, 
                                   RBTDevAddrArray* aConflicts = NULL ) = 0;

    /**
     * Indicates to the caller that a service-level connection has disconnected.
     *
     * The caller of this function must be ready to receive new commands on the 
     * CBTEngPlugin interface during execution of this function i.e. it should 
     * have released all resources relating to execution of such commands. 
     *
     * @since  S60 v3.2
     * @param  aAddr The address of the remote device.
     * @param  aProfile Identifier for the service-level connection.
     * @param  aErr Status information, if there is an error.
     */
    virtual void DisconnectComplete( const TBTDevAddr& aAddr, 
                                      TBTProfile aProfile, TInt aErr ) = 0;

    };


/**
 *  BTEng plug-in base class.
 *
 *  This is the base class from which a BT Engine ECom plug-in inherit. 
 *  BT Engine plug-ins implement Bluetooth profiles. This interface is 
 *  used to issue commands relating to service-level connection management 
 *  (i.e. relating to profile connections). Each plugin implementation 
 *  identifies itself through the SDP Service Class UUIDs defined by 
 *  the Bluetooth SIG for profiles it supports.
 *
 *  @lib bteng.lib
 *  @since S60 v3.2
 */
class CBTEngPlugin : public CBase
    {

public:

    /**  Array of BT profiles. */
    typedef RArray<TBTProfile> RProfileArray;

    /**
     * Constructor; loads the plug-in through the ECom framework.
     *
     * @since S60 v3.2
     * @param aImplementationUid The UID of the interface implementation 
     *                           to be loaded.
     * @return Pointer to the constructed CBTEngPlugin object.
     */
    inline static CBTEngPlugin* NewL( const TUid aImplementationUid );

    /**
     * Destructor; destroys the plug-in through the ECom framework.
     */
    inline virtual ~CBTEngPlugin();

    /**
     * Sets the observer for receiving connection events through the 
     * MBTEngPluginObserver interface. This method MUST be called 
     * before any command is given to the plug-in
     *
     * @since S60 v3.2
     * @param aObserver The observer of connection events.
     */
    virtual void SetObserver( MBTEngPluginObserver* aObserver ) = 0;

    /**
     * Gets the list of profiles implemented by this plug-in.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * @since S60 v3.2
     * @param aProfiles Array of integers identifying the BT profiles 
     *                  implemented by this plug-in.
     */
    virtual void GetSupportedProfiles( RProfileArray& aProfiles ) = 0;

    /**
     * Indicates if this plug-in implements the specified profiles.
     * Profiles are identified by the SDP Service Class UUID (short form), 
     * as assigned by the Bluetooth SIG.
     *
     * @since S60 v3.2
     * @param aProfile Integer identifying the BT profile.
     * @return ETrue if the plug-in implements the requested profile, 
     *               otherwise EFalse.
     */
    virtual TBool IsProfileSupported( const TBTProfile aProfile ) const = 0;

    /**
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
    virtual TInt Connect( const TBTDevAddr& aAddr ) = 0;

    /**
     * Instructs the plug-in to cancel the creation a service-level 
     * connection with the specified Bluetooth address.
     *
     * In addition, as this is part of an asynchronous operation, the plug-in 
     * completes the outstanding asyncronous request status with KErrCancel.
     *
     * @since S60 v3.2
     * @param aAddr The BT device address identifying a remote device.
     */
    virtual void CancelConnect( const TBTDevAddr& aAddr ) = 0;

    /**
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
    virtual TInt Disconnect( const TBTDevAddr& aAddr, 
                              TBTDisconnectType aDiscType ) = 0;

    /**
     * Gets the addresses of the connected devices for the specified profile.
     *
     * @since S60 v3.2
     * @param aAddrArray On return, holds the Bluetooth device addresses 
     *                   of the connected Bluetooth devices for the 
     *                   requested profile.
     * @param aConnectedProfile The profile for which the existing 
     *                          connections are requested.
     * @return KErrNone if sucessful, otherwise the error code 
     *                  indicating the error situation.
     */
    virtual void GetConnections( RBTDevAddrArray& aAddrArray, 
                                  TBTProfile aConnectedProfile ) = 0;

    /**
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
    virtual TBTEngConnectionStatus IsConnected( const TBTDevAddr& aAddr ) = 0;
    

private: // data

    /**
     * UID set by ECOM when the instance is created.
     * Used when the instance is destroyed.
     */
    TUid iInstanceUid;

    };


// -----------------------------------------------------------------------------
// Create the requested implementation.
// -----------------------------------------------------------------------------
//
inline CBTEngPlugin* CBTEngPlugin::NewL( const TUid aImplementationUid )
    {
    TInt32 keyOffset = _FOFF( CBTEngPlugin, iInstanceUid );
    TAny* plugin = REComSession::CreateImplementationL( aImplementationUid, 
                                                         keyOffset );
    return reinterpret_cast<CBTEngPlugin*>( plugin );
    }


// -----------------------------------------------------------------------------
// The implementation has been deleted.
// -----------------------------------------------------------------------------
//
inline CBTEngPlugin::~CBTEngPlugin()
    {
    REComSession::DestroyedImplementation( iInstanceUid );
    }


#endif // BTENGINEPLUGIN_H
