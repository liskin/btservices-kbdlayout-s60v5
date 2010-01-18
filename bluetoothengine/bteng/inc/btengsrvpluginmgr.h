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
* Description:  Helper class for BTEng server to manage and interface with
*                the profile plug-ins the profile plug-ins.
*
*/



#ifndef BTENGSRVPLUGINMGR_H
#define BTENGSRVPLUGINMGR_H

#include "btengclientserver.h"
#include "btengplugin.h"

class CBTEngServer;
class TEComResolverParams;
typedef RPointerArray<CImplementationInformation> RImplInfoPtrArray;
typedef RPointerArray<CBTEngPlugin> RPluginPtrArray;

/**  ?description */
//const ?type ?constant_var = ?constant;


/**
 *  ?one_line_short_description
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSrvPluginMgr ) : public CBase, 
                                          public MBTEngPluginObserver
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngSrvPluginMgr* NewL( CBTEngServer* aServer );

    /**
     * Destructor
     */
    virtual ~CBTEngSrvPluginMgr();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void ProcessCommandL( const RMessage2& aMessage );

    /**
     * ?description
     *
     * @since S60 v3.2
     */
    void DisconnectAllPlugins();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aParams Selection parameters for loading a specific plugin.
     *                If it is empty (zero-length descriptor), 
     *                all plug-ins are loaded.
     */
    void LoadProfilePluginsL( const TEComResolverParams aParams );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @return The number of ECom plug-ins still to load.
     */
    TInt LoadPluginL();

    /**
     * ?description
     *
     * @since S60 v3.2
     */
    void UnloadProfilePlugins();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void LoadBTSapPluginL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void UnloadBTSapPlugin();
    
    /**
    * Disconnects the profile specified by aProfile
    * Currently used only when emergency call is created for disconnecting SAP
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void DisconnectProfile( TBTProfile aProfile );
	
    /**
     * Checks if any audio connection is established
     *
     * @since S60 v5.0
     * @return ETrue if any audio connection is established
     */
    TBool CheckAudioConnectionsL();
	
    /**
     * Returns the service (limited to services managed in bteng scope)
     * level connection status of the specified device.
     *
     * @param aAddr the address of the device
     * @return one of TBTEngConnectionStatus enums
     */
    TBTEngConnectionStatus IsDeviceConnected( const TBTDevAddr& aAddr );    
    
// from base class MBTEngPluginObserver

    /**
     * From MBTEngPluginObserver.
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void ConnectComplete( const TBTDevAddr& aAddr, 
                                   TBTProfile aProfile, TInt aErr, 
                                   RBTDevAddrArray* aConflicts = NULL );

    /**
     * From MBTEngPluginObserver.
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void DisconnectComplete( const TBTDevAddr& aAddr, 
                                      TBTProfile aProfile, TInt aErr );

private:

    /**
     * C++ default constructor
     */
    CBTEngSrvPluginMgr( CBTEngServer* aServer );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @return ?description
     */
    TInt Connect( const TBTDevAddr& aAddr, const TBTDeviceClass& aDeviceClass );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @return ?description
     */
    TInt CancelConnect( const TBTDevAddr& aAddr );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @return ?description
     */
    TInt Disconnect( const TBTDevAddr& aAddr, TBTDisconnectType aDiscType );


    /**
     * ?description
     *
     * @since S60 v5.1
     * @param aDeviceClass Device class of the remote device is used if 
     *                      no EIR data of this device exists.
     * @param aAddr The address of remote device is used to get EIR data.
     *              Firstly use the service UUIDs in EIR data to determine if 
     *              the remote device is connectable or not. 
     * @return The index of plugin in the plugin array, which supports the 
     *              profiles via device class, 
     *          . 
     */
    TInt GetConnectablePluginIndex( const TBTDeviceClass& aDeviceClass, const TBTDevAddr& aAddr = TBTDevAddr() );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @return ?description
     */
    TInt GetConnectedAddresses( RBTDevAddrArray& aAddrArray, TBTProfile aProfile );

    /**
     * Checks from feature manager and central repository if the requested
     * feature is supported and enabled.
     *
     * @since S60 v3.2
     * @param aProfile The profile to check if it is supported and enabled.
     */
    TBool CheckFeatureEnabledL( TInt aProfile );

    /**
     * Maps a Clasee of Device to a profile identifier (UUID short form).
     *
     * @since S60 v3.2
     * @param aProfile The profile to map to a feature.
     */
    static TBTProfile MapDeviceClassToProfile( const TBTDeviceClass& aDeviceClass );

    /**
     * Maps a profile identifier (UUID short form) to a feature 
     * to check from the feature manager.
     *
     * @since S60 v3.2
     * @param aProfile The profile to map to a feature.
     */
    static TInt MapProfileToFeature( TInt aProfile );
    
    /**
     * Connectable if the UUID in Eir data is supported by plugins.
     *
     * @since S60 v5.1
     * @return The first index of plugin supporting specified UUID.
     */
    TInt GetConnectablePluginIndexByEir();
    
    /**
     * Get Eir data of device in cache.
     * 
     * @since S60 v5.1
     * @param aAddr The address of remote device
     * @return Symbian error code
     */
    TInt DoGetEirData( const TBTDevAddr& aAddr );
    
    /**
     * Utility function to check if profile supported by plugins.
     * 
     * @param aProfile The profile to check
     * @return The index of the first plugin in plugin array  
     *         supporting this profile, KErrNotFound if not supported.
     */
    TInt GetFirstPluginOfProfileSupported( TBTProfile aProfile );
	
	/** 
	Returns ETrue if the specified plugin is to be loaded in the current 
	'Enterprise Disabling of Bluetooth' mode, EFalse otherwise. 
	@param The implementation UID of the plugin we want to know whether should 
	be elided for Enterprise Disabling of Bluetooth reasons.
	@return See description.
	*/
	TBool FilterByEnterpriseDisablementModeL(TUid aUid) const;	
    
private: // data

    /**
     * Array of pointers to ECom plug-in information objects.
     */
    RImplInfoPtrArray iPluginInfoArray;

    /**
     * Array of pointers to the BTEng ECom plug-ins loaded by BTEng.
     */
    RPluginPtrArray iPluginArray;

    /**
     * The server instance.
     * Not own.
     */
    CBTEngServer* iServer;
    
    /**
     * Hose Resolver to get cached EIR.
     * Own.
     */    
    RHostResolver iHostResolver;
    
    /**
     * used for getting cached EIR.
     */
    TInquirySockAddr iInquirySockAddr;

    /**
     * Entry to store inquiry result.
     */
    TNameEntry iNameEntry;
    
    /**
     * Container of UUIDs, gotten from Eir data.
     * Own.
     */
    RExtendedInquiryResponseUUIDContainer iUuidContainter;
    };


//#include "?include_file.inl"


#endif // BTENGSRVPLUGINMGR_H
