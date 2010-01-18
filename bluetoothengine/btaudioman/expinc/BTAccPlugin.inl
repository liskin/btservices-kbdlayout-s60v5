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
*                 This is the interface implementation which is 
*                used by plugins. Server uses this implementation to create
*                plugins.
*  Version     : %version:  1.1.3.2.4 %
*
*/



// INCLUDE FILES

#include <ecom/ecom.h>       // declares E-com framework classes
#include <btaccParams.h>   // declares TPluginParams

inline CBTAccPlugin::~CBTAccPlugin()
    {
    REComSession::DestroyedImplementation(iInstanceId);
    }

inline CBTAccPlugin::CBTAccPlugin(TPluginParams& aParams)
:    iObserver(aParams.Observer()),
    iImplementationUid(aParams.ImplementationUid())
    {
    }

inline CBTAccPlugin* CBTAccPlugin::NewL(TPluginParams& aParams)
    {
    //TRACE_OPT(KPRINTFTRACE, DebugPrint(_L("[BTAccServer]\t CBTAccPlugin::NewL()")));

    CBTAccPlugin* self = reinterpret_cast<CBTAccPlugin*>(
        REComSession::CreateImplementationL(
            aParams.ImplementationUid(), 
            _FOFF(CBTAccPlugin, iInstanceId),
            (TAny*)&aParams)
        );

    return self;
    }

inline MBTAccObserver& CBTAccPlugin::Observer()
    {
    return iObserver;
    }

inline TUid CBTAccPlugin::Uid() const
    {
    return iImplementationUid;
    }
    
inline void CBTAccPlugin::AccOutOfUse()
    {  
    }

inline void CBTAccPlugin::StartRecording()
    {
        
    }

//
inline void CBTAccPlugin::ConnectToAccessory(const TBTDevAddr& /*aAddr*/, TRequestStatus& /*aStatus*/)
    {
        
    }

//
inline void CBTAccPlugin::CancelConnectToAccessory(const TBTDevAddr& /*aAddr*/)
    {
        
    }
    
//
inline void CBTAccPlugin::OpenAudioLink(const TBTDevAddr& /*aAddr*/, TRequestStatus& /*aStatus*/)
    {
        
    }

//    
inline void CBTAccPlugin::CancelOpenAudioLink(const TBTDevAddr& /*aAddr*/ )
    {
        
    }

//
inline void CBTAccPlugin::CloseAudioLink(const TBTDevAddr& /*aAddr*/, TRequestStatus& /*aStatus*/)
    {
        
    }
  
//    
inline void CBTAccPlugin::CancelCloseAudioLink(const TBTDevAddr& /*aAddr*/ )
    {
        
    }

inline void CBTAccPlugin::ActivateRemoteVolumeControl() {}

inline void CBTAccPlugin::DeActivateRemoteVolumeControl() {}

inline TInt CBTAccPlugin::GetRemoteSupportedFeature()
    {
    return 0;
    }
    
inline TInt CBTAccPlugin::AudioLinkLatency()
    {
    return KErrNotFound;
    }
//
// End of file
