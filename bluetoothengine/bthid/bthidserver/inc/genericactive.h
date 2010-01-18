/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  An Active Object offering its request status for any
 *                asynchronous request handling.
 *
*/


#ifndef C_GENERICACTIVE_H
#define C_GENERICACTIVE_H

//  INCLUDES
#include <e32base.h>

class CGenericActive;

/**
 * The observer of CGenericActive's request events
 *
 * This class defines the interface to handle request events from CGenericActive.
 *
 * @since S60 v3.1
 */
class MGenericActiveObserver
    {

public:

    /**
     * Called by CGenericActive::RunL() to handle the request completion event.
     *
     * @since S60 v5.0
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void RequestCompletedL(CGenericActive& aActive) = 0;

    /**
     * alled by CGenericActive::RunL() to handle the cancellation of an outstanding request.
     *
     * @since S60 v5.0
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void CancelRequest(CGenericActive& aActive) = 0;

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @since S60 v5.0
     * @param ?arg1 ?description
     */
    virtual void HandleError(TInt aId, TInt aError) = 0;
    };

/**
 *
 * CGenericActive provides TRequestStatus for an async operation but it doesn't know the
 * detail of the request. CGenericActive could be used to implement timers, P&S subscribes 
 * that are "simple" enough operations. 
 *
 * @since S60 v3.1
 */
class CGenericActive : public CActive
    {

public:

    static CGenericActive* New(MGenericActiveObserver& aObserver,
            CActive::TPriority aPriority, TInt aRequestId);

    static CGenericActive* NewL(MGenericActiveObserver& aObserver,
            CActive::TPriority aPriority, TInt aRequestId);

    virtual ~CGenericActive();

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

    TRequestStatus& RequestStatus();

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

    CGenericActive(MGenericActiveObserver& aObserver,
            CActive::TPriority aPriority, TInt aRequestId);

private:

    /**
     * The observer which is interested in the request handling events.
     * Not own.
     */
    MGenericActiveObserver& iObserver;

    /**
     * The request identifier assigned to this active object.
     */
    TInt iRequestId;

    };

#endif  // C_GENERICACTIVE_H
