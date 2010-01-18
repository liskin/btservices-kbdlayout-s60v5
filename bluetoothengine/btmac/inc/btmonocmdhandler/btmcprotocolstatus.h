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
*     
*
*/


#ifndef BTMCSTATE_H
#define BTMCSTATE_H

#include <e32base.h>
#include <etelmm.h>

#include "btmc_defs.h"
#include "btmcprofileid.h"

class TBtmcProtocolStatus
    {
    public:
        TBtmcProtocolStatus();
        TInt iAGSupportedFeature;
        TInt iAccSupportedFeature;
        TInt iAdvancedErrorCode; 
        TBtmcProfileId iProfile;
        TInt iIndicatorNotif;
        TBTMonoATCallerIdNotif iCallerIdNotif;
        TBTMonoATCallWaitingNotif iCallWaitingNotif;
        TInt iCallBits;
        TBool iSlc;
        TBool iNetworkRegStatusNotif;
        TBool iOutgoingCallNotif;
    };

inline TBtmcProtocolStatus::TBtmcProtocolStatus() 
    : iAGSupportedFeature(KBTAGSupportedFeatureV10), iAccSupportedFeature(0),
      iAdvancedErrorCode(0), iProfile(EBtmcHFP0105), iIndicatorNotif(KIndAllDeActivated),
      iCallerIdNotif(EBTMonoATCallerIdNotifDisabled), iCallWaitingNotif(EBTMonoATCallWaitingNotifDisabled), 
      iCallBits(0), iSlc(EFalse), iOutgoingCallNotif(EFalse)
    {
    }

#endif // BTMCSTATE_H

