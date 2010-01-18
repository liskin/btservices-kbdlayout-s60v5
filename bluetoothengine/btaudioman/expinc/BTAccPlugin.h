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
*                 This is the interface declaration and implementation which is 
*                used by plugins. Contains also virtual methods which are implemented
*                by plugins, and hence used by server to call plugins. 
*  Version     : %version:  1.1.3.2.4 %
*
*/


#ifndef BTACC_PLUGIN_H
#define BTACC_PLUGIN_H

// FORWARD DECLARATION

#include <btaccObserver.h>
#include <e32base.h>
#include <btaccParams.h>


class CBTAccPlugin : public CBase
    {
public: // used by 'Plugin Mananger' to create and destroy plugin instances

    /** 
    Constructor.
    @param aParams Parameters from BT Acc Server, used to initialise the 
    implementation.
    @return Ownership of a new plugin.
    */
    static CBTAccPlugin* NewL(TPluginParams& aParams);

    /** Destructor. */
     virtual ~CBTAccPlugin();

public:
    /** 
    * Called by BT Acc Sever to connect to mono or stereo accessory
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
    * Called by BT Acc Sever to cancel connect to mono or stereo accessory
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void CancelConnectToAccessory(const TBTDevAddr& aAddr);

    /** 
    * Called by BT Acc Sever to disconnect from mono, stereo 
    * or remote control profile accessory
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void DisconnectAccessory(const TBTDevAddr& aAddr,TRequestStatus& aStatus) = 0;

    /** 
    * Called by BT Acc Sever to open audio link
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void OpenAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
    
    /** 
    * Called by BT Acc Sever to cancel opening of audio link
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void CancelOpenAudioLink(const TBTDevAddr& aAddr );

    /** 
    * Called by BT Acc Sever to close audio link
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void CloseAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    /** 
    * Called by BT Acc Sever to cancel closing of audio link
    @param aAddr Bluetooth Device address of the remote device
    @return result of operation
    */
    virtual void CancelCloseAudioLink(const TBTDevAddr& aAddr );
    
    /** 
    * Called by BT Acc Sever to inform plugin that accessory is in use (attached to acc fw). 
    @param  None.
    @return None.
    */
    virtual void AccInUse() = 0;
    
    /** 
    * Called by BT Acc Sever to inform plugin that accessory is out of use (detached from acc fw). 
    @param  None.
    @return None.
    */
    virtual void AccOutOfUse();
    
    
    /** 
    * BTAccServer informs 'BTSAC plugin' that it can start streaming
    @param  None.
    @return None.
    */
    virtual void StartRecording();

    /** 
    * Check whether plugin supports mono audio
    @param     None.
    @return ETrue if plugin supports mono audio, TFalse otherwise.
    */
    virtual TProfiles PluginType() = 0;
    
    virtual void ActivateRemoteVolumeControl();

    virtual void DeActivateRemoteVolumeControl();

    virtual TInt GetRemoteSupportedFeature();

    virtual TInt AudioLinkLatency();
    /**
    * Called by BT Acc Server to query the plugin's implementation UID.
    @return The plugin's implementation UID.
    */
    TUid Uid() const;


protected: // called by concrete bearer plugins
    /** 
    * Constructor
    @param aParams Parameters from plugin manager, used to initialise the 
    implementation.
    */
    CBTAccPlugin(TPluginParams& aParams);

    /**
    Accessor for the observer.
    @return The observer.
    */
    MBTAccObserver& Observer();

private: // unowned
    MBTAccObserver& iObserver;

private: // owned
    /**
    UID set by ECOM when the instance is created. Used when the instance is 
    destroyed.
    */
    TUid iInstanceId;

    /**
    Implementation UID of concrete instance.
    */
    const TUid iImplementationUid;
    };
    
#include "btaccPlugin.inl"


#endif // BTACC_PLUGIN_H
