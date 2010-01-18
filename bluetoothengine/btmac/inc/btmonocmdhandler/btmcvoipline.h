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
* Description:  A VoIP line
*
*/

#ifndef BTMCVOIPLINE_H_
#define BTMCVOIPLINE_H_

#include "btmccallstatus.h"
#include <mcallinformationobserver.h>
#include <e32base.h>

#include "btmcactive.h"
#include "btmccallactive.h"
#include "btmcline.h"
#include <mcallinformationobserver.h>
#include <ccallinformation.h>
#include <ccpdefs.h>

// FORWARD DECLARATIONS
class CBtmcCallStatus;
class MCCPCall;
class MCCPObserver;

NONSHARABLE_CLASS(CBtmcVoIPLine) : public CBase , public MBtmcLine, public MCallInformationObserver
    {
    public:
        
        /**
        * Two-phased constructor.
        */
        static CBtmcVoIPLine* NewL(CBtmcCallStatus& aParent, RMobilePhone& aPhone);

        /**
        * Two-phased constructor.
        */
        static CBtmcVoIPLine* NewLC(CBtmcCallStatus& aParent, RMobilePhone& aPhone);

        /**
        * Destructor.
        */
        ~CBtmcVoIPLine();                        
        
    public:   //From MBtmcLine

        /* From MBtmcLine; Returns the current call status 
         * @return Bit mask
         */
        TInt CallStatusL() const;
        
        /* From MBtmcLine; method which returns array of active calls 
         * @return Array of active calls
         */
        const RPointerArray<CBtmcCallActive>& ActiveCalls() const;
        
        /* From MBtmcLine; method which returns reference 
         * to RMobilePhone. Used only by MobileLine 
         * @return Reference to RMobilePhone
         */
        RMobilePhone& Phone();

        /* From MBtmcLine; method which returns reference 
         * to the call of type: ECallTypePS. Used only by VoipLine 
         * @return Reference to MCall
         */
        const MCall& CallInformationL();
        
        /* From MBtmcLine
         * Returns the type of line 
         * @return ETrue for Voip and EFalse otherwise
         * 
         */
        TBool IsVoip();

    private:
        
        // From MCallInformationObserver
        
        /**
         * Signals that there are changes in ongoing calls.
         *    
         * @return void
         */
        void CallInformationChanged();
        
    private:
        /**
        * C++ default constructor.
        */
        CBtmcVoIPLine(CBtmcCallStatus& aParent, RMobilePhone& aPhone);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();       

        /**
         * Leavable version of CallInformationChanged
         *    
         * @return void
         */
        void CallInformationChangedL();
        
   //Members
    private:        
            CBtmcCallStatus& iParent; 
            CCallInformation* iInfo;
            RPointerArray<CBtmcCallActive> iCallActives;            
            RMobilePhone& iPhone; //
    };

#endif /* BTMCVOIPLINE_H_ */
