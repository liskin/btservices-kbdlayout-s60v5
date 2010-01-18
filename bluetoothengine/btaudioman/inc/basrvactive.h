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


#ifndef C_BASRVACTIVE_H
#define C_BASRVACTIVE_H

//  INCLUDES
#include <e32base.h>

class CBasrvActive;

/**
 * The observer of CBasrvActive's request events
 *
 * This class defines the interface to handle a async request events from CBasrvActive.
 *
 * @since S60 v3.1
 */
class MBasrvActiveObserver
    {
    
public:

    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void RequestCompletedL(CBasrvActive& aActive) = 0;
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void CancelRequest(CBasrvActive& aActive) = 0;
    
    };

/**
 * A base class for abstract Active Objects.
 *
 * This class provides TRequestStatus for an async operation but it doesn't know the
 * detail of the request. CBasrvActive could be used to implement timers, P&S subscribes 
 * that are "simple" enough operations. 
 *
 * @since S60 v3.1
 */
class CBasrvActive : public CActive
    {
    
public:

    static CBasrvActive* New(MBasrvActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);
    

    static CBasrvActive* NewL(MBasrvActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    static CBasrvActive* NewLC(MBasrvActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    virtual ~CBasrvActive();

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
     * Gets the identifier of the request that this active object is serving.
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

    CBasrvActive(MBasrvActiveObserver& aObserver, 
        CActive::TPriority aPriority, TInt aRequestId);

    MBasrvActiveObserver& Observer();

private:

    /**
     * The observer which is interested in the request handling events.
     * Not own.
     */
    MBasrvActiveObserver& iObserver;
    
    /**
     * The request identifier assigned to this active object.
     */
    TInt iRequestId;
    
    };

#endif  // C_BASRVACTIVE_H
