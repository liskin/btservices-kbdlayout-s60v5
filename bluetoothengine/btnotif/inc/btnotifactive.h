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
* Description:  Declares active object helper class.
*
*/

#ifndef BTNOTIFACTIVE_H
#define BTNOTIFACTIVE_H


#include <e32base.h>

class CBTNotifActive;

/**
 *  Class MBTNotifActiveObserver
 *
 *  Callback class for receiving a completed active object event.
 *  Users of CBTEngActive need to derive from this class. 
 */
class MBTNotifActiveObserver
    {

public:

    /**
     * Callback to notify that an outstanding request has completed.
     *
     * @since S60 5.1
     * @param aActive Pointer to the active object that completed.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void RequestCompletedL( CBTNotifActive* aActive, TInt aId, 
                                    TInt aStatus ) = 0;

    /**
     * Callback to notify that an error has occurred in RunL.
     *
     * @param aActive Pointer to the active object that had error.
     * @param aId The ID that identifies the outstanding request.
     * @param aStatus The status of the completed request.
     */
    virtual void HandleError( CBTNotifActive* aActive, TInt aId, 
                                    TInt aError ) = 0;

    /**
     * Callback to notify that request is canceled.
     * Active object may add specified steps here. 
     *
     * @param aActive Pointer to the active object that is canceled.
     * @param aId The ID that identifies the outstanding request.
     */
    virtual void DoCancelRequest( CBTNotifActive* aActive, TInt aId ) = 0;
    };
/**
 *  Class CBTNotifActive
 */
NONSHARABLE_CLASS( CBTNotifActive ) : public CActive
    {

public:
    /**
     * Two-phased constructor.
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId       Request ID to identify which request is completed.
     * @return          Pointer to the constructed CBTNotifActive object.
     */ 
    static CBTNotifActive* NewL( MBTNotifActiveObserver* aObserver, TInt aId, TInt aPriority );

   /**
    * Destructor.
    */
    virtual ~CBTNotifActive();

    /**
     * Get the request ID of this active object.
     *
     * @since S60 v5.1
     * @return The request ID of this active object.
     */
    inline TInt RequestId();

    /**
     * Set a new request ID for this active object.
     *
     * @since S60 v5.1
     * @param The new request ID of this active object.
     */
    inline void SetRequestId( TInt aId );

    /**
     * Activate the active object.
     *
     * @since S60 v5.1
     */
    inline void GoActive();

    /**
     * Cancel an outstanding request.
     *
     * @since S60 v5.1
     */
    inline void CancelRequest();

    /**
     * Get a reference to the active object request status.
     *
     * @since S60 v5.1
     * @return Reference to the active object request status.
     */
    TRequestStatus& RequestStatus();

// from base class CActive

    /**
     * From CActive.
     * Called by the active scheduler when the request has been cancelled.
     *
     * @since S60 v5.1
     */
    void DoCancel();

    /**
     * From CActive.
     * Called by the active scheduler when the request has been completed.
     *
     * @since S60 v5.1
     */
    void RunL();

    /**
     * From CActive.
     * Called by the active scheduler when an error in RunL has occurred.
     *
     * @since S60 v5.1
     */
    TInt RunError( TInt aError );

private:

    /**
     * C++ default constructor
     *
     * @since S60 v5.1
     * @param aObserver Pointer to callback interface that receives notification
     *                  that the request has been completed.
     * @param aId ID of the request (for the client to keep track of multiple 
     *            active objects).
     * @param aPriority Priority of 
     */
    CBTNotifActive( MBTNotifActiveObserver* aObserver, TInt aId, TInt aPriority );

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v5.1
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
    MBTNotifActiveObserver* iObserver;

    };

#include "btnotifactive.inl"


#endif // BTNOTIFACTIVE_H
