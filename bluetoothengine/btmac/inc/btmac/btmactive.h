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
* Description:  Base Active Object declaration
*
*/


#ifndef C_BTMACTIVE_H
#define C_BTMACTIVE_H

//  INCLUDES
#include <e32base.h>
#include "btmactiveobserver.h"

/**
 * The base class of Active Object in BTMAC
 *
 * This class provides TRequestStatus for an async operation but it doesn't know the
 * detail of the request. CBtmActive could be used to implement timers, P&S subscribes 
 * that are "simple" enough operations. 
 *
 * @since S60 v3.1
 */
class CBtmActive : public CActive
    {
    
public:

    static CBtmActive* NewL(MBtmActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    virtual ~CBtmActive();

public:

    /**
     * Indicates a request has been issued and it is now outstanding
     *
     * @since S60 v3.1
     */
    virtual void GoActive();

    /**
     * Gets the request identifier that this active object is responsible for.
     *
     * @since S60 v3.1
     * @return the request identifier
     */
    TInt RequestId() const;
    
    /**
     * Sets the request identifier that this active object is responsible for.
     *
     * @since S60 v3.1
     * @param the request identifier
     */
    void SetRequestId(TInt aRequestId);

protected:

    /**
     * From CActive. 
     * cancels the outstanding request.
     *
     * @since S60 v3.1
     */
    virtual void DoCancel();

    /**
     * From CActive. 
     * Handles the request completion event.
     *
     * @since S60 v3.1
     */
    virtual void RunL();

    /**
     * From CActive. 
     * Handles the leave from RunL().
     *
     * @since S60 v3.1
     * @param aError the leave code in RunL()
     * @return the error code to Active Scheduler
     */
    virtual TInt RunError(TInt aError);

protected:

    CBtmActive(MBtmActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    MBtmActiveObserver& Observer();

private:

    /**
     * The observer which is interested in the request handling events.
     * Not own.
     */
    MBtmActiveObserver& iObserver;
    
    /**
     * The request identifier assigned to this active object.
     */
    TInt iRequestId;
    
    };

#endif  // C_BTMACTIVE_H
