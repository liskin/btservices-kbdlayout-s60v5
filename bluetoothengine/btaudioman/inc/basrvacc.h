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
*
*/


#ifndef C_BTASRVACC_H
#define C_BTASRVACC_H

#include <e32base.h> 
#include <bttypes.h>
#include <btengconstants.h>
#include <AccessoryTypes.h>
#include "BTAccInfo.h"
#include "btaccTypes.h"

class CBasrvAccState;
class TAccInfo;
class CBasrvAccMan;
class CBasrvPowerControlAgent;
class CBasrvRvc;

class CBasrvAcc : public CBase
    {
public:
    static CBasrvAcc* NewL(CBasrvAccMan& aAccMan);
    
    static CBasrvAcc* NewLC(CBasrvAccMan& aAccMan);

    ~CBasrvAcc();

    const TBTDevAddr& Remote();
        
    void ConnectL(const TBTDevAddr& aAddr);
    
    void CancelConnect();
    
    void DisconnectL();

    void AccessoryConnectedL(const TBTDevAddr& aAddr, TProfiles aProfile);

    void AccOpenedAudio(TProfiles aProfile);
    
    void AccClosedAudio(TProfiles aProfile);
    
    void AccessoryDisconnectedL(TProfiles aProfile);
    
    const TAccInfo* AccInfo(const TBTDevAddr& aAddr);
    
    TAccInfo& AccInfo();
    
    /**
    * Called when an audio open request comes from Acc FW.
    */
    void OpenAudioL(TAccAudioType aType);

    /**
    * Called when an audio close request comes from Acc FW.
    */
    void CloseAudioL(TAccAudioType aType);

    void ChangeStateL(CBasrvAccState* aNewState);
    
    CBasrvAccMan& AccMan();

    TBTEngConnectionStatus ConnectionStatus() const;
    
    void RequestSniffMode();

    void RequestActiveMode();

    void CancelPowerModeControl();    

    void NotifyLinkChange2Rvc();
    
    void PreventLowPowerMode();
    
    void AllowLowPowerMode();
    
    /**
     * Get the supported features of a profile in this device.
     * @param aProfile the profile identifier
     * @return the supported features value.
     */
    TInt SupportedFeature( TProfiles aProfile );
    
protected:

    /**
    * Default constructor
    */
    CBasrvAcc(CBasrvAccMan& aAccMan);

    void ConstructL();

protected:
    CBasrvAccMan& iAccMan;
    
    CBasrvAccState* iState;
    
    TAccInfo iAccInfo;    

    CBasrvPowerControlAgent* iPowerController;
    CBasrvRvc* iRvc;
    };

#endif      // C_BTASRVACC_H

// End of File
