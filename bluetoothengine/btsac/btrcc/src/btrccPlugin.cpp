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
* Description:  BT Remote Control Controller class declaration.
*
*/


// INCLUDE FILES

#include <btdevice.h>
#include <e32property.h>
#include <remconaddress.h>

#include "btrccPlugin.h"
#include "debug.h"
#include "btrccLinker.h"

// CONSTANTS

#ifdef _DEBUG
// Length of a BT hardware address (BD_ADDR) as a descriptor
const TUint8 KBTRCCBTDevAddrDesLength = KBTDevAddrSize * 2;
#endif

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCPlugin::NewL
// -----------------------------------------------------------------------------
//
CBTRCCPlugin* CBTRCCPlugin::NewL(TPluginParams& aParams)
    {
    CBTRCCPlugin* self = new (ELeave) CBTRCCPlugin(aParams);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTRCCPlugin::~CBTRCCPlugin()
	{
	TRACE_FUNC
	delete iLinker;
	}

// -----------------------------------------------------------------------------
// CBTRCCPlugin::CBTRCCPlugin
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CBTRCCPlugin::CBTRCCPlugin(TPluginParams& aParams)
    : CBTAccPlugin(aParams)
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCPlugin::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::ConstructL()
    {
    TRACE_FUNC
	iLinker = CBTRCCLinker::NewL(Observer());
   	}

// -----------------------------------------------------------------------------
// CBTRCCPlugin::ConnectToAccessory
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    iLinker->Connect(aAddr, aStatus);
    }

// -----------------------------------------------------------------------------
// CBTRCCPlugin::CancelConnectToAccessory
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::CancelConnectToAccessory(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    iLinker->CancelConnect(aAddr);
    }
   	
// -----------------------------------------------------------------------------
// CBTRCCPlugin::DisconnectAccessory
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::DisconnectAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
	{
	TRACE_FUNC
	TRACE_INFO_SEG(
        {TBuf<KBTRCCBTDevAddrDesLength> buf; aAddr.GetReadable(buf); Trace(_L("BT Addr %S"), &buf);})
    iLinker->Disconnect(aStatus, aAddr);
	}

// -----------------------------------------------------------------------------
// CBTRCCPlugin::AccInUse
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::AccInUse()
	{
	TRACE_FUNC
	}
	
// -----------------------------------------------------------------------------
// CBTRCCPlugin::PluginType
// -----------------------------------------------------------------------------
//
TProfiles CBTRCCPlugin::PluginType()
    {
    TRACE_FUNC
    return EAnyRemConProfiles;
    }

// -----------------------------------------------------------------------------
// CBTRCCPlugin::ActivateRemoteVolumeControl
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::ActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    iLinker->ActivateRemoteVolumeControl();
    }

// -----------------------------------------------------------------------------
// CBTRCCPlugin::PluginType
// -----------------------------------------------------------------------------
//
void CBTRCCPlugin::DeActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    iLinker->DeActivateRemoteVolumeControl();
    }

//  End of File  
