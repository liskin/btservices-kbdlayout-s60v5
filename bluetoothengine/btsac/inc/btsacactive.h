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


#ifndef C_BTSACACTIVE_H
#define C_BTSACACTIVE_H

//  INCLUDES
#include <e32base.h>
#include "btsacActiveObserver.h"

/**
 * The base class of Active Object.
 *
 * This class provides TRequestStatus for an async operation but it doesn't know the
 * detail of the request. CBtsacActive could be used to implement timers, P&S subscribes 
 * that are "simple" enough operations. 
 *
 * @since S60 v3.1
 */
class CBtsacActive : public CActive
    {
    
public:

    static CBtsacActive* NewL(MBtsacActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    static CBtsacActive* NewLC(MBtsacActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    virtual ~CBtsacActive();

public:

    /**
     * Calls SetActive().
     *
     * @since S60 v3.1
     */
    virtual void GoActive();

    /**
     * Gets the identifier of the request that this active object is serving.
     *
     * @since S60 v3.1
     * @return the request identifier
     */
    TInt RequestId() const;
    
    /**
     * Sets the identifier of the request that this active object is serving.
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

    CBtsacActive(MBtsacActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId);

    MBtsacActiveObserver& Observer();

private:

    /**
     * The observer which is interested in the request handling events.
     * Not own.
     */
    MBtsacActiveObserver& iObserver;
    
    /**
     * The request identifier assigned to this active object.
     */
    TInt iRequestId;
    
    };

#endif  // C_BTSACACTIVE_H
