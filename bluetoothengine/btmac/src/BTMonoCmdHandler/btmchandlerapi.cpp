/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Extended Phone RemCon Handler 
*
*/


#include "btmchandlerapi.h"
#include "btmcprotocol.h"
#include "debug.h"

EXPORT_C CBtmcHandlerApi* CBtmcHandlerApi::NewL(
    MBtmcObserver& aObserver, TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated)
    {
    CBtmcHandlerApi* self=new (ELeave) CBtmcHandlerApi();
    CleanupStack::PushL(self);
    self->ConstructL(aObserver, aProfile, aBTDevAddr, aAccessoryInitiated);
    CleanupStack::Pop(self);
    return self;
    }

EXPORT_C CBtmcHandlerApi::~CBtmcHandlerApi()
    {
    TRACE_FUNC_ENTRY
    delete iHandler;
    TRACE_FUNC_EXIT
    }

EXPORT_C void CBtmcHandlerApi::HandleProtocolDataL(const TDesC8& aData)
    {
    TRACE_FUNC_ENTRY
    iHandler->NewProtocolDataL(aData);
    TRACE_FUNC_EXIT
    }

EXPORT_C void CBtmcHandlerApi::HandleNrecCompletedL(TInt aErr)
    {
    iHandler->HandleNrecCompletedL(aErr);
    }

EXPORT_C void CBtmcHandlerApi::ActivateRemoteVolumeControl()
    {
    iHandler->ActivateRemoteVolumeControl();
    }

EXPORT_C void CBtmcHandlerApi::DeActivateRemoteVolumeControl()
    {
    iHandler->DeActivateRemoteVolumeControl();
    }

EXPORT_C TInt CBtmcHandlerApi::GetRemoteSupportedFeature()
    {
    return iHandler->GetRemoteSupportedFeature();
    }

CBtmcHandlerApi::CBtmcHandlerApi()
    {
    }

void CBtmcHandlerApi::ConstructL(MBtmcObserver& aObserver, TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated)
    {
    TRACE_FUNC_ENTRY
    iHandler = CBtmcProtocol::NewL(aObserver, aProfile, aBTDevAddr, aAccessoryInitiated);
    TRACE_FUNC_EXIT
    }

// End of file
