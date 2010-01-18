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
* Description:  Notify phone ring and caller ID
*
*/


#ifndef BTMCCALLACTIVE_H
#define BTMCCALLACTIVE_H

//  INCLUDES
#include <e32base.h>
#include <etelmm.h>
#include "btmcactive.h"

/**
*  CBtmcCallActive monitoring a call status.
*/
NONSHARABLE_CLASS(CBtmcCallActive) : public CBtmcActive
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CBtmcCallActive* NewL(
            MBtmcActiveObserver& aObserver, 
            CActive::TPriority aPriority, 
            TInt aServiceId,
            RMobileLine& aLine,
            const TName& aName);

        static CBtmcCallActive* NewLC(
            MBtmcActiveObserver& aObserver, 
            CActive::TPriority aPriority,
            TInt aServiceId,
            RMobileLine& aLine,
            const TName& aName);

        /**
        * Destructor.
        */
        ~CBtmcCallActive();
    
        void GoActive();

        /**
        * Gets call name
        * @param TDes&
        * @return None
        */
        const TDesC& CallName() const;

        /**
        * Gets remote's phone number
        * @param aText Phone number
        * @return TInt Error code
        */
        const TDesC& RemotePartyNumber() const;

        /**
        * Returns call status
        * @param None
        * @return TInt
        */
        RMobileCall::TMobileCallStatus CallStatus() const;
        
        /**
        * Returns call info
        * @param info RMobileCall::TMobileCallInfoV1 resulting call info
        * @param RMobileCall::TMobileCallRemotePartyInfoV1& remote remote side info
        * @return none
        */
        void GetCallInfo(RMobileCall::TMobileCallInfoV1& info);

	    /**
	    * Returns the emergency call status of this object
	    * @param None
	    * @return TBool
	    */
	    TBool IsEmergencyCall() const;
        
    protected:
    
        /**
        * From CActive. Called when asynchronous request completes.
        * @since 3.0
        * @param None
        * @return None
        */
        void RunL();
        
        void DoCancel();

        TInt RunError(TInt aErr);

    private:
    
        /**
        * C++ default constructor.
        */
        CBtmcCallActive(
            MBtmcActiveObserver& aObserver, 
            CActive::TPriority aPriority, 
            TInt aServiceId,
            RMobileLine& aLine,
            const TName& aName);
    
        void ConstructL();

    private:
        RMobileLine& iLine;
        RMobileCall iCall;

        // stores current call status
        RMobileCall::TMobileCallStatus iCallStatus;
        RMobileCall::TMobileCallStatus iPrevCallStatus;
        // stores call name 
        TName iName;

        // remote party's phone number
        TBuf<RMobilePhone::KMaxMobileTelNumberSize> iRemoteNumber;
		
		TBool iEmergency;
    };

#endif  // BTMCCALLACTIVE_H

// End of File
