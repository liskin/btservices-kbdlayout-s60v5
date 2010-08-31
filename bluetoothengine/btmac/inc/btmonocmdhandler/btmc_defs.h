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
* Description: 
*     Extended Phone RemCon API.
*
*/


#ifndef BTMCDEFS_H
#define BTMCDEFS_H

#include <e32base.h>

enum TBTMonoATVoiceRecognition
    {
    EBTMonoATVoiceRecognitionOff = 0,
    EBTMonoATVoiceRecognitionOn = 1,
    };

enum TBTMonoVoiceRecognitionInitiator
    {
    EBTMonoVoiceRecognitionDefaultInitiator = 0,  // phone
    EBTMonoVoiceRecognitionActivatedByRemote,
    EBTMonoVoiceRecognitionDeactivatedByRemote,
    };

#ifndef NO101APPDEPFIXES
// Local definition of application-layer PubSub key 
// (TSINDKeys::ERecognitionState, in KSINDUID = KUidSystemCategoryValue):
const TInt KBTMonoVoiceRecognitionStateKey = 0;

// Local definition of application-layer key value
// (TRecognitionStateValues::ERecognitionStarted):
const TInt KBTMonoVoiceRecognitionStarted = 0;
#endif  //NO101APPDEPFIXES

enum TBTMonoATPhoneIndicatorId
    {
    EBTMonoATNetworkIndicator = 1,
    EBTMonoATCallIndicator = 2,
    EBTMonoATCallSetupIndicator = 3,
    EBTMonoATCall_SetupIndicator = 4,
    EBTMonoATCallHeldIndicator = 5,
    EBTMonoATSignalStrengthIndicator = 6,
    EBTMonoATRoamingIndicator = 7,
    EBTMonoATBatteryChargeIndicator = 8
    };

enum TBTMonoATNetworkStatus
    {
    EBTMonoATNetworkUnavailable = 0,
    EBTMonoATNetworkAvailable = 1
    };

enum TBTMonoATRoamingStatus
    {
    EBTMonoATRoamingInactive = 0,
    EBTMonoATRoamingActive = 1
    };

enum TBTMonoATCallStatus
    {
    EBTMonoATNoCall = 0,
    EBTMonoATCallActive = 1
    };

enum TBTMonoATCallSetupStatus
    {
    EBTMonoATNoCallSetup = 0,
    EBTMonoATCallRinging = 1,
    EBTMonoATCallDialling = 2,
    EBTMonoATCallConnecting = 3
    };

enum TBTMonoATCallHeldStatus
    {
    EBTMonoATNoCallHeld = 0,
    EBTMonoATCallHeldAndActive = 1,
    EBTMonoATCallHeldOnly = 2
    };


enum TBTMonoATPhoneNumberType
    {
    EBTMonoATPhoneNumberUnavailable = 128,
    EBTMonoATPhoneNumberNational = 129,
    EBTMonoATPhoneNumberInternational = 145
    };

// mandatory indicators including call, call setup and call held:
const TInt KMandatoryInds     = 0x00000001; 

// "service" indicator:
const TInt KIndServiceBit    = 0x00000002;

// "signal" indicator:
const TInt KIndSignalBit     = 0x00000004;

// "roam" indicator:
const TInt KIndRoamBit       = 0x00000008;

// "battchg" indicator:
const TInt KIndChargeBit     = 0x00000010;

// value to enable all indicators (when CMER=3,0,0,1 is received):
const TInt KIndAllActivated = KMandatoryInds |
                              KIndServiceBit | 
                              KIndSignalBit |
                              KIndRoamBit |
                              KIndChargeBit;

// value to disable all indicators:
const TInt KIndAllDeActivated = 0;

enum TBTMonoATCallerIdNotif
    {
    EBTMonoATCallerIdNotifDisabled = 0,
    EBTMonoATCallerIdNotifEnabled = 1,
    };
    
enum TBTMonoATCallWaitingNotif
    {
    EBTMonoATCallWaitingNotifDisabled = 0,
    EBTMonoATCallWaitingNotifEnabled = 1,
    };

enum TBTMonoATCallerIdNetworkServiceStatus
    {
    EBTMonoATCallerIdNetworkServiceUnavailable = 0,
    EBTMonoATCallerIdNetworkServiceAvailable = 1,
    EBTMonoATCallerIdNetworkServiceUnknown = 2
    };
    
enum TBTMonoATCregN
	{
	EBTMonoCregDisable = 0,
	EBTMonoCregEnableUnsolicited = 1,
	EBTMonoCregEnableAll = 2,
	};
enum TBTMonoATCregServiceStatus
	{
	EBTMonoCregNetworkServiceNotRegistered = 0,
	EBTMonoCregNetworkServiceHomeNetwork = 1,
	EBTMonoCregNetworkServiceNotRegisteredSearching = 2,
	EBTMonoCregNetworkServiceRegistrationDenied = 3,
	EBTMonoCregNetworkServiceUnknown = 4,
	EBTMonoCregNetworkServiceRegisteredRoaming = 5,
	};

const TInt KBTAGSupportedFeatureV10 = 0x2f;
const TInt KBTAGSupportedFeatureV15 = 0xef;

const TInt KRingInterval = 5 * 1000* 1000;  // 5 sec

const TInt KSlcTimeout = 20 * 1000 * 1000;  // 20 sec

const TInt KFirstHspCkpdTimeout = 1 * 1000 * 1000; // 1 sec

const TInt KCallDiallingBit       = 0x00000001;
const TInt KCallRingingBit        = 0x00000010;
const TInt KCallAnsweringBit      = 0x00000100;
const TInt KCallConnectingBit     = 0x00001000;
const TInt KCallConnectedBit      = 0x00010000;
const TInt KCallHoldBit           = 0x00100000;
const TInt KActiveCallMask        = 0x00110000;
const TInt KCallAllStatusMask     = 0x00111111;

const TInt KHfFeatureBitVoiceRecognition = 0x08;
const TInt KHfFeatureBitVolumeControl = 0x10;

_LIT8(KDesTestCodeCINDv15, "(\"service\",(0,1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"call_setup\",(0-3)),(\"callheld\",(0-2)),(\"signal\",(0-5)),(\"roam\",(0-1)),(\"battchg\",(0-5))");

_LIT8(KDesTestCodeCHLDv15, "(0,1,1x,2,2x,3,4)");

_LIT8(KDesTestCodeBVRA, "(0,1)");
_LIT8(KDesTestCodeCLIP, "(0,1)");
_LIT8(KDesTestCodeCCWA, "(0,1)");

#endif // BTMCDEFS_H

