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


#ifndef BTMCNUMBER_H
#define BTMCNUMBER_H

//  INCLUDES
#include <e32base.h>
#include <etelmm.h>
#include "btmcactive.h"
#include "btmcobserver.h"

class CBtmcProtocol;

/**
*  CBtmcNumber for getting MSISDN number.
*/
NONSHARABLE_CLASS(CBtmcNumber) : public CBtmcActive
    {
    public:
    
    	/** CBtmcNumber states.
    	*/
    	enum BtmcNumberState
    		{
    		/** Not valid */
    		EStateNull = 0,
    		/** Getting the MSISDN - getting store info*/
    		EStateGettingMsisdnStoreInfo = 1,
    		/** Getting the MSISDN - getting store entry*/
    		EStateGettingMsisdnStoreEntry = 2,
    		};
    	
        /**
        * Two-phased constructor.
        */
        static CBtmcNumber* NewL(
            MBtmcActiveObserver& aObserver,
            CBtmcProtocol& aProtocol, 
            CActive::TPriority aPriority, 
            TInt aServiceId);
    
        /**
        * Destructor.
        */
        ~CBtmcNumber();
    
        void GoActive();
        
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
        CBtmcNumber(
            MBtmcActiveObserver& aObserver,
            CBtmcProtocol& aProtocol, 
            CActive::TPriority aPriority, 
            TInt aServiceId);
    
        void ConstructL();
        /**
        * Gets MSISDN store info to check how many numbers are stored on SIM
        */
        void GetMsisdnStoreInfo();
        /**
        * Gets single MSISDN store entry
        */
        void GetMsisdnStoreEntry();

    private:
        CBtmcProtocol& iProtocol;
        RTelServer iServer;
        RMobilePhone iPhone;
        RMobileONStore iStore;
		RMobileONStore::TMobileONEntryV1 iEntry;
		RMobileONStore::TMobileONEntryV1Pckg iPckg;
		RMobileONStore::TMobileONStoreInfoV1Pckg iOnStoreInfoPckg;
		RMobileONStore::TMobileONStoreInfoV1 iOnStoreInfo;
        /**
        * Defines current CBtmcNumber state
        */
		BtmcNumberState iState;
        /**
        * Index which is used to get MSISDN entries
        */
		TInt iUsedEntriesCount;
        /**
        * Count of correct numbers returned by MSISDN
        */
		TInt iCorrectNumbersCount;
    };


#endif
