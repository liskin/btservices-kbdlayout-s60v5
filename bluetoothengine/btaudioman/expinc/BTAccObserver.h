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
* Description: 
*                 Inteface used by Plugins for call-back to Accessory Server
*
*/


#ifndef BTACC_OBSERVER_H
#define BTACC_OBSERVER_H

#include <e32def.h> // TInt
#include <btaccTypes.h>

// FORWARD DECLARATION
class TBTDevAddr;

// DATA

// CLASS DECLaRATION        
class MBTAccObserver
    {
public:

    
    /** 
    Called when an incoming response from a remote is ready to be picked up by 
    RemCon.
    @param aAddr The address the response came from.
    @return Error. If KErrNone, RemCon is committing to collecting the message 
    using GetResponse. If non-KErrNone, the message will be dropped by the 
    bearer.
    */
    virtual void NewAccessory(const TBTDevAddr& aAddr, TProfiles aType) = 0;    
    
    /** 
    Called when an incoming response from a remote is ready to be picked up by 
    RemCon.
    @param aAddr The address the response came from.
    @return Error. If KErrNone, RemCon is committing to collecting the message 
    using GetResponse. If non-KErrNone, the message will be dropped by the 
    bearer.
    */
    virtual void AccessoryDisconnected(const TBTDevAddr& aAddr, TProfiles aType) = 0;
    
    /** 
    Called when an incoming response from a remote is ready to be picked up by 
    RemCon.
    @param aAddr The address the response came from.
    @return Error. If KErrNone, RemCon is committing to collecting the message 
    using GetResponse. If non-KErrNone, the message will be dropped by the 
    bearer.
    */
    virtual void RemoteAudioOpened(const TBTDevAddr& aAddr, TProfiles  aType) = 0;
    
    /** 
    Called when an incoming response from a remote is ready to be picked up by 
    RemCon.
    @param aAddr The address the response came from.
    @return Error. If KErrNone, RemCon is committing to collecting the message 
    using GetResponse. If non-KErrNone, the message will be dropped by the 
    bearer.
    */
    virtual void RemoteAudioClosed(const TBTDevAddr& aAddr, TProfiles  aType) = 0;
    
    /**
    * Requests Accessory Framework or Dos to disable NREC
    * @param aBDAddress             BT hardware address of the accessory.
    * @return                       None.
    */
    virtual TInt DisableNREC(const TBTDevAddr& aAddr) = 0; 
    
    /** 
    * Called when a remote should be temporarily taken out of use. 
	* @param aAddr The address of the remote.
	*/
	virtual void AccessoryTemporarilyUnavailable(const TBTDevAddr& aAddr, TProfiles aType) = 0;
    
    
    virtual TInt ConnectionStatus(const TBTDevAddr& aAddr) = 0;
    
    virtual TBool IsAvrcpVolCTSupported() = 0;
    virtual TBool IsAvrcpLegacyVolCTSupported() = 0;
    virtual TBool IsAbsoluteVolumeSupported(const TBTDevAddr& aAddr) = 0;
    
    /**
     * Get the supported features of a profile in a device.
     * @param aAddr The address of the device.
     * @param aType the profile identifier
     * @return the supported features value.
     */
    virtual TInt SupportedFeature( const TBTDevAddr& aAddr, TProfiles aType ) const = 0;
    };

#endif // BTACC_OBSERVER_H
