/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Retrieves accessory information from Bluetooth Accessory Server
*
*/


#ifndef BTACCINFOREQUESTER_H
#define BTACCINFOREQUESTER_H

// INCLUDES
#include <e32base.h>
#include <AsyCmdTypes.h>
#include <bt_sock.h>
#include "BTAccClient.h"
#include "BTAccInfo.h"

// FORWARD DECLARATIONS
class TAccInfo;

// CLASS DECLARATION
class MBTAccInfoNotifier
    {
    public:
        virtual void GetBTAccInfoCompletedL(TInt aErr, const TASYBTAccInfo& aAccInfo) = 0;
    };

/**
*  Active object to fetch Acc Info from BTAccClient.
*
*  @lib BTAccInfo.dll
*  @since Series 60 3.1
*/
class CBTAccInfoRequester : public CActive
    {
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aNotifier A reference used for call-back to BTAccInfoCmdHandler class
        * @return The created object.        
        */
        static CBTAccInfoRequester* NewL(MBTAccInfoNotifier& aNotifier);
        
        /**
        * Destructor.
        */
        virtual ~CBTAccInfoRequester();

    public: // New functions
    
       /**
        * Gets the info of a BT audio accessory
        * @since Series 60 3.1
        * @param TBTDevAddr bluetooth device of the accessory
        *        whose information is required
        */    
        void GetBTAccInfoL(const TBTDevAddr& aAddr);
        
    protected: // Functions from base classes
        
        /**
        * From CActive Handles an active object's request completion event.
        */
        void RunL();
        
        void DoCancel();
        
    private:

        /**
        * C++ default constructor.
        * @param aNotifier A reference used for call-back to BTAccInfoCmdHandler class
        */
        CBTAccInfoRequester(MBTAccInfoNotifier& aNotifier);

    private:    // Data
    
        MBTAccInfoNotifier& iNotifier; // Store a reference to get a call-back to BTAccInfoCmdHandler class
        RBTAccClient iBTAccClient; // Call BT Acc Server component to retrieve information of an accessory
        
        TAccInfo iAccInfo;
        TPckg<TAccInfo> iAccInfoPckg;
    };

#endif // BTACCINFOREQUESTER_H
            
// End of File
