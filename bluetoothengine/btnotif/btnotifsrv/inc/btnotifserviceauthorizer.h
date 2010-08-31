/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: class for prompting user to authorize a service
*              connection request.
*
*/

#ifndef BTNOTIFSERVICEAUTHORIZER_H
#define BTNOTIFSERVICEAUTHORIZER_H

#include <e32base.h>
#include "btnotificationresult.h"
#include "bluetoothdevicedialogs.h"

class CBTNotifSecurityManager;
class CBluetoothNotification;


NONSHARABLE_CLASS(CBTNotifServiceAuthorizer) 
    : public CBase, public MBTNotificationResult
    {
public:
    static CBTNotifServiceAuthorizer* NewL(CBTNotifSecurityManager& aParent);
    ~CBTNotifServiceAuthorizer();
    
private:
    CBTNotifServiceAuthorizer(CBTNotifSecurityManager& aParent);
    void ConstructL();
    
public:
    void StartNotifierL(const RMessage2& aMessage);
    
private: // From MBTNotificationResult
    void MBRDataReceived(CHbSymbianVariantMap& aData);
    void MBRNotificationClosed(TInt aError, const TDesC8& aData);
    
private:
    void PrepareNotificationL(TBool& aAutoAuthorize,
            TBluetoothDialogParams::TBTDialogType aType,
        TBTDialogResourceId aResourceId, TBool aPaired);
    TBool IsExistingConnectionToAudioL(const TBTDevAddr& aDevAddr);
    
private:
    CBTNotifSecurityManager& iParent;
    
    /**
     * Buffer containing the parameters of the client message.
     * Own.
     */
    RBuf8 iParams;
    
    /**
     * Pointer to an outstanding user interaction.
     * Not owned.
     */
    CBluetoothNotification* iNotification;
    
    // Contains the device name provided in params
    TBTDeviceName iCurrentDeviceName;
    
     // The client request.
    RMessage2 iNotifierMessage;
    
    // The bluetooth SDP Id of the connecting device.
    TUint iServiceId;
    
    // Variable that defines if the connecting device is to be trusted or not.
    TBool iSetTrusted;
    
    // Defines if the check box in the dialog is checked or not.
    TBool   iCheckBoxState;
    
    // Defines if the connecting device is paired or not.
    TBool   iPairedDevice;
    
    // Defines the class of the connecting device.
    TInt iDeviceClass;
    };

#endif /* BTNOTIFSERVICEAUTHORIZER_H_ */
