/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#ifndef BTMCHANDLERAPI_H
#define BTMCHANDLERAPI_H

#include "btmcprofileid.h"
#include "btmcobserver.h"

// forward declarations
class CBtmcProtocol;

NONSHARABLE_CLASS(CBtmcHandlerApi) : public CBase
      {
public:

    IMPORT_C static CBtmcHandlerApi* NewL(
        MBtmcObserver& aObserver, TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated);

    IMPORT_C ~CBtmcHandlerApi();
    
    /**
    * Handle an AT command from HF or HS unit.
    *
    */
    IMPORT_C void HandleProtocolDataL(const TDesC8& aData);
    
    IMPORT_C void HandleNrecCompletedL(TInt aErr);
    
    IMPORT_C void ActivateRemoteVolumeControl();

    IMPORT_C void DeActivateRemoteVolumeControl();     
    
    IMPORT_C TInt GetRemoteSupportedFeature();
    
    /**
     * Set the support status of remote volume control of HSP in the remote device.
     * @param aSupported ETrue if remote volume control is supported by HS unit.
     */
    IMPORT_C void SetHspRvcSupported(TBool aSupported);

private:
    CBtmcHandlerApi();
    
    // 2nd phase construction, called by NewL()
    void ConstructL(MBtmcObserver& aObserver, TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated);

private:
    CBtmcProtocol* iHandler;
    };


#endif // BTMCHANDLERAPI_H

// End of File
 