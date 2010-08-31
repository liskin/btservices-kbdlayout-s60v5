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
* Description:  The base active object declaration
*
*/


#ifndef BT_SERVICE_DELAYED_DESTROYER_H
#define BT_SERVICE_DELAYED_DESTROYER_H

//  INCLUDES
#include <btengdiscovery.h>
#include <e32base.h>

NONSHARABLE_CLASS (CBTServiceDelayedDestroyer) : public CActive
    {
    
public:

    static CBTServiceDelayedDestroyer* NewL(CActive::TPriority aPriority);

    static CBTServiceDelayedDestroyer* NewLC(CActive::TPriority aPriority);

    virtual ~CBTServiceDelayedDestroyer();

public:

    /**
     * Calls SetActive().
     */
    void GoActive();
    
    // Sets the pointer which will be destroyed.
    // Must be called with GoActive, they are pairs, first 
    // call SetDestructPointer and then GoActive.
    void SetDestructPointer(CBTEngDiscovery* aPtr);

private:    

    /**
     * From CActive. 
     * cancels the outstanding request.
     */
    virtual void DoCancel();

    /**
     * From CActive. 
     * Handles the request completion event.
     * Deletes CBTEngDiscovery object which is passed via SetDestructPointer method.
     */
    virtual void RunL();

    /**
     * From CActive. 
     * Handles the leave from RunL().
     * @param aError the leave code in RunL()
     * @return the error code to Active Scheduler
     */
    virtual TInt RunError(TInt aError);

private:    

    CBTServiceDelayedDestroyer(CActive::TPriority aPriority);

private:
    TRequestStatus iStatus;
    CBTEngDiscovery* iPtr;    
    };

#endif  // BT_SERVICE_DELAYED_DESTROYER_H
