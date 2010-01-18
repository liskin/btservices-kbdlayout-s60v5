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
* Description:  General active object offering asynchronous service 
*
*/


#ifndef BTRCCACTIVE_H
#define BTRCCACTIVE_H

//  INCLUDES
#include <e32base.h>

class CBTRCCActive;

class MBTRCCActiveObserver
    {
    public:
        virtual void RequestCompletedL(CBTRCCActive& aActive, TInt aErr) = 0;
        
        virtual void CancelRequest(TInt aServiceId) = 0;
    };

/**
*  CBTRCCActive listens to the Volume PS Key changes.
*/
NONSHARABLE_CLASS(CBTRCCActive) : public CActive
    {
    public:
    
        /**
        * Two-phased constructor.
        */
        static CBTRCCActive* NewL(MBTRCCActiveObserver& aObserver, TInt aServiceId = 0, 
            CActive::TPriority aPriority = EPriorityStandard /*TPriority::EPriorityStandard*/);

        static CBTRCCActive* NewLC(MBTRCCActiveObserver& aObserver, TInt aServiceId = 0, 
                CActive::TPriority aPriority = EPriorityStandard /*TPriority::EPriorityStandard*/);

        /**
        * Destructor.
        */
        ~CBTRCCActive();

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
        CBTRCCActive(MBTRCCActiveObserver& aObserver, 
                TInt aServiceId, CActive::TPriority aPriority);

        MBTRCCActiveObserver& Observer();

    private:
        MBTRCCActiveObserver& iObserver;
        TInt iServiceId;
    };

#endif  // BTRCCACTIVE_H

// End of File
