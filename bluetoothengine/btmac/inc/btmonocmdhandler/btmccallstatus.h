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


#ifndef BTMCCALLSTATUS_H
#define BTMCCALLSTATUS_H

#include <e32base.h>
#include <etelmm.h>
#include "btmcprofileid.h"
#include "btmccallactive.h"
#include "btmcline.h"
#include "btmcvoipline.h"
// forward declarations
class CBtmcMobileLine;
class CATResult;
class CBtmcProtocol;
class MCall;

NONSHARABLE_CLASS(CBtmcCallStatus) : public CBase, public MBtmcActiveObserver
      {
    public:
        static CBtmcCallStatus* NewL(
            CBtmcProtocol& aProtocol, 
            RMobilePhone& aPhone,
            TBtmcProfileId aProfile );

        ~CBtmcCallStatus();
        
        /**
        * returns the bit mask
        */
        TInt CallStatusL() const;

        void ReportCallStatusL();

        /**
        * responds to CLCC
        **/
        
        void HandleClccL();

    private:  // From MBtmcActiveObserver
    
        void RequestCompletedL(CBtmcActive& aActive, TInt aErr);
        
        void CancelRequest(TInt aServiceId);

        
    private:
        CBtmcCallStatus(CBtmcProtocol& aProtocol);
        
        // 2nd phase construction, called by NewL()
        void ConstructL(RMobilePhone& aPhone, TBtmcProfileId aProfile);
        
        CATResult* MakeClccL(RMobileCall::TMobileCallInfoV1& info, 
            RMobileCall::TMobileCallRemotePartyInfoV1& remote, TInt aConferenceCallCount);
        
        CATResult* MakeVoIPClccL(const MCall& info);
        
        void HandleMobileCallEventL(
            const TName& aCallName, 
            const TDesC& aRemotePartyName,
            RMobileCall::TMobileCallStatus aStatus, TBool aOutgoingCall = EFalse);
            
        void ReportCallEventL(TInt aPrevStatus, TInt aNewStatus, TBool aOutgoing = EFalse);

        void ReportRingAndClipL(TBool aColp = EFalse);

        void StartTimerL(TInt aService, TInt aTimeout);
        
        void StopTimer(TInt aService);
        
        void ReportCallIndicatorL(TBTMonoATPhoneIndicatorId aIndicator, TInt aValue);

		// aNum returns the currently ringing number
		void GetRingingNumL(TDes8& aNum);
		void GetOutgoingNumL(TDes8& aNum);
        
		TBTMonoATPhoneNumberType NumberType( const TDesC8& aNum );
		
    private:
        CBtmcProtocol& iProtocol;
        RTimer iTimer;// owned
        CBtmcActive* iTimerActive; // owned, ringing timer
        TBuf8<RMobilePhone::KMaxMobileTelNumberSize> iRemoteTelNum;
        
        RPointerArray<MBtmcLine> iLines; // owned
        
        friend class CBtmcMobileLine;
        friend class CBtmcVoIPLine;
        };

#endif // BTMCCALLSTATUS_H

// End of File
 