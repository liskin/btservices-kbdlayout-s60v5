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
* Description:  Gets the MSISDN
*
*/


#ifndef BTMCOPERATOR_H
#define BTMCOPERATOR_H

//  INCLUDES
#include <e32base.h>
#include <etelmm.h>
#include <rmmcustomapi.h>
#include "btmcactive.h"

class CBtmcProtocol;

/**
*  CBtmcOperator for getting MSISDN number.
*/
NONSHARABLE_CLASS(CBtmcOperator) : public CBtmcActive
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CBtmcOperator* NewL(
            MBtmcActiveObserver& aObserver,
            CBtmcProtocol& aProtocol, 
            CActive::TPriority aPriority, 
            TInt aServiceId);

        /**
        * Destructor.
        */
        ~CBtmcOperator();

        void GoActive();

        /**
        * Gets the subscriber number
        */
        const TDesC& GetName() const;
        
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
        CBtmcOperator(
            MBtmcActiveObserver& aObserver,
            CBtmcProtocol& aProtocol, 
            CActive::TPriority aPriority, 
            TInt aServiceId);
    
        void ConstructL();

    private:
        CBtmcProtocol& iProtocol;
        RTelServer iServer;
        RMobilePhone iPhone;
		RMobileONStore iStore;
        RMmCustomAPI iCustomApi;
        RMmCustomAPI::TOperatorNameInfo iInfo;
    };


#endif