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
* Description:  Gets the Signal strength indications
*
*/


#ifndef BTMCBATTERY_H
#define BTMCBATTERY_H

#include <e32base.h>
#include <etelmm.h>
#include <e32property.h>
#include <hwrmpowerstatesdkpskeys.h>

class CBtmcPhoneStatus;

/**
*  CBtmcBattery for getting signal strength.
*/
NONSHARABLE_CLASS(CBtmcBattery) : public CActive
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CBtmcBattery* NewL(
            CBtmcPhoneStatus& aParent);

        static CBtmcBattery* NewLC(
            CBtmcPhoneStatus& aParent);
    
        /**
        * Destructor.
        */
        ~CBtmcBattery();
    
        void GoActive();
        
        /*
        * returns current battery charge
        */
        TInt GetBatteryCharge();
        
        
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
        CBtmcBattery(
            CBtmcPhoneStatus& aParent);
    
        void ConstructL();
        
        void ConvertToHFPScale(TInt &aStrength);

    private:
        CBtmcPhoneStatus& iParent;
        RTimer iTimer;
        RProperty iProperty;
        EPSHWRMBatteryLevel iBatteryStrength;
        TInt iCharge;
    };


#endif