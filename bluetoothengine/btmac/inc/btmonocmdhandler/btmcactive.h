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


#ifndef BTMCACTIVE_H
#define BTMCACTIVE_H

//  INCLUDES
#include <e32base.h>
#include "btmc_defs.h"

class CBtmcActive;

class MBtmcActiveObserver
    {
    public:
        virtual void RequestCompletedL(CBtmcActive& aActive, TInt aErr) = 0;
        
        virtual void CancelRequest(TInt aServiceId) = 0;
    };

/**
*  CBtmcActive generates RING for incoming call.
*/
NONSHARABLE_CLASS(CBtmcActive) : public CActive
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CBtmcActive* NewL(MBtmcActiveObserver& aObserver, 
            CActive::TPriority aPriority, TInt aServiceId);

        static CBtmcActive* NewLC(MBtmcActiveObserver& aObserver, 
            CActive::TPriority aPriority, TInt aServiceId);

        /**
        * Destructor.
        */
        ~CBtmcActive();

    public:
    
        virtual void GoActive();

        TInt ServiceId() const;
        
        void SetServiceId(TInt aServiceId);

    protected:
    
        /**
        * From CActive. Called when asynchronous request completes.
        * @since 3.0
        * @param None
        * @return None
        */
        virtual void RunL();
        
        virtual void DoCancel();

    protected:
    
        /**
        * C++ default constructor.
        */
        CBtmcActive(MBtmcActiveObserver& aObserver, 
            CActive::TPriority aPriority, TInt aServiceId);

        MBtmcActiveObserver& Observer();

    private:
        MBtmcActiveObserver& iObserver;
        TInt iServiceId;
    };

#endif  // BTMCACTIVE_H

// End of File
