/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Active object helper class.
*
*/



#ifndef BTENGACTIVE_H
#define BTENGACTIVE_H


#include <e32base.h>

class CBTEngActive;

/**  ?description */
//const ?type ?constant_var = ?constant;


/**
 *  Class MBTEngActiveObserver
 *
 *  Callback class for receiving a completed active object event.
 *  Users of CBTEngActive need to derive from this class. 
 *
 *  @lib bteng*.lib
 *  @since S60 v3.2
 */
class MBTEngActiveObserver
    {

public:

    /**
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 v3.2
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTEngActive* aActive, TInt aId, 
                                    TInt aStatus ) = 0;

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTEngActive* aActive, TInt aId, 
                                    TInt aError ) = 0;

    };


/**
 *  Class CBTEngActive
 *
 *  ?more_complete_description
 *
 *  @lib bteng*.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngActive ) : public CActive
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 v3.2
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @return Pointer to the constructed CBTEngActive object.
     */
    static CBTEngActive* NewL( MBTEngActiveObserver& aObserver, 
                                TInt aId, TInt aPriority );

    /**
     * Destructor
     */
    virtual ~CBTEngActive();

    /**
     * Get the request ID of this active object.
     *
     * @since S60 v3.2
     * @return The request ID of this active object.
     */
    inline TInt RequestId();

    /**
     * Set a new request ID for this active object.
     *
     * @since S60 v3.2
     * @param The new request ID of this active object.
     */
    inline void SetRequestId( TInt aId );

    /**
     * Activate the active object.
     *
     * @since S60 v3.2
     */
    inline void GoActive();

    /**
     * Cancel an outstanding request.
     *
     * @since S60 v3.2
     */
    inline void CancelRequest();

    /**
     * Get a reference to the active object request status.
     *
     * @since S60 v3.2
     * @return Reference to the active object request status.
     */
    TRequestStatus& RequestStatus();

// from base class CActive

    /**
     * From CActive.
     * Called by the active scheduler when the request has been cancelled.
     *
     * @since S60 v3.2
     */
    void DoCancel();

    /**
     * From CActive.
     * Called by the active scheduler when the request has been completed.
     *
     * @since S60 v3.2
     */
    void RunL();

    /**
     * From CActive.
     * Called by the active scheduler when an error in RunL has occurred.
     *
     * @since S60 v3.2
     */
    TInt RunError( TInt aError );

private:

    /**
     * C++ default constructor
     *
     * @since S60 v3.2
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId ID of the request (for the client to keep track of multiple 
     *            active objects).
     * @param aPriority Priority of 
     */
    CBTEngActive( MBTEngActiveObserver& aObserver, TInt aId, TInt aPriority );

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v3.2
     */
    void ConstructL();

private: // data

    /**
     * ID of the request (used only by our client).
     */
    TInt iRequestId;

    /**
     * Our observer.
     * Not own.
     */
    MBTEngActiveObserver& iObserver;

    };


#include "btengactive.inl"


#endif // BTENGACTIVE_H
