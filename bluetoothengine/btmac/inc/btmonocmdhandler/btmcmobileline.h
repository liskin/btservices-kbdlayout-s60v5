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
* Description:  A mobile line
*
*/


#ifndef BTMCMOBILELINE_H
#define BTMCMOBILELINE_H


//  INCLUDES
#include <e32base.h>
#include <etelmm.h>      // for etel
#include <mmtsy_names.h> // for etel

#include "btmccallactive.h"
#include "btmcline.h"

#include "btmcdummy.h"

// FORWARD DECLARATIONS
class CBtmcCallStatus;

/**
*  CBtmcMobileLine listens call state events (from RMobileCall) 
*  and passes them to MBtmcMobileLineObserver
*/
NONSHARABLE_CLASS(CBtmcMobileLine) : public CBase, public MBtmcLine, public MBtmcActiveObserver
    {
    public:
        
        /**
        * Two-phased constructor.
        */
        static CBtmcMobileLine* NewL(
            CBtmcCallStatus& aParent, RMobilePhone& aPhone, const TDesC& aLineName);

        /**
        * Two-phased constructor.
        */
        static CBtmcMobileLine* NewLC(
            CBtmcCallStatus& aParent, RMobilePhone& aPhone, const TDesC& aLineName);

        /**
        * Destructor.
        */
        ~CBtmcMobileLine();
        
        /**
        * returns the bit mask
        */
        TInt CallStatusL() const;
        
        const MCall& CallInformationL();
        
        /**
        * returns active call container
        */
        const RPointerArray<CBtmcCallActive>& ActiveCalls() const;
        
        /**
        * Returns call status
        * @param None
        * @return the call status
        */
        //RMobileCall::TMobileCallStatus CallStatus(const TName& aCallName) const;
        
        /**
        * Accessor for phone
        **/
        RMobilePhone& Phone();
        
        /* From MBtmcLine
         * Returns the type of call 
         * @return ETrue for Voip and EFalse otherwise
         * 
         */
        TBool IsVoip(); 
        
    private:  // From MBtmcActiveObserver
    
        void RequestCompletedL(CBtmcActive& aActive, TInt aErr);
        
        void CancelRequest(TInt aServiceId);
            
    private:
        /**
        * C++ default constructor.
        */
        CBtmcMobileLine(
            CBtmcCallStatus& aParent, RMobilePhone& aPhone, const TDesC& aLineName);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:
        CBtmcCallStatus& iParent;

        // needed for opening RMobileLine
        RMobilePhone& iPhone; // unowned

        RMobileLine iLine; // owned
        
        RPointerArray<CBtmcCallActive> iCallActives;
        
        CBtmcActive* iLineIncomingActive;
        CBtmcActive* iLineOutgoingActive;
        
        TBtmcDummyCall iDummyCall; // used in implemtation of CallInformationL()

        TBuf<KMaxName> iLineName;
        
        // passed to RMobileLine::NotifyCallAdded
        TName iName;
        
    };
#endif  // BTMCMOBILELINE_H

// End of File
