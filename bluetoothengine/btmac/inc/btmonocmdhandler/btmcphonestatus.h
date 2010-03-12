/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Phone status monitor and notification
*
*/


#ifndef BTMCPHONESTATUS_H
#define BTMCPHONESTATUS_H


//  INCLUDES
#include <e32base.h>
#include <etelmm.h>
#include <e32property.h>
#include "btmcactive.h"
#include "btmcprofileid.h"
// MACROS

// DATA TYPES
    
// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CBtmcProtocol;
class RMobilePhone;
class CBtmcVolume;
class CBtmcSignal;
class CBtmcBattery;

// CLASS DECLARATION

/**
*  CBtmcPhoneStatus listens call state events (from RMobileCall) 
*  and passes them to MBtmcPhoneStatusObserver
*/
NONSHARABLE_CLASS(CBtmcPhoneStatus) : public CBase, public MBtmcActiveObserver
    {
    public:
        
        /**
        * Two-phased constructor.
        */
        static CBtmcPhoneStatus* NewL(
            CBtmcProtocol& aProtocol, RMobilePhone& aPhone, TBtmcProfileId aProfile);

        /**
        * Two-phased constructor.
        */
        static CBtmcPhoneStatus* NewLC(
            CBtmcProtocol& aProtocol, RMobilePhone& aPhone, TBtmcProfileId aProfile);

        /**
        * Destructor.
        */
        ~CBtmcPhoneStatus();
        
        RMobilePhone::TMobilePhoneRegistrationStatus NetworkStatus() const;
        
        void SetVolumeControlFeatureL(TBool aEnabled);
        
        void SetVoiceRecognitionControlL(TBool aEnabled);
        
        void SetSpeakerVolumeL(TInt aHfVol);
        
        void ActivateRemoteVolumeControl();

        void DeActivateRemoteVolumeControl();        
        
        TInt GetSignalStrength();
        
        TInt GetRssiStrength();
        
        TInt GetBatteryCharge();
        
        void SetRecognitionInitiator(TBTMonoVoiceRecognitionInitiator aInitiator);
        
        void HandleNetworkRegistrationEventL(
            RMobilePhone::TMobilePhoneRegistrationStatus aOldStatus, 
            RMobilePhone::TMobilePhoneRegistrationStatus aNewStatus);
        
        void HandleSpeakerVolumeEventL(TInt aVol);
        
        void HandleVoiceDialEventL(TInt aEnabled);
        
        void HandleNetworkStrengthChangeL(TInt8 aStrength);
        
        void HandleBatteryChangeL(TInt aLevel);        
        
    private:  // From MBtmcActiveObserver
    
        void RequestCompletedL(CBtmcActive& aActive, TInt aErr);
        
        void CancelRequest(TInt aServiceId);
            
    private:
        /**
        * C++ default constructor.
        */
        CBtmcPhoneStatus(CBtmcProtocol& aProtocol, RMobilePhone& aPhone);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(TBtmcProfileId aProfile);

        void ReportIndicatorL(TBTMonoATPhoneIndicatorId aIndicator, TInt aValue);

    private:
        CBtmcProtocol& iProtocol; // unowned
        RMobilePhone& iPhone; // unowned
        
        RPointerArray<CBtmcActive> iActives; // owned, Network and speed dial subscribe

        // for storing current network status
        RMobilePhone::TMobilePhoneRegistrationStatus iNetworkStatus;
        // for storing new network status
        RMobilePhone::TMobilePhoneRegistrationStatus iNewNetworkStatus;
        
        // Volume control
        CBtmcVolume* iVol;
        
        // Voice dial
        RProperty iVoiceDialProperty;
        
        TBool iVoiceRecognitionEnabled;
        TBTMonoVoiceRecognitionInitiator iVoiceRecogInitiator;
        
        CBtmcSignal* iSignal; // owned
        CBtmcBattery* iBattery; // owned        
    };
    
#endif  // BTMCPHONESTATUS_H

// End of File
