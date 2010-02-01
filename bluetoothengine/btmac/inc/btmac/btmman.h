/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  the entry of this plugin.
*  Version     : %version: 12.1.7 %
*
*/


#ifndef C_BTMMAN_H
#define C_BTMMAN_H

#include <e32base.h>
#include <es_sock.h>

#include <btaccTypes.h>
#include <btaccPlugin.h>
#include <e32property.h>
//#include <BTServerInternalPSKeys.h>
#include "btmcobserver.h"
#include "btmcprofileid.h"
#include "btmactiveobserver.h"
#include "btmdefs.h"
#include <btengdiscovery.h>
//#include <btengdomaincrkeys.h>

class RSocketServ;
class CBtmState;
class CBtmActive;
class CBtmcHandlerApi;

/**
 * the main class of BTMAC.
 *
 * It implements plugin API and is the owner of the state machine.
 *
 *  @since S60 v3.1
 */
class CBtmMan : public CBTAccPlugin, public MBtmcObserver, public MBtmActiveObserver
    {
public:
    static CBtmMan* NewL(TPluginParams& aParams);

    virtual ~CBtmMan();

    /** 
     * State transition
     * 
     * @since S60 v3.1
     * @param aState the next state to be entered.
     */
    void ChangeStateL(CBtmState* aNext);

    /** 
     * Gets the socket server.
     * 
     * @since S60 v3.1
     * @return the socket server.
     */
    RSocketServ& SockServ();
 
    /** 
     * Indicates a new connection from a mono accessory.
     * 
     * @since S60 v3.1
     * @param aAddr The acc BD address
     * @param aProfile the connected profile
     */
    void NewAccessory(const TBTDevAddr& aAddr, TProfiles aProfile);    
    
    /** 
     * Indicates a disconnection originated from a mono accessory.
     * 
     * @since S60 v3.1
     * @param aAddr The acc BD address
     * @param aProfile the disconnected profile
     */
    void AccessoryDisconnected(const TBTDevAddr& aAddr, TProfiles aProfile);
    
    /** 
     * Indicates a audio link open originated from a mono accessory.
     * 
     * @since S60 v3.1
     * @param aAddr The acc BD address
     * @param aProfile the connected profile
     */
    void RemoteAudioOpened(const TBTDevAddr& aAddr, TProfiles aProfile);
    
    /** 
     * Indicates a audio link close originated from a mono accessory.
     * 
     * @since S60 v3.1
     * @param aAddr The acc BD address
     * @param aProfile the connected profile
     */
    void RemoteAudioClosed(const TBTDevAddr& aAddr, TProfiles aProfile);

    /**
     * Register a BT service (Profile) in SDP server
     * 
     * @since S60 v3.1
     * @param aService the service
     * @param aChannel the RFComm channel that the service is registered on
     */
    void RegisterServiceL(TUint aService, TUint aChannel);
    
    /**
     * Delete the given registered service from SDP server.
     * 
     * @since S60 v3.1
     * @param aService the service
     */
    void DeleteRegisteredService(TUint aService); 
    
    /**
     * Delete all registered services from SDP server
     * 
     * @since S60 v3.1
     */
    void DeleteAllRegisteredServices(); 

    /**
     * Get a last used channel(port) for a specified service
     * 
     * @since S60 v3.2
     */
    TInt GetLastUsedChannel(TUint aService); 
    
    
    /**    
     * Get audio latency
     *
     * @since S60 v5.0
     */ 
    TInt AudioLinkLatency();   
    
    TBool IsTrashBinEmpty();
    
    void LoadCmdHandlerL(TBtmcProfileId aProfile, const TBTDevAddr& aAddr, TBool aAccessoryInitiated);
    
    /**
     * Set the support status of remote volume control of HSP in the remote device.
     * @param aSupported ETrue if remote volume control is supported by HS unit.
     */
    void SetCmdHandlerHspRvcSupported( TBool aSupported );
    
    void DeleteCmdHandler();

    void NewProtocolDataL(const TDesC8& aData);

    TBool IsAccInuse() const;

    TBool IsEdrSupported() const;

    /**
     * Update the cmdhandler the support of remove volume control.
     * @param aAddr the address of the remote device.
     */
    void SetCmdHandlerRvcSupport( const TBTDevAddr& aAddr );
    
private:
    // From base class CBTAccPlugin

    /** 
     * Connect to mono accessory.
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     * @param aStatus On completion, will contain an error code
     */
    void ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Cancel connect
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     */
    void CancelConnectToAccessory(const TBTDevAddr& aAddr);

    /** 
     * Disconnect to mono accessory
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     * @param aStatus On completion, will contain an error code
     */
    void DisconnectAccessory(const TBTDevAddr& aAddr,TRequestStatus& aStatus);

    /** 
     * Open audio link
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     * @param aStatus On completion, will contain an error code
     */
    void OpenAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
     * Close open audio link
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     */
    void CancelOpenAudioLink(const TBTDevAddr& aAddr );

    /** 
     * Called by BT Acc Sever to connect to mono or stereo accessory
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     * @param aStatus On completion, will contain an error code
     */
    void CloseAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
    
    /** 
     * Close audio link
     * 
     * @since S60 v3.1
     * @param aAddr BD address of the remote device
     * @param aStatus On completion, will contain an error code
     */
    void CancelCloseAudioLink(const TBTDevAddr& aAddr );

    /** 
     * Informs accessory is in use. 
     * 
     * @since S60 v3.1
     */
    void AccInUse();
    
    void AccOutOfUse();

    /** 
     * Check which accessory type is supported by this plugin.
     * 
     * @since S60 v3.1
     * @return the supported plugin type.
     */
    TProfiles PluginType();
    
    void ActivateRemoteVolumeControl();

    void DeActivateRemoteVolumeControl();

    TInt GetRemoteSupportedFeature();

    // From base class CBtmActive

    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void RequestCompletedL(CBtmActive& aActive);

    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void CancelRequest(CBtmActive& aActive);

private:
    // From MBtmcObserver
    void SlcIndicateL(TBool aSlc);

    void SendProtocolDataL(const TDesC8& aData);

    
private:
    CBtmMan(TPluginParams& aParams);

    void ConstructL();

private:
    /**
     * The socket server.
     */
    RSocketServ iSockServ;
    
    /**
     * the current state.
     */
    CBtmState* iState;

    /**
     * The place where obsolete state instances are temporarily stored.
     */
    RPointerArray<CBtmState> iTrashBin;
    
    CBtmActive* iActive;
    
    /**
     * the property for mono accessory connection state
     */
    RProperty iMonoStateProp;
    
    /**
     * the property for connected mono accessory BD address
     */
    RProperty iMonoAddrProp;

    /**
     * the list of registered service in SDP server.
     */
    RArray<TBtmService> iServices;

    /*
    * at command handler
    */
    CBtmcHandlerApi* iCmdHandler;
    
    TBool iAccInuse;
    
    TBool iEdr;
    
    CBTEngDiscovery* iBteng;
    };

#endif // C_BTMMAN_H
