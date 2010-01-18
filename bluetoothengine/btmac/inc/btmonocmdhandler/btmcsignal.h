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


#ifndef BTMCSIGNAL_H
#define BTMCSIGNAL_H

//  INCLUDES
#include <e32base.h>
#include <etelmm.h>

class CBtmcPhoneStatus;

const TInt KSignalTimerTimeout = 2000000;
/**
*  CBtmcSignal for getting signal strength.
*/
NONSHARABLE_CLASS(CBtmcSignal) : public CActive
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CBtmcSignal* NewL(CBtmcPhoneStatus& aParent);

        static CBtmcSignal* NewLC(CBtmcPhoneStatus& aParent);
    
        /**
        * Destructor.
        */
        ~CBtmcSignal();
    
        void GoActive();
        
        /*
        * Returns current signal strength
        */
        TInt GetSignalStrength();
        
        TInt GetRssiStrength();
        
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
        CBtmcSignal(CBtmcPhoneStatus& aParent);
    
        void ConstructL();
    
        void ConvertToHFPScale(TInt8 &aSignal);

    private:
        CBtmcPhoneStatus& iParent;
        RTelServer iServer;
        RMobilePhone iPhone;
        TInt32 iDbm;
        TInt8 iBars;
        TInt8 iPreviousBars;
    };


#endif